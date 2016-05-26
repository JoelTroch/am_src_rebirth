/*
    Copyright 2001 to 2004. The Battle Grounds Team and Contributors

    This file is part of the Battle Grounds Modification for Half-Life.

    The Battle Grounds Modification for Half-Life is free software;
    you can redistribute it and/or modify it under the terms of the
    GNU Lesser General Public License as published by the Free
    Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    The Battle Grounds Modification for Half-Life is distributed in
    the hope that it will be useful, but WITHOUT ANY WARRANTY; without
    even the implied warranty of MERCHANTABILITY or FITNESS FOR A
    PARTICULAR PURPOSE.  See the GNU Lesser General Public License
    for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with The Battle Grounds Modification for Half-Life;
    if not, write to the Free Software Foundation, Inc., 59 Temple Place,
    Suite 330, Boston, MA  02111-1307  USA

    You must obey the GNU Lesser General Public License in all respects for
    all of the code used other than code distributed with the Half-Life
    SDK developed by Valve.  If you modify this file, you may extend this
    exception to your version of the file, but you are not obligated to do so.
    If you do not wish to do so, delete this exception statement from your
    version.
*/

#ifndef FUNC_GRASS
#define FUNC_GRASS

#include "particle_defs.h"
/*
class CGrass : public CBaseEntity 
{
	char sParticleDefintionFile[MAX_PARTICLE_PATH];
	unsigned int iID;
public:
	bool bIsOn;
	void Spawn( void );
    void KeyValue( KeyValueData* pKeyValueData );
    void MakeAware( CBaseEntity* pEnt );
	void EXPORT Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void Precache (void){};
	
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
};
*/

#define SF_GRASS_ACTIVE 1

class CGrass : public CBaseEntity
{
	char sParticleDefintionFile[MAX_PARTICLE_PATH];
	unsigned int iID;
public:
	void Spawn( void );
	void KeyValue( KeyValueData* pKeyValueData );
	void Precache( void );
	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	void EXPORT ResumeThink( void );
	void EXPORT TurnOn( void );
	void EXPORT TurnOff( void );

//	void SendData( void );
    void MakeAware( CBaseEntity* pEnt );

	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	STATE GetState( void );
		
	bool bIsOn;
};

#endif
