/*Ryokeen --------- Physic Ropes --------- */
/*Most of the physic code is from NeHe tuts*/

//TO DO:
//Nice code, but why it crashes? Try to create a new rope and the game will crash. 
//Maybe I forgot something to add.

#include "hud.h"
#include "cl_util.h"

// Triangle rendering apis are in gEngfuncs.pTriAPI

#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"


#include "pm_defs.h"
#include "event_api.h"
#include "pmtrace.h"						

#include "pys_rope.h"

extern vec3_t v_angles,v_origin;

void VectorAngles( const float *forward, float *angles );

void GLRopeRender::DrawBeam(vec3_t start,vec3_t end,float width,char *Sprite)
{
	HSPRITE texture = SPR_Load(Sprite);

	struct model_s * pModel;

	pModel = (struct model_s *)gEngfuncs.GetSpritePointer( texture );

	if(!gEngfuncs.pTriAPI->SpriteTexture(pModel, 0))
	return;

	vec3_t dir = (start-end).Normalize();
	vec3_t view,v_right,mid;

	gEngfuncs.GetViewAngles(view);
	AngleVectors(view, view, NULL, NULL);
	view = view.Normalize();

	float l = (start-end).Length();
	mid = start+(start-end).Normalize()*l*0.5;

	view = (v_origin - mid).Normalize();

	v_right = CrossProduct(view,dir);

	v_right = v_right.Normalize();

	gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
	gEngfuncs.pTriAPI->CullFace(TRI_NONE);

	gEngfuncs.pTriAPI->Begin( TRI_TRIANGLES );

	gEngfuncs.pTriAPI->Color4f(1,1,1,1);
	gEngfuncs.pTriAPI->Brightness(1);

	gEngfuncs.pTriAPI->TexCoord2f(0, 1);
	gEngfuncs.pTriAPI->Vertex3fv(end - v_right * width);

	gEngfuncs.pTriAPI->TexCoord2f(1, 1);
	gEngfuncs.pTriAPI->Vertex3fv(end + v_right * width);

	gEngfuncs.pTriAPI->TexCoord2f(1, 0);
	gEngfuncs.pTriAPI->Vertex3fv(start + v_right * width);


		//Repeats for triangles
	gEngfuncs.pTriAPI->TexCoord2f(0, 1);
	gEngfuncs.pTriAPI->Vertex3fv(end - v_right * width);

	gEngfuncs.pTriAPI->TexCoord2f(1, 0);
	gEngfuncs.pTriAPI->Vertex3fv(start + v_right * width);
		//End repeats
		
	gEngfuncs.pTriAPI->TexCoord2f(0, 0);
	gEngfuncs.pTriAPI->Vertex3fv(start - v_right * width);


	gEngfuncs.pTriAPI->End();
	gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
}

GLRopeRender gRopeRender;

GLRopeRender::GLRopeRender()
{
	num_pys_rope = 0;
}
GLRopeRender::~GLRopeRender()
{
}

void Simulation::test_collision(void)
{
	pmtrace_t pmtrace;
	vec3_t force;
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);

	vec3_t pos,oldpos;
	vec3_t diff;
	for(int i = 0; i < numOfMasses; i++)
	{
		vec3_t prevpos;
		prevpos = masses[i]->pos - masses[i]->vel * 0.1;						// Change in position is added to the position.

		float R = Length(masses[i]->pos-prevpos);

		gEngfuncs.pEventAPI->EV_PushPMStates();
	
		// Now add in all of the players.
		gEngfuncs.pEventAPI->EV_SetSolidPlayers ( -1 );	

		gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
		gEngfuncs.pEventAPI->EV_PlayerTrace( masses[i]->oldpos, masses[i]->pos, PM_STUDIO_BOX, -1, &pmtrace );


	//	pmtrace = *(gEngfuncs.PM_TraceLine( masses[i]->oldpos, masses[i]->pos, PM_TRACELINE_ANYVISIBLE, 2, -1 ));

		if(pmtrace.fraction != 1)
		{
			double proj = DotProduct(masses[i]->vel, pmtrace.plane.normal);
			VectorMA(masses[i]->vel, -proj*2.0f, pmtrace.plane.normal,masses[i]->vel);

			diff = masses[i]->oldpos-pmtrace.endpos;
			VectorNormalize(diff);

			masses[i]->vel = masses[i]->vel * 0.9;

		//	force = diff*7000;

			masses[i]->pos = pmtrace.endpos+diff*2;
		//	masses[i]->applyForce(force*10000000);

		//	gEngfuncs.Con_Printf("Collision !!\n");
		}
		//gEngfuncs.Con_Printf("fraction %f!!\n",pmtrace.fraction);

		gEngfuncs.pEventAPI->EV_PopPMStates();
	}
}

void Spring::solve()
{
		vec3_t springVector;
		springVector = mass1->pos - mass2->pos;
		float r = springVector.Length();
	
		vec3_t force;//force initially has a zero value
		
		if (r != 0)	
		{
			force = ((springVector / r) * (r-springLength) * (springConstant));

			force =  force - ((mass1->vel - mass2->vel) * -frictionConstant);

			mass1->applyForce(-force);													//force is applied to mass1
			mass2->applyForce(force);	
		}
}


extern vec3_t v_origin;

void GLRopeRender::CreateRope(char *datafile,cl_entity_t *start_source,cl_entity_t *end_source)
{
	char file[1024];

	if(start_source == NULL)
	{
		gEngfuncs.Con_Printf("No Start_Ent\n");
		return;
	}

	if(end_source == NULL)
	{
		gEngfuncs.Con_Printf("No End_Ent\n");
		return;
	}

	gEngfuncs.Con_Printf("CreateRope\n" );

	m_Ropes[num_pys_rope].can_collide = false;

	sprintf( file, "physic/ropes/%s.rope", datafile);

	char *szFile = (char *)gEngfuncs.COM_LoadFile( file, 5, NULL);
	char szToken[1024];

	if (!szFile)
	{
		gEngfuncs.Con_Printf("Couldn't open Rope file %s for simulation\n", file );
		return;
	}
	else
	{
		szFile = gEngfuncs.COM_ParseFile(szFile, szToken);

		while (szFile)
		{
			if ( !stricmp( szToken, "num_segments" ) )
			{
				szFile = gEngfuncs.COM_ParseFile(szFile,szToken);
				m_Ropes[num_pys_rope].num_segments = atof(szToken);
			}
			else if ( !stricmp( szToken, "rope_lenght" ) )
			{
				szFile = gEngfuncs.COM_ParseFile(szFile,szToken);
				m_Ropes[num_pys_rope].segment_lenght = atof(szToken);
			}
			else if ( !stricmp( szToken, "scale" ) )
			{
				szFile = gEngfuncs.COM_ParseFile(szFile,szToken);
				m_Ropes[num_pys_rope].scale = atof(szToken);
			}
			else if ( !stricmp( szToken, "constant" ) )
			{
				szFile = gEngfuncs.COM_ParseFile(szFile,szToken);
				m_Ropes[num_pys_rope].springconst = atof(szToken);
			}
			else if ( !stricmp( szToken, "mass" ) )
			{
				szFile = gEngfuncs.COM_ParseFile(szFile,szToken);
				m_Ropes[num_pys_rope].mass = atof(szToken);
			}
			else if ( !stricmp( szToken, "springfriction" ) )
			{
				szFile = gEngfuncs.COM_ParseFile(szFile,szToken);
				m_Ropes[num_pys_rope].frictionConstant = atof(szToken);
			}
			else if ( !stricmp( szToken, "airfriction" ) )
			{
				szFile = gEngfuncs.COM_ParseFile(szFile,szToken);
				m_Ropes[num_pys_rope].airFrictionConstant = atof(szToken);
			}
			else if ( !stricmp( szToken, "sim_speed" ) )
			{
				szFile = gEngfuncs.COM_ParseFile(szFile,szToken);
				m_Ropes[num_pys_rope].sim_speed = atof(szToken);
			}
			else if ( !stricmp( szToken, "update_range" ) )
			{
				szFile = gEngfuncs.COM_ParseFile(szFile,szToken);
				m_Ropes[num_pys_rope].update_range = atof(szToken);
			}
			else if ( !stricmp( szToken, "sprite" ) )
			{
				szFile = gEngfuncs.COM_ParseFile(szFile,szToken);
				strncpy(m_Ropes[num_pys_rope].Sprite, szToken, sizeof(m_Ropes[num_pys_rope].Sprite) );
			}
			else if ( !stricmp( szToken, "can_collide" ) )
			{
				szFile = gEngfuncs.COM_ParseFile(szFile,szToken);
				if ( !stricmp( szToken, "true" ))
				{
					m_Ropes[num_pys_rope].can_collide = true;
				}
				if ( !stricmp( szToken, "false" ))
				{
					m_Ropes[num_pys_rope].can_collide = false;
				}
			}
			szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
		}
	}

//	sprintf( rope->Sprite, "sprites/%s", rope->Sprite);

	gEngfuncs.COM_FreeFile( szFile );

	Vector g = Vector(0,0,-10);

	m_Ropes[num_pys_rope].start_ent = start_source;
	m_Ropes[num_pys_rope].end_ent = end_source;

	m_Ropes[num_pys_rope].start_origin = start_source->curstate.origin;
	m_Ropes[num_pys_rope].end_origin = end_source->curstate.origin;

	vec3_t dir = m_Ropes[num_pys_rope].start_origin - m_Ropes[num_pys_rope].end_origin;
	dir = -dir;
	float Length = dir.Length();
	Length = Length/m_Ropes[num_pys_rope].num_segments-1;
	float l = Length;
	VectorNormalize(dir);

	m_Ropes[num_pys_rope].ropeSimulation = new RopeSimulation(m_Ropes[num_pys_rope].num_segments,m_Ropes[num_pys_rope].mass,g,m_Ropes[num_pys_rope].airFrictionConstant,m_Ropes[num_pys_rope].start_origin,dir,Length);

	for( int i= 0; i < m_Ropes[num_pys_rope].ropeSimulation->numOfMasses; i++)
		m_Ropes[num_pys_rope].ropeSimulation->getMass(i)->pos = m_Ropes[num_pys_rope].start_origin + dir * l*i;


	m_Ropes[num_pys_rope].ropeSimulation->getMass(0)->pos = m_Ropes[num_pys_rope].start_origin;

	m_Ropes[num_pys_rope].ropeSimulation->springs = new Spring*[m_Ropes[num_pys_rope].num_segments - 1];
													
		for(i= 0; i < m_Ropes[num_pys_rope].num_segments-1; i++)		
		{
			m_Ropes[num_pys_rope].ropeSimulation->springs[i] = new Spring(m_Ropes[num_pys_rope].ropeSimulation->masses[i], m_Ropes[num_pys_rope].ropeSimulation->masses[i + 1], 
				m_Ropes[num_pys_rope].springconst, l+m_Ropes[num_pys_rope].segment_lenght, m_Ropes[num_pys_rope].frictionConstant);
		}

	m_Ropes[num_pys_rope].red = start_source->curstate.rendercolor.r;
	m_Ropes[num_pys_rope].green = start_source->curstate.rendercolor.g;
	m_Ropes[num_pys_rope].blue = start_source->curstate.rendercolor.b;
	m_Ropes[num_pys_rope].alpha = start_source->curstate.renderamt;

	m_Ropes[num_pys_rope].red = m_Ropes[num_pys_rope].red/255;
	m_Ropes[num_pys_rope].green = m_Ropes[num_pys_rope].green/255;
	m_Ropes[num_pys_rope].blue = m_Ropes[num_pys_rope].blue/255;

	gEngfuncs.Con_Printf("Added Rope to List\n");

		m_Ropes[num_pys_rope].free = false;

	num_pys_rope++;
}


void GLRopeRender::TrashRope( pys_rope* part )
{
delete part;
}

void GLRopeRender::DrawRopes( float fltime )
{
	int VisibleRopes = 0;

	for(int i = 0; i < num_pys_rope;i++ )
	{
			VisibleRopes++;

			if(m_Ropes[i].free)
				continue;

			if(m_Ropes[i].start_ent->curstate.body)
			m_Ropes[i].start_origin = m_Ropes[i].start_ent->curstate.origin;

			if(m_Ropes[i].end_ent->curstate.body)
			m_Ropes[i].end_origin = m_Ropes[i].end_ent->curstate.origin;

			DrawRope(&m_Ropes[i],fltime);
	}
}

void GLRopeRender::StartRenderer( void )
{
	num_pys_rope = 0;

	for(int i = 0; i < MAX_ROPES; i++)
		m_Ropes[i].free = true;

	gEngfuncs.Con_Printf("Start GLRope Renderer\n");
}

extern vec3_t v_origin;

void GLRopeRender::DrawRope(pys_rope *rope,float fltime)
{
	bool update = true;

	float dt = fltime;// / 1000.0f;						
	
	float maxPossible_dt = 0.002f;				
	
  	int numOfIterations = (int)(dt / maxPossible_dt) + 1;		
	if (numOfIterations != 0)									
		dt = dt / numOfIterations;							

	for(int i = 0; i < rope->ropeSimulation->numOfMasses; i++)
	{
		rope->ropeSimulation->masses[i]->oldpos = rope->ropeSimulation->masses[i]->pos;
	}

	for (int a = 0; a < numOfIterations; ++a)				
	{
		rope->ropeSimulation->operate(dt*rope->sim_speed);
	}

	if(rope->can_collide)
	rope->ropeSimulation->test_collision();

	
	if(rope->start_ent->curstate.body)
	{
	rope->ropeSimulation->getMass(0)->pos = rope->start_origin;
	rope->ropeSimulation->getMass(0)->vel = Vector(0,0,0);
	}

	if(rope->end_ent->curstate.body)
	{
	rope->ropeSimulation->getMass(rope->ropeSimulation->numOfMasses-1)->pos = rope->end_origin;
	rope->ropeSimulation->getMass(rope->ropeSimulation->numOfMasses-1)->vel = Vector(0,0,0);
	}

	vec3_t viewAngles, v_up, v_right, v_forward,temp,Cross,backpoint;
	
	// Start Drawing The Rope.											// Set Color To Yellow
	for (a = 0; a < rope->ropeSimulation->numOfMasses - 1; ++a)
	{
		Mass* mass1 = rope->ropeSimulation->getMass(a);
		float* pos1 = mass1->pos;
	
		Mass* mass2 = rope->ropeSimulation->getMass(a + 1);
		float* pos2 = mass2->pos;

		DrawBeam(pos1,pos2,rope->scale,rope->Sprite);
	}	
}
