//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_Red : public C_AI_BaseNPC
{
public:
	DECLARE_CLASS( C_Red, C_AI_BaseNPC );
	DECLARE_CLIENTCLASS();

					C_Red();
	virtual			~C_Red();

private:
	C_Red( const C_Red & ); // not defined, not accessible
};

IMPLEMENT_CLIENTCLASS_DT(C_Red, DT_NPC_Red, CNPC_Red)
END_RECV_TABLE()

C_Red::C_Red()
{
}


C_Red::~C_Red()
{
}


