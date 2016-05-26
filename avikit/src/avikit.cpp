// AVIKit
// ---------------------------------------------------------------------------
//
// Copyright (c) 2003, Ruari O'Sullivan
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// 
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution. 
// 
// Neither the name of AVIKit's copyright owner nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission. 
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// ---------------------------------------------------------------------------

#include "..\include\avikit.h"



void AVIKit::addError(int error)
{
  errorList.push_back(error);
}

// get errors in order of occurrance.
int AVIKit::getError(char **emsg)
{
  int error = 0;

  if (errorList.size() != 0)
  {
    error = errorList.front();
    errorList.pop_front();
  }

  if (emsg != NULL)
  {
    switch (error)
    {
    case AVIKIT_NOERROR:
      (*emsg) = "No error.";
      break;

    case AVIKIT_ERROR_BADFILE:
      (*emsg) = "No handler found, or file does not exist.";
      break;
    case AVIKIT_ERROR_BADFORMAT:
      (*emsg) = "Corrupt AVI or unknown format.";
      break;
    case AVIKIT_ERROR_MEMERROR:
      (*emsg) = "Not enough memory to open AVI.";
      break;
    case AVIKIT_ERROR_DISKERROR:
      (*emsg) = "Disk error attempting to read AVI.";
      break;

    // errors opening video
    case AVIKIT_ERROR_NOVIDEO:
      (*emsg) = "No video stream found in specified file.";
      break;
    case AVIKIT_ERROR_VIDEOERROR:
      (*emsg) = "VFW failed to read video stream.";
      break;

    // errors opening audio
    case AVIKIT_ERROR_ACM:
      (*emsg) = "ACM failed to open conversion stream.";
      break;
    case AVIKIT_ERROR_ACMCODEC:
      (*emsg) = "ACM does not support this audio codec.";
      break;

    default:
      (*emsg) = "Unknown error.";
      break;
    }
  }

  return error;
}



// Converts a compressed audio stream into uncompressed PCM.
bool AVIKit::ACMConvertAudio(PAVISTREAM audio_stream, WAVEFORMAT *audio_header, short *bits)
{
  WAVEFORMATEX dest_header, *sh, *dh;
  AVISTREAMINFO stream_info;

  // WMA codecs, both versions - they simply don't work.
  if (audio_header->wFormatTag == 0x160 || audio_header->wFormatTag == 0x161)
  {
    addError(AVIKIT_ERROR_ACMCODEC);
    return false;
  }

  // get audio stream info to work with
  AVIStreamInfo(audio_stream, &stream_info, sizeof(stream_info));

  if (audio_header_size < sizeof(WAVEFORMATEX))
  {
    addError(AVIKIT_ERROR_ACM); // "couldn't get header for ACM conversion"
    return false;
  }

  sh = (WAVEFORMATEX *)audio_header;

  *bits = 16;

  // how much of this is actually required?
  dest_header.wFormatTag = WAVE_FORMAT_PCM; // yay
  dest_header.wBitsPerSample = *bits; // 16bit
  dest_header.nChannels = sh->nChannels;
  dest_header.nSamplesPerSec = sh->nSamplesPerSec; // take straight from the source stream
  dest_header.nAvgBytesPerSec = ((*bits)>>3)*sh->nChannels*sh->nSamplesPerSec;
  dest_header.nBlockAlign = ((*bits)>>3)*sh->nChannels;
  dest_header.cbSize = 0; // no more data.

  dh = &dest_header;

  // open the stream
  if (acmStreamOpen(&cpa_conversion_stream, NULL, sh, dh, 
                      NULL, 0, 0, 0) != MMSYSERR_NOERROR)
  {
    // try with 8 bit destination instead
    *bits = 8;

    dest_header.wBitsPerSample = *bits; // 8bit
    dest_header.nAvgBytesPerSec = ((*bits)>>3)*sh->nChannels*sh->nSamplesPerSec;
    dest_header.nBlockAlign = ((*bits)>>3)*sh->nChannels; // 1 sample at a time

    if (acmStreamOpen(&cpa_conversion_stream, NULL, sh, dh, 
                        NULL, 0, 0, 0) != MMSYSERR_NOERROR)
    {
      addError(AVIKIT_ERROR_ACM); // "couldn't open ACM conversion stream"
      return false;
    }
  }

  cpa_blockalign = sh->nBlockAlign;
  unsigned long dest_length = 0;

  // mp3 specific fix
  if (sh->wFormatTag == 0x55)
  {
    LPMPEGLAYER3WAVEFORMAT k = (LPMPEGLAYER3WAVEFORMAT)sh;
    cpa_blockalign = k->nBlockSize;
  }

  // get the size of the output buffer for streaming the compressed audio
  if (acmStreamSize(cpa_conversion_stream, cpa_blockalign, &dest_length, 
                      ACM_STREAMSIZEF_SOURCE) != MMSYSERR_NOERROR)
  {
    addError(AVIKIT_ERROR_ACM); // "couldn't get ACM conversion stream size"
    acmStreamClose(cpa_conversion_stream, 0);
    return false;
  }

  cpa_srcbuffer = (unsigned char *)malloc(cpa_blockalign);
  cpa_dstbuffer = (unsigned char *)malloc(dest_length); // maintained buffer for raw data

  // prep the headers!
  cpa_conversion_header.cbStruct = sizeof(ACMSTREAMHEADER);
  cpa_conversion_header.fdwStatus = 0;
  cpa_conversion_header.dwUser = NULL; // no user data
  cpa_conversion_header.pbSrc = cpa_srcbuffer; // source buffer
  cpa_conversion_header.cbSrcLength = cpa_blockalign; // source buffer size
  cpa_conversion_header.cbSrcLengthUsed = 0;
  cpa_conversion_header.dwSrcUser = NULL; // no user data
  cpa_conversion_header.pbDst = cpa_dstbuffer; // dest buffer
  cpa_conversion_header.cbDstLength = dest_length; // dest buffer size
  cpa_conversion_header.cbDstLengthUsed = 0;
  cpa_conversion_header.dwDstUser = NULL; // no user data
//    cpa_conversion_header.dwReservedDriver; // don't modify ever

  if (acmStreamPrepareHeader(cpa_conversion_stream, &cpa_conversion_header, 0) != MMSYSERR_NOERROR)
  {
    addError(AVIKIT_ERROR_ACM); // "couldn't prep headers"
    acmStreamClose(cpa_conversion_stream, 0);
    return false;
  }

  cpa_blocknum = 0; // start at 0.
  cpa_blockpos = 0;
  cpa_blockoffset = 0;

  AVIStreamRead(audio_stream, cpa_blocknum * cpa_blockalign, cpa_blockalign, cpa_srcbuffer, cpa_blockalign, NULL, NULL);
  acmStreamConvert(cpa_conversion_stream, &cpa_conversion_header, ACM_STREAMCONVERTF_BLOCKALIGN | ACM_STREAMCONVERTF_START);

  // convert first chunk twice. it often fails the first time. BLACK MAGIC.
  AVIStreamRead(audio_stream, cpa_blocknum * cpa_blockalign, cpa_blockalign, cpa_srcbuffer, cpa_blockalign, NULL, NULL);
  acmStreamConvert(cpa_conversion_stream, &cpa_conversion_header, ACM_STREAMCONVERTF_BLOCKALIGN);

  return true;
}

AVIKit::AVIKit(const char *filename, bool load_audio)
{
  AVISTREAMINFO stream_info;

  // default state: non-working.
  active = false;

#ifndef AVIKIT_NOINIT_VFW
  // consider initialising VFW
  if (avikits == 0) // if the reference counter is 0, do so.
    AVIFileInit();

  avikits++;
#endif

  // load the AVI
  LONG hr = AVIFileOpen(&pfile, filename, OF_SHARE_DENY_WRITE, 0L);

  if (hr != 0) // error opening AVI:
  {
    switch (hr)
    {
    case AVIERR_BADFORMAT:
      addError(AVIKIT_ERROR_BADFORMAT); // "corrupt file or unknown format."
      break;
    case AVIERR_MEMORY:
      addError(AVIKIT_ERROR_MEMERROR); // "insufficient memory to open file."
      break;
    case AVIERR_FILEREAD:
      addError(AVIKIT_ERROR_DISKERROR); // "disk error reading file."
      break;
    case AVIERR_FILEOPEN:
      addError(AVIKIT_ERROR_DISKERROR); // "disk error opening file."
      break;
    case REGDB_E_CLASSNOTREG:
    default:
      addError(AVIKIT_ERROR_BADFILE); // "no handler found (or file not found)."
      break;
    }

    return;
  }

  video_stream = audio_stream = NULL;

  // Open the streams until a stream is not available. 
  long opened_streams = 0;
  while (1)
  {
    PAVISTREAM stream = NULL;

    if (AVIFileGetStream(pfile, &stream, 0L, opened_streams++) != AVIERR_OK)
      break;

    if (stream == NULL)
      break;

    AVIStreamInfo(stream, &stream_info, sizeof(stream_info));

    if (stream_info.fccType == streamtypeVIDEO && video_stream == NULL)
    {
      video_stream = stream;
      video_frames = stream_info.dwLength;
      video_xres = stream_info.rcFrame.right - stream_info.rcFrame.left;
      video_yres = stream_info.rcFrame.bottom - stream_info.rcFrame.top;

      video_fps = float(stream_info.dwRate) / float(stream_info.dwScale);
    }
    else if (stream_info.fccType == streamtypeAUDIO && audio_stream == NULL && load_audio)
    {
      audio_stream = stream;

      // read the audio header
      long size;
      AVIStreamReadFormat(audio_stream, AVIStreamStart(audio_stream), 0, &size);

      audio_header = (WAVEFORMAT *)malloc(size);
      AVIStreamReadFormat(audio_stream, AVIStreamStart(audio_stream), audio_header, &size);
      audio_header_size = size;
      audio_codec = audio_header->wFormatTag;

      // length of converted audio in samples
      audio_length = long(float(stream_info.dwLength) / audio_header->nAvgBytesPerSec);
      audio_length *= audio_header->nSamplesPerSec;

      if (audio_codec != WAVE_FORMAT_PCM)
      {
        short bits;

        if (!ACMConvertAudio(audio_stream, audio_header, &bits))
        {
          free(audio_header);
          audio_stream = NULL;
          continue;
        }

        audio_bytes_per_sample = (bits>>3)*audio_header->nChannels;
      }
      else
        audio_bytes_per_sample = audio_header->nBlockAlign;
    }
    else
      AVIStreamRelease(stream);
  }

  // Display error message-stream not found. 
  if (video_stream == NULL)
  {
    if (pfile) // If file is open, close it 
      AVIFileRelease(pfile);

    addError(AVIKIT_ERROR_NOVIDEO); // "couldn't find a valid video stream."

    return;
  }

  AVIFileRelease(pfile); // release the file

  video_getframe = AVIStreamGetFrameOpen(video_stream, NULL); // open the frame getter

	if (video_getframe == NULL)
  {
    addError(AVIKIT_ERROR_VIDEOERROR); // "error attempting to read video frames."

    return; // couldn't open frame getter.
  }
/*
  // printouts!
  AVIStreamInfo(video_stream, &stream_info, sizeof(stream_info));
  printf("Video stream: length %f\n", float(stream_info.dwLength) / video_fps);
  printf("resolution: %d x %d\n", video_xres, video_yres);
  printf("- rate: %f fps\n", video_fps);

  if (audio_stream != NULL)
  {
    AVIStreamInfo(audio_stream, &stream_info, sizeof(stream_info));
    printf("Audio stream: length %f\n", float(stream_info.dwLength) / audio_header->nAvgBytesPerSec);
    printf("- rate: %d Hz\n", audio_header->nSamplesPerSec);
    printf("- format tag: %04X\n", audio_header->wFormatTag);
    printf("- channels: %d\n", audio_header->nChannels);
    printf("- samples per sec: %d\n", audio_header->nSamplesPerSec);
    printf("- bytes per sec: %d\n", audio_header->nAvgBytesPerSec);
    printf("- block size: %d\n", audio_header->nBlockAlign);
    printf("- start: %d\n", stream_info.dwStart);
  }
*/
  active = true; // all set.
}

bool AVIKit::getVideoInfo(long *xres, long *yres, float *duration)
{
  if (!active)
    return false;

  if (xres != NULL)
    *xres = video_xres;

  if (yres != NULL)
    *yres = video_yres;

  if (duration != NULL)
    *duration = (float)video_frames / video_fps;

  return true;
}

// returns a unique frame identifier
long AVIKit::getVideoFrameNumber(float time)
{
  if (!active)
    return 0;

  return (time * video_fps);
}

// gets the raw frame data
void AVIKit::getVideoFrame(char *framedata, long frame)
{
  LPBITMAPINFOHEADER frame_info;
  char *frame_raw;

  if (!active)
    return;

  if (frame >= video_frames)
    frame = video_frames - 1;

  frame_info = (LPBITMAPINFOHEADER)AVIStreamGetFrame(video_getframe, frame);
  frame_raw = (char *)frame_info + frame_info->biSize 
                  + frame_info->biClrUsed * sizeof(RGBQUAD);

  memcpy(framedata, frame_raw, video_xres*video_yres*3*sizeof(char));

  // flip data
  for (long i=0; i<video_xres*video_yres*3; i+=3)
  {
    register char tmp;
    tmp = framedata[i];
    framedata[i] = framedata[i+2];
    framedata[i+2] = tmp;
  }
}

bool AVIKit::getAudioInfo(long *frequency, long *bytes_per_sample, long *channels, bool *is16bit)
{
  if (!active || audio_stream == NULL)
    return false;

  if (frequency != NULL)
    (*frequency) = audio_header->nSamplesPerSec;

  if (bytes_per_sample != NULL)
    (*bytes_per_sample) = this->audio_bytes_per_sample;

  if (channels != NULL)
    (*channels) = audio_header->nChannels;

  if (is16bit != NULL)
  {
    if (audio_codec == WAVE_FORMAT_PCM) // uncompressed audio!
      (*is16bit) = (audio_bytes_per_sample > audio_header->nChannels);
    else
      (*is16bit) = true; // assume compressed audio is always 16 bit
  }

  return true;
}

// sync the current audio read to a specific offset
bool AVIKit::seekPosition(unsigned long offset)
{
  if (offset < cpa_blockoffset) // well, shit. we can't seek backwards... restart
  {
    if (cpa_blockoffset - offset < 500000)
      return false; // don't bother if it's gonna catch up soon (cheap hack! works!)

    cpa_blocknum = 0; // start at 0, eh.
    cpa_blockpos = 0;
    cpa_blockoffset = 0;

    AVIStreamRead(audio_stream, cpa_blocknum * cpa_blockalign, cpa_blockalign, cpa_srcbuffer, cpa_blockalign, NULL, NULL);
    acmStreamConvert(cpa_conversion_stream, &cpa_conversion_header, ACM_STREAMCONVERTF_BLOCKALIGN | ACM_STREAMCONVERTF_START);

    // convert first chunk twice. it often fails the first time. BLACK MAGIC.
    AVIStreamRead(audio_stream, cpa_blocknum * cpa_blockalign, cpa_blockalign, cpa_srcbuffer, cpa_blockalign, NULL, NULL);
    acmStreamConvert(cpa_conversion_stream, &cpa_conversion_header, ACM_STREAMCONVERTF_BLOCKALIGN);
  }

  // now then: seek forwards to the required block
  int breaker = 30; // maximum zero blocks: anti-freeze protection
  while (cpa_blockoffset + cpa_conversion_header.cbDstLengthUsed < offset)
  {
    cpa_blocknum++;
    cpa_blockoffset += cpa_conversion_header.cbDstLengthUsed;

    AVIStreamRead(audio_stream, cpa_blocknum * cpa_blockalign, cpa_blockalign, cpa_srcbuffer, cpa_blockalign, NULL, NULL);
    acmStreamConvert(cpa_conversion_stream, &cpa_conversion_header, ACM_STREAMCONVERTF_BLOCKALIGN);

    if (cpa_conversion_header.cbDstLengthUsed == 0)
      breaker--;
    else
      breaker = 30;

    if (breaker <= 0)
      return false;

    cpa_blockpos = 0;
  }

  // seek to the right position inside the block
  cpa_blockpos = offset - cpa_blockoffset;

  return true;
}

// get a chunk of audio from the stream
void AVIKit::getAudioChunk(char *audiodata, long offset, long length)
{
  // zero data past the end of the file
  if (offset + length > audio_length)
  {
    if (offset <= audio_length)
    {
      long remaining_length = audio_length - offset;

      getAudioChunk(audiodata, offset, remaining_length);

      for (long i=remaining_length*audio_bytes_per_sample;i<length*audio_bytes_per_sample;i++)
        audiodata[i] = 0;
    }
    else
    {
      for (long i=0;i<length*audio_bytes_per_sample;i++)
        audiodata[i] = 0;
    }
  }

  if (audio_codec == WAVE_FORMAT_PCM) // uncompressed audio!
  {
    // very simple - read straight out
    AVIStreamRead(audio_stream, offset, length, audiodata, length*audio_bytes_per_sample, NULL, NULL);
  }
  else // compressed audio!
  {
    long lenread = 0;

    offset *= audio_bytes_per_sample; // convert to bytes
    length *= audio_bytes_per_sample;

    if (!seekPosition(offset)) // seek to correct chunk and all that stuff
      return; // don't continue if we're waiting for the play pointer to catch up

    while (length > 0)
    {
      long blockread = cpa_conversion_header.cbDstLengthUsed - cpa_blockpos;

      if (blockread <= 0) // read next
      {
        cpa_blocknum++;
        cpa_blockoffset += cpa_conversion_header.cbDstLengthUsed;

        AVIStreamRead(audio_stream, cpa_blocknum * cpa_blockalign, cpa_blockalign, cpa_srcbuffer, cpa_blockalign, NULL, NULL);
        acmStreamConvert(cpa_conversion_stream, &cpa_conversion_header, ACM_STREAMCONVERTF_BLOCKALIGN);

        cpa_blockpos = 0;

        continue;
      }

      if (blockread > length)
        blockread = length;

      // copy the data
      memcpy(audiodata + lenread, (void *)(cpa_dstbuffer + cpa_blockpos), blockread);

      cpa_blockpos += blockread;
      lenread += blockread;
      length -= blockread;
    }
  }

  if (audio_bytes_per_sample == audio_header->nChannels)
  {
    // 8-bit data needs to be recentered for fmod
    for (long i=0; i<length;i++)
      audiodata[i] += (char)128;
  }
}

AVIKit::~AVIKit(void) // clean up and leave.
{
  if (active)
  {
    AVIStreamGetFrameClose(video_getframe);

    if (audio_stream != NULL)
    {
      AVIStreamRelease(audio_stream);
      free(audio_header);

      if (audio_codec != WAVE_FORMAT_PCM)
      {
        acmStreamUnprepareHeader(cpa_conversion_stream, &cpa_conversion_header, 0);
        acmStreamClose(cpa_conversion_stream, 0);
        free(cpa_srcbuffer);
        free(cpa_dstbuffer);
      }
    }

    AVIStreamRelease(video_stream);
  }

#ifndef AVIKIT_NOINIT_VFW
  // consider releasing VFW
  avikits--;

  // if the reference counter is down to 0, do so.
  if (avikits == 0)
    AVIFileExit();
#endif
}



// init from the avi itself
AVIKitBuffer::AVIKitBuffer(AVIKit *avi, long s_buffer_size)
{
  this->avi = avi;

  this->s_buffer_size = s_buffer_size;

  this->s_buffer_data_start = 0;
  this->s_buffer_data_end = 0;

  this->s_stream_data_start = 0;
  this->s_stream_data_end = 0;

  avi->getAudioInfo(&frequency, &bytes_per_sample, &channels, NULL);
  
  bytedump = NULL;
}

AVIKitBuffer::~AVIKitBuffer(void)
{
  if (bytedump != NULL)
    free(bytedump);
}

// passthrough to the AVIKit info thing.
bool AVIKitBuffer::getAudioInfo(long *frequency, long *bytes_per_sample, long *channels, bool *is16bit)
{
  return avi->getAudioInfo(frequency, bytes_per_sample, channels, is16bit);
}

// updates internal buffer data, and tells you what you need to fetch.
void AVIKitBuffer::makeUpdate(long playpos, float time, long *s_update_pos, long *s_update_length)
{
  long s_time = long(time * (float)frequency);

  long s_buffer_progress = playpos - s_buffer_data_start;
  if (s_buffer_progress < 0) s_buffer_progress += s_buffer_size;

  // move the start of the buffered section forwards as stuff is no longer needed
  s_buffer_data_start = (s_buffer_data_start + s_buffer_progress) % s_buffer_size;
  s_stream_data_start += s_buffer_progress;

  // update the gap at the end of the buffer...
  *s_update_pos = s_buffer_data_end;
  *s_update_length = s_buffer_progress;

  // and move the end 'pointers' forwards over the gap
  s_buffer_data_end = (s_buffer_data_end + s_buffer_progress) % s_buffer_size;
  s_stream_data_end += s_buffer_progress;

  // possible resync?
  if (abs(s_stream_data_start - s_time) > 2000)
  {
    s_stream_data_start = s_time;
    s_stream_data_end = s_time + s_buffer_size;

    *s_update_pos = s_buffer_data_start; // reload the entire buffer
    *s_update_length = s_buffer_size;
  }
}

// function for fetching raw data from the internal buffer.
void AVIKitBuffer::getUpdate(char *rawbuffer, long s_update_pos, long s_update_length)
{
  int s_stream_update_pos;

  // get the start pos in stream terms, not buffer terms
  s_stream_update_pos = (s_update_pos + s_buffer_size - s_buffer_data_start) % s_buffer_size;
  s_stream_update_pos += s_stream_data_start;

  // and get the data from the stream.
  avi->getAudioChunk(rawbuffer, s_stream_update_pos, s_update_length);
}

void AVIKitBuffer::getUpdate(char *leftbuffer, char *rightbuffer, long s_update_pos, long s_update_length)
{
  int s_stream_update_pos;

  // get the start pos in stream terms, not buffer terms
  s_stream_update_pos = (s_update_pos + s_buffer_size - s_buffer_data_start) % s_buffer_size;
  s_stream_update_pos += s_stream_data_start;

  if (channels == 2) // for stereo, we have to disinterleave stuffs
  {
    long i;

    // ensure we have enough memory for the conversion
    if (bytedump == NULL)
    {
      bytedump_length = 2 * s_update_length * bytes_per_sample; // double the need
      bytedump = (char *)malloc(bytedump_length);
    }
    else if (bytedump_length < s_update_length * bytes_per_sample)
    {
      bytedump_length = 2 * s_update_length * bytes_per_sample; // ...and again
      bytedump = (char *)realloc(bytedump, bytedump_length);
    }

    if (bytes_per_sample == 4) // 16 bit
    {
      avi->getAudioChunk(bytedump, s_stream_update_pos, s_update_length);

      for (i=0;i<s_update_length;i++)
      {
        leftbuffer[2*i+0] = bytedump[4*i+0];
        leftbuffer[2*i+1] = bytedump[4*i+1];
        rightbuffer[2*i+0] = bytedump[4*i+2];
        rightbuffer[2*i+1] = bytedump[4*i+3];
      }
    }
    else // 8 bit
    {
      avi->getAudioChunk(bytedump, s_stream_update_pos, s_update_length);

      for (i=0;i<s_update_length;i++)
      {
        leftbuffer[i] = bytedump[2*i];
        rightbuffer[i] = bytedump[2*i+1];
      }
    }
  }
  else // mono - just duplicate the data.
  {
    avi->getAudioChunk(leftbuffer, s_stream_update_pos, s_update_length);

    memcpy(rightbuffer, leftbuffer, s_update_length * bytes_per_sample);
  }
}
