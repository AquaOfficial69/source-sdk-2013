//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements the tripmine grenade.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "beam_shared.h"
#include "shake.h"
#include "grenade_tripmine.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "explode.h"
#ifdef EZ2
#include "mapbase/matchers.h"
#endif
#ifdef EZ2
#include "hl2_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern const char* g_pModelNameLaser;

ConVar    sk_plr_dmg_tripmine		( "sk_plr_dmg_tripmine","0");
ConVar    sk_npc_dmg_tripmine		( "sk_npc_dmg_tripmine","0");
ConVar    sk_tripmine_radius		( "sk_tripmine_radius","0");

LINK_ENTITY_TO_CLASS( npc_tripmine, CTripmineGrenade );

BEGIN_DATADESC( CTripmineGrenade )

	DEFINE_FIELD( m_hOwner,		FIELD_EHANDLE ),
	DEFINE_FIELD( m_flPowerUp,	FIELD_TIME ),
	DEFINE_FIELD( m_vecDir,		FIELD_VECTOR ),
	DEFINE_FIELD( m_vecEnd,		FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_flBeamLength, FIELD_FLOAT ),
	DEFINE_FIELD( m_pBeam,		FIELD_CLASSPTR ),
	DEFINE_FIELD( m_posOwner,		FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_angleOwner,	FIELD_VECTOR ),
#ifdef MAPBASE
	DEFINE_KEYFIELD( m_flPowerUpTime, FIELD_FLOAT, "PowerUpTime" ),
	DEFINE_FIELD( m_hAttacker,	FIELD_EHANDLE ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "Activate", InputActivate ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Deactivate", InputDeactivate ),
	DEFINE_INPUTFUNC( FIELD_EHANDLE, "SetOwner", InputSetOwner ),

	// Outputs
	DEFINE_OUTPUT( m_OnExplode, "OnExplode" ),
#endif

#ifdef EZ2
	DEFINE_FIELD( m_nTripmineClass, FIELD_INTEGER ),
	DEFINE_KEYFIELD( m_nTripmineClassString, FIELD_STRING, "TripmineClass" ),
	DEFINE_KEYFIELD( m_TripmineColor, FIELD_COLOR32, "TripmineColor" ),
	DEFINE_FIELD( m_bTripped, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_hPlacer, FIELD_EHANDLE ),
#endif

	// Function Pointers
	DEFINE_THINKFUNC( WarningThink ),
	DEFINE_THINKFUNC( PowerupThink ),
	DEFINE_THINKFUNC( BeamBreakThink ),
	DEFINE_THINKFUNC( DelayDeathThink ),

END_DATADESC()

CTripmineGrenade::CTripmineGrenade()
{
	m_vecDir.Init();
	m_vecEnd.Init();
	m_posOwner.Init();
	m_angleOwner.Init();

#ifdef MAPBASE
	m_flPowerUpTime = 2.0;
#endif

#ifdef EZ2
	m_nTripmineClass = CLASS_NONE;
	m_TripmineColor.r = 255;
	m_TripmineColor.g = 55;
	m_TripmineColor.b = 52;
	m_TripmineColor.a = 64;
#endif
}

void CTripmineGrenade::Spawn( void )
{
	Precache( );
	// motor
	SetMoveType( MOVETYPE_FLY );
	SetSolid( SOLID_BBOX );
	SetModel( "models/Weapons/w_slam.mdl" );

#ifdef EZ2
	CBaseEntity * pOwner = GetOwnerEntity();
	if ( pOwner != NULL )
	{
		m_nTripmineClass = pOwner->Classify();

		m_hPlacer = pOwner;

		if (m_hPlacer->IsPlayer())
		{
			CHL2_Player *pHL2Player = static_cast<CHL2_Player*>(m_hPlacer.Get());
			if (pHL2Player)
			{
				pHL2Player->OnSetupTripmine( this );
			}
		}
	}
#endif

    IPhysicsObject *pObject = VPhysicsInitNormal( SOLID_BBOX, GetSolidFlags() | FSOLID_TRIGGER, true );
	pObject->EnableMotion( false );
	SetCollisionGroup( COLLISION_GROUP_WEAPON );

	SetCycle( 0.0f );
	m_nBody			= 3;
#ifdef MAPBASE
	if (m_flDamage == 0)
		m_flDamage		= sk_plr_dmg_tripmine.GetFloat();
	if (m_DmgRadius == 0)
		m_DmgRadius		= sk_tripmine_radius.GetFloat();
#else
	m_flDamage		= sk_plr_dmg_tripmine.GetFloat();
	m_DmgRadius		= sk_tripmine_radius.GetFloat();
#endif

	ResetSequenceInfo( );
	m_flPlaybackRate	= 0;
	
	UTIL_SetSize(this, Vector( -4, -4, -2), Vector(4, 4, 2));

#ifdef MAPBASE
	if (!HasSpawnFlags(SF_TRIPMINE_START_INACTIVE))
	{
		if (m_flPowerUpTime > 0)
		{
			m_flPowerUp = gpGlobals->curtime + m_flPowerUpTime;
	
			SetThink( &CTripmineGrenade::PowerupThink );
			SetNextThink( gpGlobals->curtime + 0.2 );
		}
		else
		{
			MakeBeam( );
			RemoveSolidFlags( FSOLID_NOT_SOLID );
			m_bIsLive			= true;

			//EmitSound( "TripmineGrenade.Activate" );
		}
	}
#else
	m_flPowerUp = gpGlobals->curtime + 2.0;
	
	SetThink( &CTripmineGrenade::PowerupThink );
	SetNextThink( gpGlobals->curtime + 0.2 );
#endif

	m_takedamage		= DAMAGE_YES;

	m_iHealth = 1;

	EmitSound( "TripmineGrenade.Place" );
	SetDamage ( 200 );

	// Tripmine sits at 90 on wall so rotate back to get m_vecDir
	QAngle angles = GetAbsAngles();
	angles.x -= 90;

	AngleVectors( angles, &m_vecDir );
	m_vecEnd = GetAbsOrigin() + m_vecDir * 2048;

	AddEffects( EF_NOSHADOW );
}


void CTripmineGrenade::Precache( void )
{
	PrecacheModel("models/Weapons/w_slam.mdl"); 

	PrecacheScriptSound( "TripmineGrenade.Place" );
	PrecacheScriptSound( "TripmineGrenade.Activate" );
}


void CTripmineGrenade::WarningThink( void  )
{
	// set to power up
	SetThink( &CTripmineGrenade::PowerupThink );
	SetNextThink( gpGlobals->curtime + 1.0f );
}


void CTripmineGrenade::PowerupThink( void  )
{
	if (gpGlobals->curtime > m_flPowerUp)
	{
		MakeBeam( );
		RemoveSolidFlags( FSOLID_NOT_SOLID );
		m_bIsLive			= true;

		// play enabled sound
		EmitSound( "TripmineGrenade.Activate" );
	}
	SetNextThink( gpGlobals->curtime + 0.1f );
}


void CTripmineGrenade::KillBeam( void )
{
	if ( m_pBeam )
	{
		UTIL_Remove( m_pBeam );
		m_pBeam = NULL;
	}
}

#ifdef EZ2
bool CTripmineGrenade::KeyValue( const char * szKeyName, const char * szValue )
{
	if ( FStrEq( szKeyName, "TripmineClass" ) )
	{
		for (int i = CLASS_NONE; i < NUM_AI_CLASSES; i++)
		{
			if ( FStrEq( szValue, g_pGameRules->AIClassText( i ) ) )
			{
				m_nTripmineClass = (Class_T)i;
				break;
			}
		}
	}

	return BaseClass::KeyValue( szKeyName, szValue );
}
#endif

void CTripmineGrenade::MakeBeam( void )
{
	trace_t tr;

	UTIL_TraceLine( GetAbsOrigin(), m_vecEnd, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

	m_flBeamLength = tr.fraction;



	// If I hit a living thing, send the beam through me so it turns on briefly
	// and then blows the living thing up
	CBaseEntity *pEntity = tr.m_pEnt;
	CBaseCombatCharacter *pBCC  = ToBaseCombatCharacter( pEntity );

	// Draw length is not the beam length if entity is in the way
	float drawLength = tr.fraction;
	if (pBCC)
	{
		SetOwnerEntity( pBCC );
		UTIL_TraceLine( GetAbsOrigin(), m_vecEnd, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
		m_flBeamLength = tr.fraction;
		SetOwnerEntity( NULL );
		
	}

	// set to follow laser spot
	SetThink( &CTripmineGrenade::BeamBreakThink );

	// Delay first think slightly so beam has time
	// to appear if person right in front of it
	SetNextThink( gpGlobals->curtime + 1.0f );

#ifndef EZ
	Vector vecTmpEnd = GetLocalOrigin() + m_vecDir * 2048 * drawLength;
#else
	Vector vecTmpEnd = GetAbsOrigin() + m_vecDir * 2048 * drawLength;
#endif

	m_pBeam = CBeam::BeamCreate( g_pModelNameLaser, 0.35 );
	m_pBeam->PointEntInit( vecTmpEnd, this );

#ifdef EZ2
	m_pBeam->SetColor( m_TripmineColor.r, m_TripmineColor.g, m_TripmineColor.b );
	m_pBeam->SetScrollRate( 25.6 );
	m_pBeam->SetBrightness( m_TripmineColor.a );
#else
	m_pBeam->SetColor( 255, 55, 52 );
	m_pBeam->SetScrollRate( 25.6 );
	m_pBeam->SetBrightness( 64 );
#endif
	
	int beamAttach = LookupAttachment("beam_attach");
	m_pBeam->SetEndAttachment( beamAttach );
}


void CTripmineGrenade::BeamBreakThink( void  )
{
	// See if I can go solid yet (has dropper moved out of way?)
	if (IsSolidFlagSet( FSOLID_NOT_SOLID ))
	{
		trace_t tr;
		Vector	vUpBit = GetAbsOrigin();
		vUpBit.z += 5.0;

		UTIL_TraceEntity( this, GetAbsOrigin(), vUpBit, MASK_SHOT, &tr );
		if ( !tr.startsolid && (tr.fraction == 1.0) )
		{
			RemoveSolidFlags( FSOLID_NOT_SOLID );
		}
	}

	trace_t tr;

	// NOT MASK_SHOT because we want only simple hit boxes
	UTIL_TraceLine( GetAbsOrigin(), m_vecEnd, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

	// ALERT( at_console, "%f : %f\n", tr.flFraction, m_flBeamLength );

	// respawn detect. 
	if ( !m_pBeam )
	{
		MakeBeam( );
		if ( tr.m_pEnt )
			m_hOwner = tr.m_pEnt;	// reset owner too
	}


	CBaseEntity *pEntity = tr.m_pEnt;
	CBaseCombatCharacter *pBCC  = ToBaseCombatCharacter( pEntity );

#ifdef EZ2
	// Tripmines do not detonate when tripped by entities that treat their class as friendly or by the owner
	if ( ( pBCC && pBCC->GetDefaultRelationshipDisposition( m_nTripmineClass ) == D_LI ) || ( GetOwnerEntity() && GetOwnerEntity() == pBCC ) )
	{
		SetNextThink( gpGlobals->curtime + 0.05f );
		return;
	}
#endif

	if (pBCC || fabs( m_flBeamLength - tr.fraction ) > 0.001)
	{
		m_iHealth = 0;
#ifdef EZ2
		m_bTripped = true;
#endif
#ifdef MAPBASE
		Event_Killed( CTakeDamageInfo( (CBaseEntity*)m_hOwner, pEntity, 100, GIB_NORMAL ) );
#else
		Event_Killed( CTakeDamageInfo( (CBaseEntity*)m_hOwner, this, 100, GIB_NORMAL ) );
#endif
		return;
	}

	SetNextThink( gpGlobals->curtime + 0.05f );
}

#if 0 // FIXME: OnTakeDamage_Alive() is no longer called now that base grenade derives from CBaseAnimating
int CTripmineGrenade::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	if (gpGlobals->curtime < m_flPowerUp && info.GetDamage() < m_iHealth)
	{
		// disable
		// Create( "weapon_tripmine", GetLocalOrigin() + m_vecDir * 24, GetAngles() );
		SetThink( &CTripmineGrenade::SUB_Remove );
		SetNextThink( gpGlobals->curtime + 0.1f );
		KillBeam();
		return FALSE;
	}
	return BaseClass::OnTakeDamage_Alive( info );
}
#endif

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CTripmineGrenade::Event_Killed( const CTakeDamageInfo &info )
{
	m_takedamage		= DAMAGE_NO;

#ifdef MAPBASE
	m_hAttacker = info.GetAttacker();
#endif

	SetThink( &CTripmineGrenade::DelayDeathThink );
	SetNextThink( gpGlobals->curtime + 0.25 );

	EmitSound( "TripmineGrenade.StopSound" );
}


void CTripmineGrenade::DelayDeathThink( void )
{
	KillBeam();
	trace_t tr;
	UTIL_TraceLine ( GetAbsOrigin() + m_vecDir * 8, GetAbsOrigin() - m_vecDir * 64,  MASK_SOLID, this, COLLISION_GROUP_NONE, & tr);
	UTIL_ScreenShake( GetAbsOrigin(), 25.0, 150.0, 1.0, 750, SHAKE_START );

	ExplosionCreate( GetAbsOrigin() + m_vecDir * 8, GetAbsAngles(), m_hOwner, GetDamage(), 200, 
		SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NOSMOKE, 0.0f, this);

#ifdef MAPBASE
	m_OnExplode.FireOutput(m_hAttacker.Get(), this);
#endif

#ifdef EZ2
	if (m_hPlacer && m_hPlacer->IsPlayer())
	{
		CHL2_Player *pHL2Player = static_cast<CHL2_Player*>(m_hPlacer.Get());
		if (pHL2Player)
		{
			if (!m_bTripped)
			{
				// UNDONE: Do this in case the attacker might've been from the player
				//if (m_hAttacker && m_hAttacker->GetOwnerEntity())
				//	m_hAttacker = m_hAttacker->GetOwnerEntity();

				pHL2Player->OnTripmineExploded( this, m_hAttacker );
			}
			else
			{
				pHL2Player->OnTripmineExploded( this, GetOwnerEntity() );
			}
		}
	}
#endif

	UTIL_Remove( this );
}

#ifdef MAPBASE
//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CTripmineGrenade::InputActivate( inputdata_t &inputdata )
{
	if (m_flPowerUpTime > 0)
	{
		m_flPowerUp = gpGlobals->curtime + m_flPowerUpTime;
	
		SetThink( &CTripmineGrenade::PowerupThink );
		SetNextThink( gpGlobals->curtime + 0.2 );
	}
	else
	{
		MakeBeam( );
		RemoveSolidFlags( FSOLID_NOT_SOLID );
		m_bIsLive			= true;

		//EmitSound( "TripmineGrenade.Activate" );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CTripmineGrenade::InputDeactivate( inputdata_t &inputdata )
{
	KillBeam( );
	//AddSolidFlags( FSOLID_NOT_SOLID );
	m_bIsLive = false;
}
#endif

