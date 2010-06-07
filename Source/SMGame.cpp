/*******************************************************************************
* Game Development Project
*
* Eric Schwabe
* 2010-04-25
*
* Game state machine
*
*******************************************************************************/

#include "DXUT.h"
#include "SMGame.h"
#include "WorldData.h"

// add new states
enum StateName 
{
	STATE_GameInitialize,   // note: first enum is the starting state
	STATE_GameStartScreen,
    STATE_GameRunning,
    STATE_GameLose,
    STATE_GameWin,
    STATE_GamePause,
    STATE_GameDebug
};

// add new substates
enum SubstateName 
{
    // none
};

/**
* Constructor
*/
SMGame::SMGame( GameController* controller, objectID mapID ) :
    StateMachine( *controller ),
    m_controller(controller),
    m_mapID(mapID)
{}

/**
* Deconstructor
*/
SMGame::~SMGame()
{}

/**
* State machine
*/
bool SMGame::States( State_Machine_Event event, MSG_Object* msg, int state, int substate )
{
BeginStateMachine

	// global message responses go here

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_GameInitialize )

    	OnEnter
            // set menu title
            m_controller->SetTitleText(L"Search And Destroy");
            ChangeState(STATE_GameStartScreen);

    /*-------------------------------------------------------------------------*/

    DeclareState( STATE_GameStartScreen )

		OnEnter

            // show title screen and set world view camera
            m_controller->ShowTitleScreen();
            m_controller->SetCameraType(GameController::CAMERA_WORLDVIEW);

            // hide minimap
            g_database.Find(m_mapID)->DisableObjectRender();

            // hold all objects
            dbCompositionList pList;
            g_database.ComposeList( pList, OBJECT_Ignore_Type );

            for(dbCompositionList::iterator it = pList.begin(); it != pList.end(); ++it)
            {
                (*it)->StopMovement();
            }

        OnMsg(MSG_DebugMode)

            // go to debug mode
            ChangeState(STATE_GameDebug);

        OnMsg(MSG_GameStart)
            
            // reset all objects
            dbCompositionList pList;
            g_database.ComposeList( pList, OBJECT_Ignore_Type );

            for(dbCompositionList::iterator it = pList.begin(); it != pList.end(); ++it)
            {
                (*it)->ResetMovement();
                (*it)->ResetPosition();
                (*it)->SetHealth(100);
                g_database.SendMsgFromSystem((*it)->GetID(), MSG_Reset);
            }

            // start game
            ChangeState(STATE_GameRunning);

        OnExit

            // hide title screen
            m_controller->HideTitleScreen();

            // show minimap
            g_database.Find(m_mapID)->EnableObjectRender();

	/*-------------------------------------------------------------------------*/

    DeclareState( STATE_GameRunning )

		OnEnter
            // player camera
            m_controller->SetCameraType(GameController::CAMERA_PLAYER);

        OnUpdate
            // check for win and lose conditions
            bool bPlayerAlive = false;
            bool bNPCAlive = false;

            // generate list of players and NPCs
            dbCompositionList pList;
            g_database.ComposeList( pList, OBJECT_NPC | OBJECT_Player );

            // check for player collisions with environment
            for(dbCompositionList::iterator it = pList.begin(); it != pList.end(); ++it)
            {
                if( (*it)->GetType() == OBJECT_NPC && (*it)->GetHealth() > 0 )
                    bNPCAlive = true;
                else if( (*it)->GetType() == OBJECT_Player && (*it)->GetHealth() > 0 )
                    bPlayerAlive = true;
            }

            // all players dead
            if( !bPlayerAlive )
                ChangeState(STATE_GameLose);

            // all NPCs dead
            else if( !bNPCAlive )
                ChangeState(STATE_GameWin);

        OnMsg(MSG_DebugMode)

            // go to debug mode
            ChangeState(STATE_GameDebug);

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_GameLose )

		OnEnter
            // set menu title
            m_controller->SetTitleText(L"You Lose!");
            ChangeState(STATE_GameStartScreen);

	/*-------------------------------------------------------------------------*/

	DeclareState( STATE_GameWin )

		OnEnter
            // set menu title
            m_controller->SetTitleText(L"You Win!");
            ChangeState(STATE_GameStartScreen);

	/*-------------------------------------------------------------------------*/
    DeclareState( STATE_GamePause )

		OnEnter
            // circle player camera
            // stop updates

        OnUpdate

    /*-------------------------------------------------------------------------*/
	    
    DeclareState( STATE_GameDebug )

		OnEnter

            // debug camera
            m_controller->SetCameraType(GameController::CAMERA_DEBUG);

            // display debug information
            m_controller->ShowDebugInfo();
            g_world.ShowPathDebug();

        OnMsg(MSG_DebugMode)

            // return to previous state
            PopState();

        OnExit

            // hide debug information
            m_controller->HideDebugInfo();
            g_world.HidePathDebug();
            g_world.SetTerrainAnalysisType(WorldData::kTerrainAnalysisNone);

    /*-------------------------------------------------------------------------*/

EndStateMachine
}