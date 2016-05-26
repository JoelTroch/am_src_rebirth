class CPys_Rope : public CPointEntity 
{
public:

void Spawn( void );
void Precache( void );
void Activate( void );
void DesiredAction( void );
void EXPORT Think( void );

void EXPORT Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

};
