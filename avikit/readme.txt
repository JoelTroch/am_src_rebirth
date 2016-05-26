AVIKit
------

The AVIKit library is copyright (c) 2003-2004, Ruari O'Sullivan. For
license information, see license.txt.

AVIKit is an open-source library for reading video and audio streams
from AVIs under Windows. It is targeted towards using videos in
virtual worlds, and is capable of seperating interleaved audio
channels for correct spatial representation in virtual worlds (as
demonstrated by the akdemo3d and AKHL sample applications).

Known bugs
----------

AVIKit does not support the Windows Media Audio or Windows Media
Audio v2 codecs. Getting MP3 audio working was quite enough in the
way of non-standard, poorly documented black magic, thank you. Other
codecs will quite naturally work perfectly if they comply fully with
ACM - so they clearly won't work at all. I haven't checked, so YMMV.
All MP3 and uncompressed audio streams should work fine.

Distribution details
--------------------

The AVIKit header, such as it is, is in /include/. The library is
precompiled in /lib/, though the source code is in /src/ should you
wish to change it for some reason.

Source code to the two stand-alone samples is provided inside the
relevant /sample/ folder. Source code to the AKHL sample is not
provided, and will not be provided. No, not even if you ask. No,
not even if you ask really nicely.

Documentation may or may not be available in /doc/, depending upon
how angsty I get over the next few days.

Acknowledgements
----------------

Much respect to David Weekly - david.weekly.org - for the code that
made MP3 support possible. Ta. Also to Sneaky_Bastard, for digging
up all sorts of things.

Finally... thanks to Microsoft, I guess, for leading the pack in
all this "multimedia" stuff we were hearing so much about during
the 90s. VFW made video decompression trivial. ACM... well, it does
the job - though it'd be nice, maybe, if it did it the way MSDN
says it should. Very, very curious.

Samples
-------

- AKDEMO

Controls:

right arrow		skip forwards
left arrow		restart video
space			toggle demo mode
ESC			quit

- AKDEMO3D

Controls:

Arrow keys		move and strafe
Right Ctrl		turn left
Numpad 0		turn right
ESC			quit

- AKHL

The AKHL demo requires Half-Life 1. Possibly version 1.1.0.0 or later,
I'm not sure.

To use the AKHL demo, copy both the AKHL folder and the file "fmod.dll"
to the Half-Life folder (overwriting any older versions of fmod.dll). 
Then, edit playlist.txt in the AKHL folder to reflect the paths of .AVI
files you have on your machine. (It may be wise to test these files in
akdemo or akdemo3d first).

If you don't have any AVI files on your machine, I recommend
http://www.redvsblue.com/.

If an AVI file does not play at all, try it in Windows Media Player
first. If it doesn't even work in there, it isn't my fault and you're
probably just missing the codec.

- randomnine (ro1615@bris.ac.uk)
http://www.bits.bris.ac.uk/randomnine/