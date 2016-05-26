/***
*
*	Copyright (c) 2001-2006, Chain Studios. All rights reserved.
*
*	This product contains software technology that is a part of Half-Life FX (R)
*	from Chain Studios ("HLFX Technology"). All rights reserved.
*
*	Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Chain Studios.
*
****/

//
// Application log functions
//
#include "hud.h"
#include "r_main.h"
#include <time.h>

int log_enabled = 1;
FILE *log_file = NULL;
bool WinNT = false;

void logPrintOpenGLInformation( void )
{
	char *extensionbuf;
	char *ptr;
	int	 c,i,j;

	if (log_enabled==0)
		return;

	fprintf(log_file, "\n----- OpenGL Information -----\n");


	fprintf(log_file,"\nGL_VENDOR:\t");
	fprintf(log_file, (char*)glGetString(GL_VENDOR));
	fprintf(log_file,"\nGL_RENDERER:\t");
	fprintf(log_file, (char*)glGetString(GL_RENDERER));
	fprintf(log_file,"\nGL_VERSION:\t");
	fprintf(log_file, (char*)glGetString(GL_VERSION));
	fprintf(log_file,"\nGL_EXTENSIONS:\n\n");

	//write extensions. as soon as this string can be _very_ huge, split it
	extensionbuf = new char[1024];
	ptr = (char*)glGetString(GL_EXTENSIONS);
	c = strlen(ptr);
	i = 0;

	while (i < c)
	{
		strncpy(extensionbuf,ptr+i,1023);
		extensionbuf[1023] = 0;

		for (j=0;j<1023;j++)
			if (extensionbuf[j] == ' ') extensionbuf[j]='\n';

		fprintf(log_file,extensionbuf);
		memset(extensionbuf,0,1024);
		i+=1023;
	}
	fprintf(log_file,"\n\n");
	delete[] extensionbuf;


	//Show some GL caps

		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &c);
		logPrintf("GL_MAX_TEXTURE_SIZE                = %d\n",c);
	
	if (GLEXT_CheckExtension("GL_EXT_texture3D"))
	{
		glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE_EXT, &c);
		logPrintf("GL_MAX_3D_TEXTURE_SIZE_EXT         = %d\n",c);
	}

	if (GLEXT_CheckExtension("GL_EXT_texture_cube_map") || GLEXT_CheckExtension("GL_ARB_texture_cube_map") )
	{
		glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB, &c);
		logPrintf("GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB   = %d\n",c);
	}

	if (GLEXT_CheckExtension("GL_NV_texture_rectangle"))
	{
		glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE_NV, &c);
		logPrintf("GL_MAX_RECTANGLE_TEXTURE_SIZE_NV   = %d\n",c);
	}

	if (GLEXT_CheckExtension("GL_ARB_multitexture"))
	{
		glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &c);
		logPrintf("GL_MAX_TEXTURE_UNITS_ARB           = %d\n",c);
	}

	if (GLEXT_CheckExtension("GL_NV_vertex_program"))
	{
		glGetIntegerv(GL_MAX_TRACK_MATRICES_NV, &c);
		logPrintf("GL_MAX_TRACK_MATRICES_NV           = %d\n",c);

		glGetIntegerv(GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV, &c);
		logPrintf("GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV = %d\n",c);
	}

	if (GLEXT_CheckExtension("GL_NV_register_combiners"))
	{
		glGetIntegerv(GL_MAX_GENERAL_COMBINERS_NV, &c);
		logPrintf("GL_MAX_GENERAL_COMBINERS_NV        = %d\n",c);
	}

	if (GLEXT_CheckExtension("GL_EXT_texture_filter_anisotropic"))
	{
		glGetIntegerv(GL_TEXTURE_MAX_ANISOTROPY_EXT, &c);
		logPrintf("GL_TEXTURE_MAX_ANISOTROPY_EXT      = %d\n",c);

		glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &c);
		logPrintf("GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT  = %d\n",c);
	}

	fprintf(log_file, "\n\n");
}

void logInitLog( char *filename )
{
	char pfilename[256];
	char ctime[128];
	time_t t;
	struct tm *tmp;
	OSVERSIONINFO	vinfo;

	if (log_enabled==0)
		return;

	strcpy(pfilename,gEngfuncs.pfnGetGameDirectory());
	strcat(pfilename,"\\");
	strcat(pfilename,filename);

	log_file = fopen( pfilename, "wt");
	assert(log_file != NULL);

	t = time(NULL);
	tmp = localtime(&t);
	strftime(ctime,127,"%H:%M:%S %d/%B/%Y %A",tmp);

	fprintf(log_file,"=================================================================\n");
	fprintf(log_file,"= Half-Life: Arrangement started at %s \n",ctime);
	fprintf(log_file,"=================================================================\n");

	
	fprintf(log_file, "\n----- Staring up -----\n");

	vinfo.dwOSVersionInfoSize = sizeof(vinfo);
	if (!GetVersionEx (&vinfo))
	{
		fprintf(log_file, "ERROR: Couldn't get OS info!");
	}
	else
	{

		logPrintf ("OS Version:\t%d.%d.%d",
					vinfo.dwMajorVersion,
					vinfo.dwMinorVersion,
					vinfo.dwBuildNumber);

		if (vinfo.szCSDVersion[0] != 0)
			logPrintf (" (%s)\n",vinfo.szCSDVersion);
		else
			fprintf(log_file,"\n");

		if ((vinfo.dwMajorVersion < 4) ||
			(vinfo.dwPlatformId == VER_PLATFORM_WIN32s))
		{
			fprintf(log_file,"WARNING: Program requires at least Win95 or NT 4.0");
		}

		if (vinfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			WinNT = true;
			fprintf(log_file,"Windows NT/2000/XP detected\n");
		}
		else
		{
			WinNT = false;
			fprintf(log_file,"Windows 95/98/Me detected\n");
		}
	}
	fprintf(log_file, "\n\n");

	//Add your startup logging here
}

void logCloseLog( )
{
	char ctime[128];
	time_t t;
	struct tm *tmp;

	if (log_enabled==0)
		return;

	if (log_file)
	{
		t = time(NULL);
		tmp = localtime(&t);
		strftime(ctime,127,"%H:%M:%S %d/%B/%Y %A",tmp);

		fprintf(log_file,"=================================================================\n");
		fprintf(log_file,"= Half-Life: Arrangement shutdown at %s \n",ctime);
		fprintf(log_file,"=================================================================\n");

		fclose(log_file);
	}
	
}

void logPrint( char *str )
{
	if (log_enabled==0)
		return;

	if (log_file)
		fprintf(log_file, "%s", str);
}

void logPrintf( char *fmt, ...)
{
	va_list		argptr;
	char		*string;

	if (log_enabled==0)
		return;

	string = new char[8192]; //8 kbytes
	assert( string != NULL );

	va_start (argptr,fmt);
	vsprintf (string, fmt,argptr);
	va_end (argptr);
	
	if (log_file)
		fprintf(log_file, "%s", string);

	delete[] string;
}