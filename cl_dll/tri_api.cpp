//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// Triangle rendering, if any

#include "hud.h"
#include "r_main.h"
#include "r_util.h"

// Triangle rendering apis are in gEngfuncs.pTriAPI
bool notinit = true;
model_s* pWorld;
unsigned int g_texture = 9;
void R_InitWorld( void )
{
	pWorld = gEngfuncs.GetEntityByIndex(0)->model;
	if(pWorld)notinit = false;
	else notinit = true;
}

void R_DrawWorld( void )
{
	return;

	if(notinit)
	{
		R_InitWorld();
		return;
	}

	float	*v;
	glpoly_t	*p;

	if (pWorld)
	{
		for (int i = 0; i < pWorld->numsurfaces; i++) 
		{
			p = pWorld->surfaces[i].polys;
			v = p->verts[0];
                                        
			glBindTexture(GL_TEXTURE_2D, g_texture);
			glColor4f (0.0, 1.0, 0.0, 1.0);
			glBegin(GL_POLYGON);
			for (int d = 0 ; d < p->numverts ; d++, v+= VERTEXSIZE)
			{
				glTexCoord2f (v[3], v[4]);
				glVertex3fv (v);
			}
                              glEnd ();
		}
	}
}

