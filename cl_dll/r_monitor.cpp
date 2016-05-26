//=======================================================================
//			Copyright (C) Shambler Team 2005
//			monitor.cpp - realtime  monitor
//			based on original code from Ryokeen    
//=======================================================================

#include "hud.h"
#include "r_main.h"
#include "r_util.h"
#include "triangleapi.h"
#include "entity_types.h"

#include "studio.h"
#include "r_util.h"
#include "r_studioint.h"
#include "r_studio.h"
extern CGameStudioModelRenderer g_StudioRenderer;

int g_iTotalScreens = 0;
int g_iTotalVisibleScreens = 0;
screen_t *m_pScreenChain = NULL;
screen_t *m_pCurrentScreen = NULL;
extern vec3_t v_origin;
cva *pScr;

//================================
// B&W Shader
//================================
unsigned int fp_screen_shader;

static char fp_screen_source[] = 
"!!ARBfp1.0\n"
"OPTION ARB_precision_hint_fastest;\n"
"PARAM c0 = {0.32000000, 0.59000000, 0.090000000, 0};\n"
"TEMP R0;\n"
"TXP R0, fragment.texcoord[0], texture[0], 2D;\n"
"DP3 result.color, c0, R0;\n"
"MOV result.color.w, fragment.color.w;\n"
"END";

unsigned int fp_noise_shader;

static char fp_noise_source[] = 
"!!ARBfp1.0\n"
"PARAM c[1] = { { 2, 1, 0, 8 } };\n"
"TEMP R0;\n"
"TEX R0.xyz, fragment.texcoord[1], texture[1], 2D;\n"
"MUL R0.xyz, R0, c[0].x;\n"
"MOV R0.w, c[0].z;\n"
"ADD R0.xyz, R0, -c[0].y;\n"
"MAD R0, R0, c[0].w, fragment.texcoord[0];\n"
"TXP result.color.xyz, R0, texture[0], 2D;\n"
"MOV result.color.w, fragment.color.primary;\n"
"END";

typedef struct cl_sbe_s
{
	int index;
	bool initialized;
} cl_sbe_t;
cl_sbe_t g_iScreenBrushEnts[MAX_EDICTS];
typedef struct screen_vertex_s
{
	float point[3];	//NB: this is also texcoord0
	float texcoord0[2];
} screen_vertex_t;
screen_vertex_t g_ScreenVerts[MAX_MIRROR_VERTS];	//static 72kb array for screen verts
int g_ScreenVertIndex = 0;
int g_iScreenBrushEntCount = 0;

void R_InitScreens(void)
{
	pScr = cvaCreateFromMemory(GL_TRIANGLES, MAX_MIRROR_VERTS, CVF_VERTEX|CVF_TEX0COORD2, sizeof(screen_vertex_t), &g_ScreenVerts[0]);
	assert(pScr != NULL);
}

void AddScreenBrushEntity( int idx )
{
	g_iScreenBrushEnts[g_iScreenBrushEntCount].index = idx;
	g_iScreenBrushEnts[g_iScreenBrushEntCount].initialized = false;
	g_iScreenBrushEntCount++;
}

void R_FreeScreens(void)
{
	screen_t *scr = m_pScreenChain;
	screen_t *temp;
	while (scr)
	{
		temp = scr->next;
		delete scr;
		scr = temp;
	}
	m_pScreenChain = NULL;

	g_iTotalScreens = 0;
	g_iTotalVisibleScreens = 0;
	g_iScreenBrushEntCount = 0;

	memset(&g_iScreenBrushEnts[0], 0, MAX_EDICTS * sizeof(cl_sbe_t));
}

void R_ResetScreens(void)
{
	R_FreeScreens();
}

void CreateScreenShader_ARB(void)
{
	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	if (!fp_screen_shader) glGenProgramsARB(1, &fp_screen_shader);

	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fp_screen_shader);
	glProgramStringARB( GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, sizeof(fp_screen_source)-1, fp_screen_source);

	int isNative = 0;
	glGetProgramivARB( GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isNative);

	if (!isNative) logPrint("WARNING: Fragment program is above native limits! (fp_screen_arb)\n");
	glDisable(GL_FRAGMENT_PROGRAM_ARB);
}

void DeleteScreenShader_ARB(void)
{
	if (fp_screen_shader) glDeleteProgramsARB(1, &fp_screen_shader);
}

void BindScreenShader_ARB(screen_t *scr) 
{
	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, fp_screen_shader );
}

void UnbindScreenShader_ARB(screen_t *scr) 
{
	glDisable(GL_FRAGMENT_PROGRAM_ARB);
}

screen_t *R_AllocateScreen(msurface_t *surf)
{
	screen_t *scr = new screen_t;
	scr->enabled = true;
	scr->visible = true;
	scr->surface = surf;
	scr->index = g_iTotalScreens;

	scr->ent = NULL;
	scr->sv_entindex = 0;

	//calc radius

	glpoly_t *p = surf->polys;
	vec3_t mins, maxs, size;
	mins[0] = mins[1] = mins[2] = 9999.0f;
	maxs[0] = maxs[1] = maxs[2] = -9999.0f;
	
	scr->firstvertex = g_ScreenVertIndex;
	
	while (p)
	{
		float* vert = p->verts[0];
		for (int v = 0; v < p->numverts; v++, vert+=VERTEXSIZE)
		{
			if (vert[0] <= mins[0]) mins[0] = vert[0];
			if (vert[1] <= mins[1]) mins[1] = vert[1];
			if (vert[2] <= mins[2]) mins[2] = vert[2];
			
			if (vert[0] >= maxs[0]) maxs[0] = vert[0];
			if (vert[1] >= maxs[1]) maxs[1] = vert[1];
			if (vert[2] >= maxs[2]) maxs[2] = vert[2];

			VectorCopy(vert, g_ScreenVerts[g_ScreenVertIndex].point);
			g_ScreenVerts[g_ScreenVertIndex].texcoord0[0] = vert[3];
			g_ScreenVerts[g_ScreenVertIndex].texcoord0[1] = vert[4];
			g_ScreenVertIndex++;
		}
		p = p->next;
	}

	scr->numvertices = g_ScreenVertIndex - scr->firstvertex;
	scr->origin[0] = (mins[0] + maxs[0]) * 0.5f;
	scr->origin[1] = (mins[1] + maxs[1]) * 0.5f;
	scr->origin[2] = (mins[2] + maxs[2]) * 0.5f;

	size = maxs - mins;

	if ((size.x >= size.y) && (size.x >= size.z)) scr->radius = size.x;
	else if ((size.y >= size.x) && (size.y >= size.z)) scr->radius = size.y;
	else scr->radius = size.z;

	scr->next = m_pScreenChain;
	m_pScreenChain = scr;

	scr->texture = pTexId++;
	glBindTexture(GL_TEXTURE_2D, scr->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	g_iTotalScreens++;
	return m_pScreenChain;
}

screen_t *R_ScreenForBrush(msurface_t *surf, cl_entity_t *e )
{
	screen_t *scr;
          cl_entity_t *cam;
          
	scr = R_AllocateScreen(surf);
	scr->ent = e;
	scr->sv_entindex = e->curstate.colormap;
	scr->enabled = (e->curstate.body>0);
	scr->color = (e->curstate.skin>0);
          scr->cam_idx = e->curstate.team;
          scr->alpha = e->curstate.renderamt / 255.0f;//alpha

	cam = UTIL_GetClientEntityWithServerIndex(scr->cam_idx);
	if(cam)//get new camera pos
	{
		VectorCopy( cam->origin, scr->cam_origin);
		VectorCopy( cam->angles, scr->cam_angles);
	}
	return scr;
}

void R_CreateScreensForBrush(cl_entity_t *e )
{
	model_t* mod = e->model;
	msurface_t* pSurf = &mod->surfaces[mod->firstmodelsurface];

	// go through each surface
	for (int i = 0; i < mod->nummodelsurfaces; i++)
	{
		if (pSurf[i].texinfo->flags & SURF_SCREEN)
		{
			R_ScreenForBrush(&pSurf[i], e );
		}
	}
}

void R_InitBrushScreensForFrame( void )
{
	//here, we get cl_entity ptr for server index
	//if it is already allocated, we suppose this screen is initialized
	//if not, we try to allocate screen for each brush side

	int i;
	cl_entity_t *e;

	for (i = 0; i < g_iScreenBrushEntCount; i++)
	{
		if (g_iScreenBrushEnts[i].initialized)
			continue;
		if (g_iScreenBrushEnts[i].index <= 0)
			break;

		//we got a server index, find a corresponding client entity
		e = UTIL_GetClientEntityWithServerIndex(g_iScreenBrushEnts[i].index);
		if (!e) continue;

		R_CreateScreensForBrush(e);

		g_iScreenBrushEnts[i].index = 0;
		g_iScreenBrushEnts[i].initialized = true;
	}
}

void R_InitScreensForFrame( void )
{
	R_InitBrushScreensForFrame();

	g_iTotalVisibleScreens = 0;

	screen_t *scr = m_pScreenChain;
	while (scr)
	{
		scr->enabled = (scr->ent->curstate.body > 0);
		scr->visible = (scr->ent->curstate.msg_time == realtime);

		if (scr->visible)
		{
			float dv[3];
			VectorSubtract(scr->origin, v_origin, dv);
			float d = Length(dv);

			if (fabs(d) > 36.0f)
			{
				scr->visible = (R_SphereInFrustum(scr->origin, scr->radius) > 0);
			}
		}

		if (!scr->visible || !scr->enabled || !scr->cam_idx)
		{
			scr = scr->next;
			continue;
		}

		g_iTotalVisibleScreens++;
		scr = scr->next;
	}
}

void R_NewScreenRenderPass( void )
{
	if (m_pCurrentScreen)
	{
		glBindTexture(GL_TEXTURE_2D, m_pCurrentScreen->texture);
		if(m_pCurrentScreen->color == 0 && !g_HardwareShaderCapable)//support for old video cards
			glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8, 0, ScreenHeight - VIEWPORT_SIZE, VIEWPORT_SIZE, VIEWPORT_SIZE, 0);
		else	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, ScreenHeight - VIEWPORT_SIZE, VIEWPORT_SIZE, VIEWPORT_SIZE, 0);
		
	}
}

void R_SetupNewScreen(ref_params_t *pparams)
{
	if (!m_pCurrentScreen)
	{
		assert(m_pCurrentScreen != NULL);
		m_pCurrentScreen = m_pScreenChain;

		if (m_pCurrentScreen->visible && m_pCurrentScreen->enabled)
			return;
	}

	screen_t *scr = m_pCurrentScreen->next;
	while (scr)
	{
		if (!scr->visible || !scr->enabled || !scr->cam_idx)
		{
			scr = scr->next;
			continue;
		}
		m_pCurrentScreen = scr;
		break;
	}

	assert(m_pCurrentScreen != NULL);
}

void R_SetupScreenRenderPass(ref_params_t *pparams)
{
          VectorCopy( m_pCurrentScreen->cam_angles, pparams->viewangles );
	VectorCopy( m_pCurrentScreen->cam_origin, pparams->vieworg );
}

void R_SetupScreenShader(screen_t *scr) 
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	//GL_SelectTexture(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D,scr->texture);
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	if (scr->alpha < 1)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		GLSTATE_ENABLE_BLEND
	}
	if (scr->color == 0 && g_HardwareShaderCapable)
	{
		BindScreenShader_ARB(scr);
	}
}

void R_DisableScreenShader(screen_t *scr) 
{
	if (scr->alpha < 1)
	{
		GLSTATE_DISABLE_BLEND
	}
	if (scr->color == 0 && g_HardwareShaderCapable)
	{
		UnbindScreenShader_ARB(scr);
	}
	glPopAttrib();
}

void R_RenderScreens( bool trans )
{
	if ((g_iTotalVisibleScreens <= 0) || g_bFinalPass)
	{
		glGetFloatv (GL_MODELVIEW_MATRIX, r_world_matrix);
		glGetFloatv (GL_PROJECTION_MATRIX, r_projection_matrix);
		GL_ExtractFrustum();
	}

	screen_t *scr = m_pScreenChain;

	pScr->bind(CVF_ALL);
	pScr->lock();
          
	glPolygonOffset(-1,-2);
	glEnable(GL_POLYGON_OFFSET_FILL);

	while (scr)
	{
		if (!scr->visible || !scr->enabled )
		{
			scr = scr->next;
			continue;
		}
		if (((scr->alpha < 1) && !trans) || ((scr->alpha == 1) && trans))
		{
			scr = scr->next;
			continue;
		}

		R_SetupScreenShader(scr);//grayscale shader
		glColor4f(0.8f, 0.8f, 0.8f, 0.8f);

		GLenum facetype;

		switch(scr->surface->polys->numverts) 
		{
			case 3:	facetype = GL_TRIANGLES;	break;
			case 4:	facetype = GL_QUADS;	break;
			default:	facetype = GL_POLYGON;	break;
		}

		//draw all screens
                    pScr->drawTypeRange(facetype, scr->firstvertex, scr->numvertices);
		R_DisableScreenShader(scr);
		
		scr = scr->next;
	}
          
	glDisable(GL_POLYGON_OFFSET_FILL);
	pScr->unlock();
	pScr->unbind();

	glFlush();
}

void R_DrawScreen(bool trans)
{
	if(trans) R_RenderScreens(true);
	else if(!g_bSkyPass && !g_bMirrorPass)
		R_RenderScreens(false);
}