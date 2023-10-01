//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ai_default.h"
#include "ai_task.h"
#include "ai_schedule.h"
#include "ai_node.h"
#include "ai_hull.h"
#include "ai_hint.h"
#include "ai_squad.h"
#include "ai_senses.h"
#include "ai_navigator.h"
#include "ai_motor.h"
#include "ai_behavior.h"
#include "ai_baseactor.h"
#include "ai_behavior_lead.h"
#include "ai_behavior_follow.h"
#include "ai_behavior_standoff.h"
#include "ai_behavior_assault.h"
#include "npc_playercompanion.h"
#include "soundent.h"
#include "game.h"
#include "npcevent.h"
#include "activitylist.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "sceneentity.h"
#include "ai_behavior_functank.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define RED_MODEL "models/bloody_cop.mdl"

ConVar	sk_red_health( "sk_red_health","0");

// Convars from npc_citizen17 but ported to Red
ConVar  sk_red_ar2_proficiency("sk_red_ar2_proficiency", "2"); // Added by 1upD. Skill rating 0 - 4 of how accurate the AR2 should be
ConVar  sk_red_default_proficiency("sk_red_default_proficiency", "1"); // Added by 1upD. Skill rating 0 - 4 of how accurate all weapons but the AR2 should be

//=========================================================
// Red activities
//=========================================================

class CNPC_Red : public CNPC_PlayerCompanion
{
public:
	DECLARE_CLASS( CNPC_Red, CNPC_PlayerCompanion );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache()
	{
#ifndef MAPBASE // This is now done in CNPC_PlayerCompanion::Precache()
		// Prevents a warning
		SelectModel( );
#endif
		BaseClass::Precache();

		PrecacheScriptSound( "NPC_Metropolice.FootstepLeft" );
		PrecacheScriptSound( "NPC_Metropolice.FootstepRight" );
		PrecacheScriptSound( "red.die" );

		// Commenting these out redause idrk whether they're necessary for Red
		//PrecacheInstancedScene( "scenes/Expressions/BarneyIdle.vcd" );
		//PrecacheInstancedScene( "scenes/Expressions/BarneyAlert.vcd" );
		//PrecacheInstancedScene( "scenes/Expressions/BarneyCombat.vcd" );
	}

	void	Spawn( void );
	void	SelectModel();
	Class_T Classify( void );
	void	Weapon_Equip( CBaseCombatWeapon *pWeapon );

	bool CreateBehaviors( void );

	void HandleAnimEvent( animevent_t *pEvent );

	float GetHitgroupDamageMultiplier(int iHitGroup, const CTakeDamageInfo& info);

	bool ShouldLookForBetterWeapon() { return false; }

	void OnChangeRunningBehavior( CAI_BehaviorBase *pOldBehavior,  CAI_BehaviorBase *pNewBehavior );

	void DeathSound( const CTakeDamageInfo &info );
	void GatherConditions();
	void UseFunc( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	bool ShouldRegenerateHealth(void);

#ifdef MAPBASE
	// Use Red's default subtitle color (215,255,255)
	bool	GetGameTextSpeechParams( hudtextparms_t &params ) { params.r1 = 215; params.g1 = 255; params.b1 = 255; return BaseClass::GetGameTextSpeechParams( params ); }
#endif

	CAI_FuncTankBehavior		m_FuncTankBehavior;
	COutputEvent				m_OnPlayerUse;

	DEFINE_CUSTOM_AI;
};


LINK_ENTITY_TO_CLASS( npc_red, CNPC_Red );

//---------------------------------------------------------
// 
//---------------------------------------------------------
IMPLEMENT_SERVERCLASS_ST(CNPC_Red, DT_NPC_Red)
END_SEND_TABLE()


//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CNPC_Red )
//						m_FuncTankBehavior
	DEFINE_OUTPUT( m_OnPlayerUse, "OnPlayerUse" ),
	DEFINE_USEFUNC( UseFunc ),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Red::SelectModel()
{
#ifdef MAPBASE
	if (GetModelName() == NULL_STRING)
#endif
	SetModelName( AllocPooledString( RED_MODEL ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Red::Spawn( void )
{
	Precache();

#ifdef MAPBASE
	m_iHealth = sk_red_health.GetInt();
#else
	m_iHealth = 80;
#endif

	// Red technically doesn't have a face so idk if we'll have use for this
	//m_iszIdleExpression = MAKE_STRING("scenes/Expressions/RedIdle.vcd");
	//m_iszAlertExpression = MAKE_STRING("scenes/Expressions/RedAlert.vcd");
	//m_iszCombatExpression = MAKE_STRING("scenes/Expressions/RedCombat.vcd");

	BaseClass::Spawn();

	AddEFlags( EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION );

	NPCInit();

	SetUse( &CNPC_Red::UseFunc );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_Red::Classify( void )
{
	return	CLASS_METROPOLICE;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Red::Weapon_Equip( CBaseCombatWeapon *pWeapon )
{
	BaseClass::Weapon_Equip( pWeapon );

	if( hl2_episodic.GetBool() && FClassnameIs( pWeapon, "weapon_ar2" ) )
	{
		// Allow Red to defend himself at point-blank range in c17_05.
		// Inherited from Barney
		pWeapon->m_fMinRange1 = 0.0f;
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_Red::HandleAnimEvent( animevent_t *pEvent )
{
	switch( pEvent->event )
	{
	case NPC_EVENT_LEFTFOOT:
		{
			EmitSound( "NPC_Metropolice.FootstepLeft", pEvent->eventtime );
		}
		break;
	case NPC_EVENT_RIGHTFOOT:
		{
			EmitSound( "NPC_Metropolice.FootstepRight", pEvent->eventtime );
		}
		break;

	default:
		BaseClass::HandleAnimEvent( pEvent );
		break;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float CNPC_Red::GetHitgroupDamageMultiplier(int iHitGroup, const CTakeDamageInfo& info)
{
	return BaseClass::GetHitgroupDamageMultiplier(iHitGroup, info);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void CNPC_Red::DeathSound( const CTakeDamageInfo &info )
{
	// Sentences don't play on dead NPCs
	SentenceStop();

	EmitSound( "red.die" );

}

bool CNPC_Red::CreateBehaviors( void )
{
	BaseClass::CreateBehaviors();
	AddBehavior( &m_FuncTankBehavior );

	return true;
}

void CNPC_Red::OnChangeRunningBehavior( CAI_BehaviorBase *pOldBehavior,  CAI_BehaviorBase *pNewBehavior )
{
	if ( pNewBehavior == &m_FuncTankBehavior )
	{
		m_bReadinessCapable = false;
	}
	else if ( pOldBehavior == &m_FuncTankBehavior )
	{
		m_bReadinessCapable = IsReadinessCapable();
	}

	BaseClass::OnChangeRunningBehavior( pOldBehavior, pNewBehavior );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Red::GatherConditions()
{
	BaseClass::GatherConditions();

	// Handle speech AI. Don't do AI speech if we're in scripts unless permitted by the EnableSpeakWhileScripting input.
	if ( m_NPCState == NPC_STATE_IDLE || m_NPCState == NPC_STATE_ALERT || m_NPCState == NPC_STATE_COMBAT ||
		( ( m_NPCState == NPC_STATE_SCRIPT ) && CanSpeakWhileScripting() ) )
	{
		DoCustomSpeechAI();
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Red::UseFunc( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	m_bDontUseSemaphore = true;
	SpeakIfAllowed( TLK_USE );
	m_bDontUseSemaphore = false;

	m_OnPlayerUse.FireOutput( pActivator, pCaller );
}

//-----------------------------------------------------------------------------
// Taken from npc_combine but edited to make Red regenerate
//-----------------------------------------------------------------------------
bool CNPC_Red::ShouldRegenerateHealth(void)
{
	return true;
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_red, CNPC_Red )

AI_END_CUSTOM_NPC()
