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
#pragma once
#define WIN32_LEAN_AND_MEAN
#define GLEW_STATIC
#include "windows.h"
#include "glew.h"
#include <gl\glu.h>
#include "cva.h"
#include "ref_params.h"
#include "com_model.h"

#pragma warning(disable:4244)

#define SAFE_GET_PROC( func, type, name)     if (!func) func = (type) wglGetProcAddress( name ); \
											 assert(func != NULL)

#define VIEWPORT_SIZE		512
#define GL_TEXTURE_NUM_BASE		(1<<25)

#define SURF_PLANEBACK		2
#define SURF_DRAWTURB		0x10

#define SURF_MIRROR			(1<<16)
#define SURF_WARPMIRROR		(1<<17)
#define SURF_SCREEN			(1<<18)

// 0-2 are axial planes
#define	PLANE_X			0
#define	PLANE_Y			1
#define	PLANE_Z			2

//Assume we will have maximum 512 mirrors with 4 verts each
//Increase it if I'm wrong...
#define	MAX_MIRROR_VERTS	2048

//We need the following extensions:

//ARB_multitexture
extern PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
extern PFNGLMULTITEXCOORD1FARBPROC glMultiTexCoord1fARB;
extern PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD2FVARBPROC glMultiTexCoord2fvARB;
extern PFNGLMULTITEXCOORD3FARBPROC glMultiTexCoord3fARB;
extern PFNGLMULTITEXCOORD3FVARBPROC glMultiTexCoord3fvARB;
extern PFNGLMULTITEXCOORD4FARBPROC glMultiTexCoord4fARB;
extern PFNGLMULTITEXCOORD4FVARBPROC glMultiTexCoord4fvARB;

//EXT_texture_3d
extern PFNGLTEXIMAGE3DEXTPROC glTexImage3DEXT;

//ARB_texture_cube_map

//ARB_vertex_program
//ARB_fragment_program
extern PFNGLPROGRAMSTRINGARBPROC glProgramStringARB;
extern PFNGLBINDPROGRAMARBPROC glBindProgramARB;
extern PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB;
extern PFNGLGENPROGRAMSARBPROC glGenProgramsARB;
extern PFNGLGETPROGRAMIVARBPROC glGetProgramivARB;
extern PFNGLGETPROGRAMSTRINGARBPROC glGetProgramStringARB;
extern PFNGLISPROGRAMARBPROC glIsProgramARB;

//NV_register_combiners
extern PFNGLCOMBINERPARAMETERFVNVPROC glCombinerParameterfvNV;
extern PFNGLCOMBINERPARAMETERIVNVPROC glCombinerParameterivNV;
extern PFNGLCOMBINERPARAMETERFNVPROC glCombinerParameterfNV;
extern PFNGLCOMBINERPARAMETERINVPROC glCombinerParameteriNV;
extern PFNGLCOMBINERINPUTNVPROC glCombinerInputNV;
extern PFNGLCOMBINEROUTPUTNVPROC glCombinerOutputNV;
extern PFNGLFINALCOMBINERINPUTNVPROC glFinalCombinerInputNV;

//ATI_pn_triangles
extern PFNGLPNTRIANGLESIATIPROC glPNTrianglesiATI;
extern PFNGLPNTRIANGLESFATIPROC glPNTrianglesfATI;

inline bool GLEXT_CheckExtension(const char *ext)
{
	return (strstr((const char *)glGetString(GL_EXTENSIONS), ext) != NULL);
}

bool GLEXT_Setup_ARB_multitexture();
bool GLEXT_Setup_EXT_texture_3d();
bool GLEXT_Setup_ARB_texture_cubemap();
bool GLEXT_Setup_ARB_vertex_program();
bool GLEXT_Setup_ARB_fragment_program();
bool GLEXT_Setup_NV_register_combiners();
bool GLEXT_Setup_ATI_pn_triangles();

void GL_Init( void );
void GL_VidInit( void );
void GL_Shutdown( void );
void GL_PreRender( void );
void GL_Render( bool trans );
void GL_ExtractFrustum( void );
void GL_MarkTextures( model_t* mod );
void R_DrawWorld( void );//test function

//Utils
float R_SphereInFrustum( vec3_t o, float radius );
cl_entity_t *UTIL_GetClientEntityWithServerIndex( int sv_index );

extern bool g_HardwareCapable;
extern bool g_HardwareShaderCapable;
extern bool g_HardwareTruformCapable;

extern int g_iTotalMirrors;
extern int g_iTotalVisibleMirrors;
extern int g_iTotalScreens;
extern int g_iTotalVisibleScreens;
extern double realtime;

extern bool g_FirstFrame;
extern bool g_RenderReady;
extern bool g_bFinalPass;
extern bool g_bEndCalc;
extern int m_RenderRefCount;   //refcounter (use for debug)

//passes info
extern bool g_bMirrorShouldpass;
extern bool g_bScreenShouldpass;
extern bool g_bSkyShouldpass;
extern bool g_bMirrorPass;
extern bool g_bScreenPass;
extern bool g_bSkyPass;

//base origin and angles
extern vec3_t g_vecBaseOrigin;       //base origin - transformed always
extern vec3_t g_vecBaseAngles;       //base angles - transformed always
extern vec3_t g_vecCurrentOrigin;	//current origin
extern vec3_t g_vecCurrentAngles;    //current angles

extern float r_projection_matrix[16];
extern float r_world_matrix[16];

extern unsigned int g_uiNoiseTex;
extern unsigned int g_uiNoiseTexDsDt;
extern unsigned int pTexId;

typedef struct mirror_s
{
	int index;
	bool enabled;
	bool visible;
	float origin[3];
	float angles[3];
	float radius;
	float alpha;

	float pr[16];
	float mv[16];

	bool water;
	bool twoside;

	//CVA-related
	int firstvertex;
	int numvertices;

	unsigned int texture;
	msurface_t *surface;

	int sv_entindex;
	cl_entity_t *ent;		//for brush model mirrors

	struct mirror_s *next;
}mirror_t;

typedef struct screen_s
{
	int index;
	bool enabled;
	bool visible;
	float origin[3];
	float radius;
	float alpha;

	//current camera member
	int cam_idx;
	float cam_origin[3];
	float cam_angles[3];

	float pr[16];
		
	//CVA-related
	int firstvertex;
	int numvertices;

	unsigned int texture;
	msurface_t *surface;

	int sv_entindex;
	cl_entity_t *ent;
	bool color;

	struct screen_s *next;
}screen_t;

extern cvar_t *r_debug;	//show renderer info
extern cvar_t *r_shadows;	//original HL shadows
extern cvar_t *r_mirrors;	//mirrors
extern cvar_t *r_screens;	//screens
extern cvar_t *gl_water;
extern cvar_t *gl_waterscale;

extern GLuint fp_mirror_arb_object;

mirror_t *R_AllocateMirror(msurface_t *surf);
void R_FreeMirrors(void);
void R_InitMirrors(void);
void R_SetupMirrorRenderPass(ref_params_t *pparams);
void R_SetupNewMirror(ref_params_t *pparams);
void R_NewMirrorRenderPass(void);
void R_RenderMirrors(bool trans);
void R_InitMirrorsForFrame(void);
void R_ResetMirrors(void);
void AddMirrorBrushEntity( int idx );
void R_DrawMirror(bool trans);

screen_t *R_AllocateScreen(msurface_t *surf);
void R_ResetScreens( void );
void R_FreeScreens( void );
void R_InitScreens( void );
void R_RenderScreens( bool trans );
void AddScreenBrushEntity( int idx );
void R_SetupScreenRenderPass(ref_params_t *pparams);
void R_SetupNewScreen(ref_params_t *pparams);
void R_NewScreenRenderPass( void );
void R_InitScreensForFrame(void);
void R_DrawScreen(bool trans);

//CG
void BindVertexProgramForMirror();
void UnbindVertexProgramForMirror();
void BindFragmentProgramForMirror();
void UnbindFragmentProgramForMirror();
bool InitializeCg( void );

//Noise
void CreateNoiseTexture3D (float scale);
void CreateNoiseTextureDsDt (float scale);

//Water
void CreateWaterShader_ARB(void);
void DeleteWaterShader_ARB(void);
void BindWaterShader_ARB(mirror_t *mir);
void UnbindWaterShader_ARB(void);

//Black & white screen
void CreateScreenShader_ARB(void);
void DeleteScreenShader_ARB(void);

//Logging
void logInitLog( char *filename );
void logCloseLog();
void logPrintOpenGLInformation( void );
void logPrint( char *str );
void logPrintf( char *fmt, ...);

extern mirror_t *m_pCurrentMirror;
extern screen_t *m_pCurrentScreen;

//GL state management

struct glstate_curstate_t
{
	unsigned char
		blend			: 1,
		depth_test		: 1,
						: 6;
};

struct glstate_curstagestate_t
{
	unsigned char
		texture2d		: 1,
		texture3d		: 1,
						: 6;
};

typedef struct 
{
	struct glstate_curstate_t curstate;
	struct glstate_curstagestate_t curstagestate[8];	//assume 8 will be max texture stages
	unsigned int current_texture_stage;
} glstate_t;

extern glstate_t glstate;

#define GLSTATE_ENABLE_BLEND		if(!glstate.curstate.blend) { glEnable(GL_BLEND); glstate.curstate.blend=1; }
#define GLSTATE_ENABLE_DEPTH_TEST	if(!glstate.curstate.depth_test) { glEnable(GL_DEPTH_TEST); glstate.curstate.depth_test=1; }
#define GLSTATE_ENABLE_TEXTURE		if(!glstate.curstagestate[glstate.current_texture_stage].texture2d) { glEnable(GL_TEXTURE_2D); glstate.curstagestate[glstate.current_texture_stage].texture2d=1; }
#define GLSTATE_ENABLE_TEXTURE3D	if(!glstate.curstagestate[glstate.current_texture_stage].texture3d) { glEnable(GL_TEXTURE_3D); glstate.curstagestate[glstate.current_texture_stage].texture3d=1; }
#define GLSTATE_DISABLE_BLEND		if(glstate.curstate.blend) { glDisable(GL_BLEND); glstate.curstate.blend=0; }
#define GLSTATE_DISABLE_DEPTH_TEST	if(glstate.curstate.depth_test) { glDisable(GL_DEPTH_TEST); glstate.curstate.depth_test=0; }
#define GLSTATE_DISABLE_TEXTURE		if(glstate.curstagestate[glstate.current_texture_stage].texture2d) { glDisable(GL_TEXTURE_2D); glstate.curstagestate[glstate.current_texture_stage].texture2d=0; }
#define GLSTATE_DISABLE_TEXTURE3D	if(glstate.curstagestate[glstate.current_texture_stage].texture3d) { glDisable(GL_TEXTURE_3D); glstate.curstagestate[glstate.current_texture_stage].texture3d=0; }

void GL_SelectTexture( GLenum mode );
