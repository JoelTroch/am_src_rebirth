/****************

  There is my new entity func_phy". 

  The original code is made by Max Fooz. It's the same code but it uses the func_pushable
  entity for that. The problem is if the pushable doesn't have any 'origin' brush, you will
  see a lot of weird things. 
  The code need a entity whit a origin, and you need to edit the map for that... so, why
  can't we do a new entity? There is... func_phy!

  I expand the code and make this: 
  
	-New 5 materials
	-New .fgd entry to specific the material of the entity.
	-Specifics sounds that depends of the material.
	-Sparks, and simple effects when you hit the entity, and depends of the material too.
	-The materials has your own weight. A plaster cube is lighter than a metal barrel.

  UNDONE:
	-Fix the bug whit the angles
	-Do a damage to the player, when he collide whit the entity. For example: If a rock fall 
	in you head, you will smashed!
	-Still expanding the code. I don't kwon... do a new things...

																						SysOp:
																						-----
****************/

class CPhy : public CBreakable
{
public:
	void	Spawn ( void );
	void	Precache( void );
	void	EXPORT PushableThink( void );
	void	Touch ( CBaseEntity *pOther );
	void	Move( CBaseEntity *pMover, int push );
	void	KeyValue( KeyValueData *pkvd );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	EXPORT StopSound( void );

	virtual int	ObjectCaps( void ) { return (CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_CONTINUOUS_USE; }
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	void TestBeam( Vector org, Vector end, Vector color, int life, int width );

	inline float MaxSpeed( void ) { return m_maxSpeed; }
	
	// breakables use an overridden takedamage
	virtual int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	
	static	TYPEDESCRIPTION m_SaveData[];

	int		m_lastSound;	// no need to save/restore, just keeps the same sound from playing twice in a row
	float	m_maxSpeed;
	float	m_soundTime;
};

TYPEDESCRIPTION	CPhy::m_SaveData[] = 
{
	DEFINE_FIELD( CPhy, m_maxSpeed, FIELD_FLOAT ),
	DEFINE_FIELD( CPhy, m_soundTime, FIELD_TIME ),
};

IMPLEMENT_SAVERESTORE( CPhy, CBreakable );

LINK_ENTITY_TO_CLASS( func_phy, CPhy );

void CPhy :: Spawn( void )
{
	Vector vecMins = pev->mins;
	Vector vecMaxs = pev->maxs;

	CBreakable::Spawn();

	pev->movetype	= MOVETYPE_BOUNCE;//MOVETYPE_PUSHSTEP;
	pev->solid		= SOLID_BBOX;
	SET_MODEL( ENT(pev), STRING(pev->model) );

	if ( pev->friction > 399 )
		pev->friction = 399;

	m_maxSpeed = 400 - pev->friction;
	SetBits( pev->flags, FL_FLOAT );

	pev->friction = 0.5;
	pev->gravity = 1;
	
	pev->origin.z += 1;	// Pick up off of the floor

	UTIL_SetOrigin( pev, pev->origin );

	SetThink(&CPhy:: PushableThink );
	pev->nextthink = 0.1;

	// Multiply by area of the box's cross-section (assume 1000 units^3 standard volume)
	pev->skin = ( pev->skin * (pev->maxs.x - pev->mins.x) * (pev->maxs.y - pev->mins.y) ) * 0.0005;
	m_soundTime = 0;
}


void CPhy :: Precache( void )
{
//UNDONE: do a specific precaches, that depends of the material. Don't precache aaaalll the sounds...

//PRECACHE STUFF
//0 - no sound
//1 - metal
//2 - plaster
//3 - glass
//4 - concrete
//5 - wood

		//wood
			PRECACHE_SOUND("fisica/madera/move1.wav");
			PRECACHE_SOUND("fisica/madera/move2.wav");
			PRECACHE_SOUND("fisica/madera/impact1.wav");
			PRECACHE_SOUND("fisica/madera/impact2.wav");

		//concrete sounds
		PRECACHE_SOUND("fisica/concreto/move1.wav");
		PRECACHE_SOUND("fisica/concreto/move2.wav");
		PRECACHE_SOUND("debris/concrete1.wav");
		PRECACHE_SOUND("debris/concrete3.wav");

		//metal
		PRECACHE_SOUND("fisica/metal/impact1.wav");
		PRECACHE_SOUND("fisica/metal/impact2.wav");
			PRECACHE_SOUND("fisica/metal/move1.wav");
			PRECACHE_SOUND("fisica/metal/move2.wav");
		PRECACHE_SOUND("fisica/metal/b_impact1.wav");
		PRECACHE_SOUND("fisica/metal/b_impact2.wav");
		PRECACHE_SOUND("fisica/metal/b_impact3.wav");

		//plastic sounds
		PRECACHE_SOUND("fisica/plastico/impact1.wav");
		PRECACHE_SOUND("fisica/plastico/impact2.wav");
		PRECACHE_SOUND("fisica/plastico/impact3.wav");
			PRECACHE_SOUND("fisica/plastico/move1.wav");
			PRECACHE_SOUND("fisica/plastico/move2.wav");

		CBreakable::Precache( );
}


void CPhy :: KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq(pkvd->szKeyName, "size") )
	{
		int bbox = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;

		switch( bbox )
		{
		case 0:	// Point
			UTIL_SetSize(pev, Vector(-8, -8, -8), Vector(8, 8, 8));
			break;

		case 2: // Big Hull!?!?	!!!BUGBUG Figure out what this hull really is
			UTIL_SetSize(pev, VEC_DUCK_HULL_MIN*2, VEC_DUCK_HULL_MAX*2);
			break;

		case 3: // Player duck
			UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
			break;

		default:
		case 1: // Player
			UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);
			break;
		}

	}
	else if ( FStrEq(pkvd->szKeyName, "buoyancy") )
	{
		pev->skin = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBreakable::KeyValue( pkvd );
}


// Pull the func_pushable
void CPhy :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !pActivator || !pActivator->IsPlayer() )
	{
			this->CBreakable::Use( pActivator, pCaller, useType, value );
		return;
	}

	if ( pActivator->pev->velocity != g_vecZero )
		Move( pActivator, 0 );
}

/*
===================
AngleBetweenVectors

===================
*/

float AngleBetweenVectors( const vec3_t v1, const vec3_t v2 )
{
	float angle;
	float l1 = v1.Length();
	float l2 = v2.Length();

	if ( !l1 || !l2 )
		return 0.0f;

	angle = acos( DotProduct( v1, v2 ) / (l1*l2) );
	angle = ( angle  * 180.0f ) / M_PI;

	return angle;
}

void NormalizeAngles( float *angles )
{
	int i;
	// Normalize angles
	for ( i = 0; i < 3; i++ )
	{
		if ( angles[i] > 180.0 )
		{
			angles[i] -= 360.0;
		}
		else if ( angles[i] < -180.0 )
		{
			angles[i] += 360.0;
		}
	}
}

void CPhy :: PushableThink( void )
{
//	ALERT( at_debug, "Think !\n" );
//	pev->velocity = pev->velocity * 0.8;
//	pev->avelocity = pev->avelocity * 0.8;
//	ALERT( at_debug, "Angles yaw: %3.2f\n", pev->angles.y );

/*	if( pev->angles.x >= 360 )
		pev->angles.x -= 360;
	if( pev->angles.x >= -360 )
		pev->angles.x += 360;*/

	if( pev->flags & FL_ONGROUND )
	{
		
		//pev->avelocity = pev->avelocity * 0.5;

	}

//	SetNextThink( 0.1 );
	pev->nextthink = 0.1;
}

void CPhy :: Touch( CBaseEntity *pOther )
{
	Vector savedangles = Vector( 0, 0, 0 );
	int negate = 0;
	TraceResult tr;
	// look down directly to know the surface we're lying.
	UTIL_TraceLine( pev->origin, pev->origin - Vector(0,0,64), ignore_monsters, edict(), &tr );

	//ALERT( at_debug, "Velocity Touch : %3.2f\n", pev->velocity.x );
	pev->velocity = pev->velocity * 0.8;
//	pev->avelocity = pev->avelocity * 0.5;

	if( !(pev->flags & FL_ONGROUND) )
	{
		//pev->avelocity.x = RANDOM_FLOAT( -400, 400 );//fooz
		if (pev->frags == 0)
		{
			//no sound
		}
/*****
Sys:
This code will do a impact sounds when the entity colides whit another entity or worldspawn, or it's damaged.
*****/
		if (pev->frags == 1) //metal
		{
			if( RANDOM_LONG( 0, 1 ))
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "fisica/metal/impact1.wav", 1, ATTN_NORM, 0, PITCH_NORM);
			else
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "fisica/metal/impact2.wav", 1, ATTN_NORM, 0, PITCH_NORM);
		}

		if (pev->frags == 2) //plaster
		{			
			switch (RANDOM_LONG(0,2)) 
			{
				case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "fisica/plastico/impact1.wav", 1, ATTN_NORM, 0, PITCH_NORM); break;
				case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "fisica/plastico/impact2.wav", 1, ATTN_NORM, 0, PITCH_NORM); break;
				case 2:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "fisica/plastico/impact3.wav", 1, ATTN_NORM, 0, PITCH_NORM); break;
			}			
		}
		
		if (pev->frags == 3) //glass
		{
			if( RANDOM_LONG( 0, 1 ))
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/glass3.wav", 1, ATTN_NORM, 0, PITCH_NORM);
			else
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/glass1.wav", 1, ATTN_NORM, 0, PITCH_NORM);
		}

		if (pev->frags == 4) //concrete
		{
			if( RANDOM_LONG( 0, 1 ))
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/concrete1.wav", 1, ATTN_NORM, 0, PITCH_NORM);
			else
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/concrete3.wav", 1, ATTN_NORM, 0, PITCH_NORM);
		}

		if (pev->frags == 5) //wood
		{
			switch (RANDOM_LONG(0,1)) 
			{
				case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "fisica/madera/impact1.wav", 1, ATTN_NORM, 0, PITCH_NORM); break;
				case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "fisica/madera/impact2.wav", 1, ATTN_NORM, 0, PITCH_NORM); break;
			}
		}
	}
	else
/******
Sys:

This code is nice, but has a bug: 
The angles of the entity it's normalized, but if you touch or move the entity, and if it have another initial
angle, the entity will move too quickly in the reverse angle.

Look that:
1- We have a stand barrel.

	------
	||||||
	|    |
	|    |
	|    |
	------

2- and you hit the barrel and the barrel fall of and it's lie on the ground

  -----------
  |	 	   ||
  |		   ||
  -----------

3- Well, do a simple touch to the barrel...

  -----------
  || 	    |
  ||		|
  -----------

If you have a barrel, don't make any diference, but if you have a more complex object (a monitor i suposed), 
you will see the bug.

It's too hard to explain, you need see it...
******/
	{	
		for( int i = 0; i<3; i++ )
		{
		if( pev->angles.x < 0 )
			negate = 1;

			if( fabs(pev->angles.x) < 45 )
				savedangles.x = 0;
			else if( fabs(pev->angles.x) >= 45 && fabs(pev->angles.x) <= 135 )
				savedangles.x = 90;
			else if( fabs(pev->angles.x) > 135 && fabs(pev->angles.x) <= 180 )
				savedangles.x = 180;
		}

		#ifndef M_PI
		#define M_PI 3.14159265358979
		#endif
		#define ang2rad (2 * M_PI / 360)

          if ( tr.flFraction < 1.0 )
          {
			Vector forward, right, angdir, angdiry;
			Vector Angles = pev->angles;
			//Fooz
			NormalizeAngles( Angles );
		
			UTIL_MakeVectorsPrivate( Angles, forward, right, NULL );
			angdir = forward;
			Vector left = -right;
			angdiry = left;

			pev->angles.x = -UTIL_VecToAngles( angdir - DotProduct(angdir, tr.vecPlaneNormal) * tr.vecPlaneNormal).x;
			pev->angles.y = UTIL_VecToAngles( angdir - DotProduct(angdir, tr.vecPlaneNormal) * tr.vecPlaneNormal).y;
		
			pev->angles.z = UTIL_VecToAngles( angdiry - DotProduct(angdiry, tr.vecPlaneNormal) * tr.vecPlaneNormal).x;
			
//			pev->angles.z = UTIL_VecToAngles( angdiry - DotProduct(angdiry, tr.vecPlaneNormal) * right ).x;
          }

		#undef ang2rad
		  
		  if( negate )
			pev->angles.x -= savedangles.x;
		  else
			pev->angles.x += savedangles.x;
	}


	if ( FClassnameIs( pOther->pev, "worldspawn" ) )
		return;

	Move( pOther, 1 );
}


void CPhy :: Move( CBaseEntity *pOther, int push )
{
	entvars_t*	pevToucher = pOther->pev;
	int playerTouch = 0;

	// Is entity standing on this pushable ?
	if ( FBitSet(pevToucher->flags,FL_ONGROUND) && pevToucher->groundentity && VARS(pevToucher->groundentity) == pev )
	{
		// Only push if floating
		if ( pev->waterlevel > 0 && pev->watertype > CONTENT_FLYFIELD)
			pev->velocity.z += pevToucher->velocity.z * 0.1;

		return;
	}

	if ( pOther->IsPlayer() )
	{
		if ( push && !(pevToucher->button & (IN_FORWARD|IN_USE)) )	// Don't push unless the player is pushing forward and NOT use (pull)
			return;
		playerTouch = 1;
	}

	float factor;

	if ( playerTouch )
	{
		if ( !(pevToucher->flags & FL_ONGROUND) )	// Don't push away from jumping/falling players unless in water
		{//sys
			if ( pev->waterlevel < 1 || pev->watertype <= CONTENT_FLYFIELD)
		//	{
		//		pOther->TakeDamage(pev, pev, 30, DMG_PARALYZE | DMG_NEVERGIB);//daño?
				return;
		//	}
			else 
		//	{
				factor = 0.1;
		//	}
		}
		else
			factor = 1;
	}
	else 
		factor = 0.25;

	if (!push)
		factor = factor*0.5;

	pev->velocity.x += pevToucher->velocity.x * factor;
	pev->velocity.y += pevToucher->velocity.y * factor;

	float length = sqrt( pev->velocity.x * pev->velocity.x + pev->velocity.y * pev->velocity.y );
	if ( push && (length > MaxSpeed()) )
	{
		pev->velocity.x = (pev->velocity.x * MaxSpeed() / length );
		pev->velocity.y = (pev->velocity.y * MaxSpeed() / length );
	}
	if ( playerTouch )
	{
		pevToucher->velocity.x = pev->velocity.x;
		pevToucher->velocity.y = pev->velocity.y;
		if ( (gpGlobals->time - m_soundTime) > 0.7 )
		{
			m_soundTime = gpGlobals->time;
			if ( length > 0 && FBitSet(pev->flags,FL_ONGROUND) )
			{
/******			
Sys:
A big hack... 
When you move the entity, it will make a sound. 

There are 6 materials type:	 
	-No defined (no material) The entity doesn't make any sound.
	-Metal (metal rough sound)
	-Plaster (a plastic tube sound)
	-Glass (UNDONE: glass move sound??)
	-Cement (concrete sound)
	-Wood (wood sound)
*******/
				if (pev->frags == 0)
				{
					//no sound
				}

				if (pev->frags == 1) //metal
				{
					if (RANDOM_LONG(0,1))
						EMIT_SOUND(ENT(pev), CHAN_WEAPON, "fisica/metal/move1.wav", 0.5, ATTN_NORM);
					else
						EMIT_SOUND(ENT(pev), CHAN_WEAPON, "fisica/metal/move2.wav", 0.5, ATTN_NORM);
					
				}

				if (pev->frags == 2) //plaster
				{
					if (RANDOM_LONG(0,1))
						EMIT_SOUND(ENT(pev), CHAN_WEAPON, "fisica/plastico/move1.wav", 0.5, ATTN_NORM);
					else
						EMIT_SOUND(ENT(pev), CHAN_WEAPON, "fisica/plastico/move2.wav", 0.5, ATTN_NORM);
				}
				
				if (pev->frags == 3) { }//glass

				if (pev->frags == 4) //cemento
				{
					if (RANDOM_LONG(0,1))
						EMIT_SOUND(ENT(pev), CHAN_WEAPON, "fisica/concreto/move1.wav", 0.5, ATTN_NORM);
					else
						EMIT_SOUND(ENT(pev), CHAN_WEAPON, "fisica/concreto/move2.wav", 0.5, ATTN_NORM);
				}

				if (pev->frags == 5) //wood
				{
					if (RANDOM_LONG(0,1))
						EMIT_SOUND(ENT(pev), CHAN_WEAPON, "fisica/madera/move1.wav", 0.5, ATTN_NORM);
					else
						EMIT_SOUND(ENT(pev), CHAN_WEAPON, "fisica/madera/move2.wav", 0.5, ATTN_NORM);
				}
		//		SetThink( StopSound );
		//		pev->nextthink = 0.1;
			}
		}
	}
}

#if 0
void CPhy::StopSound( void )
{
	Vector dist = pev->oldorigin - pev->origin;
	if ( dist.Length() <= 0 )
		STOP_SOUND( ENT(pev), CHAN_WEAPON, m_soundNames[m_lastSound] );
}
#endif
//A Fooz's Testbeam func expanded by me. Why? I needed a realistic sparks. Anyways, don't like me the lasers.
void CPhy :: TestBeam( Vector org, Vector end, Vector color, int life, int width )
{
		extern short g_sModelIndexLaser;
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_BEAMPOINTS );
			WRITE_COORD( org.x );
			WRITE_COORD( org.y );
			WRITE_COORD( org.z );

			WRITE_COORD( end.x );
			WRITE_COORD( end.y );
			WRITE_COORD( end.z );
			WRITE_SHORT( g_sModelIndexLaser );
			WRITE_BYTE( 0 ); // framerate
			WRITE_BYTE( 0 ); // framerate

			WRITE_BYTE( life ); // life
			WRITE_BYTE( width );  // width

			WRITE_BYTE( 0 );   // noise
			WRITE_BYTE( color.x );   // r, g, b
			WRITE_BYTE( color.y );   // r, g, b
			WRITE_BYTE( color.z );   // r, g, b
			WRITE_BYTE( 160 );	// brightness
			WRITE_BYTE( 0 );		// speed
		MESSAGE_END();
}

int CPhy :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	Vector			vecDir, r, anorm, rforward, rup, rright;
//	float a;
	float force = flDamage * 10;
	TraceResult trace = UTIL_GetGlobalTrace( );
	//UTIL_MakeVectors( pev->angles );

	// grab the vector of the incoming attack. ( pretend that the inflictor is a little lower than it really is, so the body will tend to fly upward a bit).
	vecDir = r = Vector( 0, 0, 0 );
	if (!FNullEnt( pevInflictor ))
	{			
		if ( FClassnameIs( pevInflictor, "projectile" ) )
			pevInflictor = VARS( pevInflictor->owner );

		CBaseEntity *pInflictor = CBaseEntity :: Instance( pevInflictor );
		if (pInflictor)
		{	
			vecDir = g_vecAttackDir = ( trace.vecEndPos - pInflictor->Center() ).Normalize();
			r = ( trace.vecEndPos - Center() ).Normalize();
		}
	}

	anorm = UTIL_VecToAngles( r );
	anorm.x = -anorm.x;
	UTIL_MakeVectorsPrivate( anorm, rforward, rright, rup );

/*****
Sys: 
Let's do a material sound when you hit the entity and big sparks when you hit the material 'metal'
This only work whit certain damage types, like a crowbar-knife hit, o a bullet impact.
******/

	if (pev->frags == 1) //metal
	{
		//if ( bitsDamageType & (DMG_BULLET | DMG_CLUB) )
		if ( ( bitsDamageType & DMG_BULLET)|| ( bitsDamageType & DMG_CLUB) )
		{
			UTIL_Ricochet( trace.vecEndPos, 0.5 );
			
			switch (RANDOM_LONG(0,2)) 
			{
				case 0:	EMIT_SOUND(ENT(pev), CHAN_BODY, "fisica/metal/b_impact1.wav", 0.9, ATTN_NORM); break;
				case 1:	EMIT_SOUND(ENT(pev), CHAN_BODY, "fisica/metal/b_impact2.wav", 0.9, ATTN_NORM); break;
				case 2:	EMIT_SOUND(ENT(pev), CHAN_BODY, "fisica/metal/b_impact3.wav", 0.9, ATTN_NORM); break;			
			}

													//int color, int count, int speed, int velocityRange//PARAMs

		//		StreakSplash( trace.vecEndPos, trace.vecPlaneNormal, 6, 20, 50, 400 );//REFERENCE
			if (RANDOM_LONG( 0, 99 ) < 40)
			UTIL_WhiteSparks( trace.vecEndPos, trace.vecPlaneNormal, 0, 5, 500, 500 );//chispas

			UTIL_WhiteSparks( trace.vecEndPos, trace.vecPlaneNormal, 9, 5, 5, 100 );//puntos
			UTIL_WhiteSparks( trace.vecEndPos, trace.vecPlaneNormal, 0, 5, 500, 20 );//chispas

		}
	}
	
	if (pev->frags == 3) //glass
		UTIL_Ricochet( trace.vecEndPos, 0.5 );

	if (pev->frags == 4) //cement
		UTIL_Sparks( trace.vecEndPos );
	
	if (pev->frags == 5) //wood
	{
		switch (RANDOM_LONG(0,2)) 
		{
			case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "debris/wood1.wav", 0.9, ATTN_NORM); break;
			case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "debris/wood2.wav", 0.9, ATTN_NORM); break;
			case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "debris/wood3.wav", 0.9, ATTN_NORM); break;			
		}
	}

//	if ( bitsDamageType & DMG_BULLET )
/*******
Sys: What the ....

If the material is "plaster" let's do more movement.
*******/
	if (pev->frags == 2) //plastico
	{
		force *= 0.8;
		pev->avelocity.z = cos( AngleBetweenVectors( vecDir, rup ) ) * force * 1;//fooz	
	}
	else// Isn't plaster
	{
		if ( ( bitsDamageType & DMG_BULLET)|| ( bitsDamageType & DMG_CLUB) )
			force *= 0.5;
	}

	pev->flags &= ~FL_ONGROUND;
	pev->origin.z += 1;

	pev->avelocity.x = cos( AngleBetweenVectors( vecDir, rup ) ) * 100;
	pev->avelocity.y = cos( AngleBetweenVectors( vecDir, -rright ) ) * 200;

//	pev->avelocity.z = cos( AngleBetweenVectors( vecDir, rup ) ) * force * 2;//fooz	
//	pev->avelocity.z = sin( AngleBetweenVectors( vecDir, rup ) ) * force * 2;//fooz

	ALERT( at_console, "X : %3.1f %3.1f° Y: %3.1f %3.1f°\n", pev->avelocity.x, AngleBetweenVectors( vecDir, rup ), pev->avelocity.y, AngleBetweenVectors( vecDir, -rright ) );

	pev->velocity = pev->velocity /*+ gpGlobals->v_up * force * RANDOM_FLOAT( 0, 0.5 )*/ + vecDir * force * RANDOM_FLOAT( 0.5, 1.0 );

//	pev->velocity = pev->velocity + gpGlobals->v_up * force * RANDOM_FLOAT( 0, 0.5 ) + vecDir * force * RANDOM_FLOAT( 0.5, 1.0 );

	//if ( pev->spawnflags & SF_PUSH_BREAKABLE )
	//	return CBreakable::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );

	return 1;
}