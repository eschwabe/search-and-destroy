/*******************************************************************************
* Game Development Project
*
* Eric Schwabe
* 2010-04-25
*
* Player state machine
*
*******************************************************************************/

#include "DXUT.h"
#include "SMPlayer.h"


// add new states
enum StateName 
{
	STATE_Initialize,   // note: first enum is the starting state
	STATE_Move,
    STATE_FireProjectile,
    STATE_FireBigProjectile,
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
SMPlayer::SMPlayer( GameObject* object ) :
    StateMachine( *object )
{}

/**
* Deconstructor
*/
SMPlayer::~SMPlayer(void)
{}

/**
* State machine
*/
bool SMPlayer::States( State_Machine_Event event, MSG_Object* msg, int state, int substate )
{
BeginStateMachine

	// global message responses go here

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Initialize )

    	OnEnter

            // select player

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Move )

		OnEnter
            // Enable movement

        //OnMsg( MSG_FireProjectile )
            // Change state

        //OnMsg( MSG_FireProjectile )
            // Change state

        OnExit
            // Disable movement

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_FireProjectile )

		OnEnter
            // Create projectile
            // Delayed state change

        OnUpdate

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_FireBigProjectile )

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