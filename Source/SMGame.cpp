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

// add new states
enum StateName 
{
	STATE_GameInitialize,   // note: first enum is the starting state
	STATE_GameRunning,
    STATE_GameOver,
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
SMGame::SMGame( GameObject* object ) :
    StateMachine( *object )
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
            // circle map camera
            // show menu

        OnExit
            // hide menu

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_GameRunning )

		OnEnter
            // third-person camera

        OnExit

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_GameOver )

		OnEnter
            // circle player camera

        OnUpdate

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
            // display debug information

        OnUpdate

    /*-------------------------------------------------------------------------*/

EndStateMachine
}