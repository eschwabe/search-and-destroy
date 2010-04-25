/*******************************************************************************
* Game Development Project
*
* Eric Schwabe
* 2010-04-25
*
* Seek player state machine
*
*******************************************************************************/

#include "DXUT.h"
#include "SMSeekPlayer.h"


// add new states
enum StateName 
{
	STATE_Initialize,   // note: first enum is the starting state
	STATE_Pursue,
    STATE_Flee,
    STATE_CapturePlayer,
    STATE_Stunned,
    STATE_Dead
};

// add new substates
enum SubstateName 
{
    // none
};


/**
* Constructor
*/
SMSeekPlayer::SMSeekPlayer( GameObject* object ) :
    StateMachine( *object )
{}

/**
* Deconstructor
*/
SMSeekPlayer::~SMSeekPlayer(void)
{}

/**
* State machine
*/
bool SMSeekPlayer::States( State_Machine_Event event, MSG_Object* msg, int state, int substate )
{
BeginStateMachine

	// global message responses go here

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Initialize )

    	OnEnter
            // select player

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Pursue )

		OnEnter

        OnUpdate

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Flee )

		OnEnter

        OnUpdate

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_CapturePlayer )

		OnEnter

        OnUpdate

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Stunned )

		OnEnter

        OnUpdate

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Dead )

		OnEnter

        OnUpdate

    /*-------------------------------------------------------------------------*/

EndStateMachine
}
