//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef SCENEENTITY_H
#define SCENEENTITY_H
#ifdef _WIN32
#pragma once
#endif

// List of the last 5 lines of speech from NPCs for bug reports
#define SPEECH_LIST_MAX_SOUNDS	5

#ifndef NEW_RESPONSE_SYSTEM
class AI_Response;
#endif

struct recentNPCSpeech_t
{
	float	time;
	char	name[ 512 ];
	char	sceneName[ 128 ];
};

int GetRecentNPCSpeech( recentNPCSpeech_t speech[ SPEECH_LIST_MAX_SOUNDS ] );
float InstancedScriptedScene( CBaseFlex *pActor, const char *pszScene,  EHANDLE *phSceneEnt = NULL, float flPostDelay = 0.0f, bool bIsBackground = false, AI_Response *response = NULL, bool bMultiplayer = false, IRecipientFilter *filter = NULL );
#ifdef MAPBASE
float InstancedAutoGeneratedSoundScene( CBaseFlex *pActor, char const *soundname, EHANDLE *phSceneEnt = NULL, float flPostDelay = 0.0f, bool bIsBackground = false, AI_Response *response = NULL, bool bMultiplayer = false, IRecipientFilter *filter = NULL );
#else
float InstancedAutoGeneratedSoundScene( CBaseFlex *pActor, char const *soundname, EHANDLE *phSceneEnt = NULL );
#endif
void StopScriptedScene( CBaseFlex *pActor, EHANDLE hSceneEnt );
void RemoveActorFromScriptedScenes( CBaseFlex *pActor, bool instancedscenesonly, bool nonidlescenesonly = false, const char *pszThisSceneOnly = NULL );
void RemoveAllScenesInvolvingActor( CBaseFlex *pActor );
void PauseActorsScriptedScenes( CBaseFlex *pActor, bool instancedscenesonly );
void ResumeActorsScriptedScenes( CBaseFlex *pActor, bool instancedscenesonly );
void QueueActorsScriptedScenesToResume( CBaseFlex *pActor, bool instancedscenesonly );
bool IsRunningScriptedScene( CBaseFlex *pActor, bool bIgnoreInstancedScenes = true );
bool IsRunningScriptedSceneAndNotPaused( CBaseFlex *pActor, bool bIgnoreInstancedScenes = true );
bool IsRunningScriptedSceneWithSpeech( CBaseFlex *pActor, bool bIgnoreInstancedScenes = false );
bool IsRunningScriptedSceneWithSpeechAndNotPaused( CBaseFlex *pActor, bool bIgnoreInstancedScenes = false );
#ifdef MAPBASE
bool IsRunningScriptedSceneWithFlexAndNotPaused( CBaseFlex *pActor, bool bIgnoreInstancedScenes = false, const char *pszNotThisScene = NULL );
bool IsTalkingInAScriptedScene( CBaseFlex *pActor, bool bIgnoreInstancedScenes = false );
CUtlVector< CHandle< CSceneEntity > > *GetActiveSceneList();
#endif
#ifdef EZ2
// Blixibon - This was added so Bad Cop stops interrupting his one-line scenes.
bool IsTalkingInAScriptedScene( CBaseFlex *pActor, bool bIgnoreInstancedScenes = false );
#endif
float GetSceneDuration( char const *pszScene );
float GetSceneSpeechDuration( char const* pszScene );
int GetSceneSpeechCount( char const *pszScene );
bool IsInInterruptableScenes( CBaseFlex *pActor );

void PrecacheInstancedScene( char const *pszScene );
HSCRIPT ScriptCreateSceneEntity( char const *pszScene );

char const *GetSceneFilename( CBaseEntity *ent );
void ReloadSceneFromDisk( CBaseEntity *ent );

#ifdef MAPBASE
const char *GetFirstSoundInScene(const char *pszScene);
const char *GetFirstSoundInScene(CChoreoScene *scene);

CBaseEntity *UTIL_FindNamedSceneEntity(const char *name, CBaseEntity *pActor, CSceneEntity *scene, bool bBaseFlexOnly = false, bool bUseClear = false);
#endif


#endif // SCENEENTITY_H
