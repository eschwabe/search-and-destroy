/*******************************************************************************
* Game Development Project
*
* Eric Schwabe
* 2010-04-25
*
* Projectile state machine
*
*******************************************************************************/

#include "DXUT.h"
#include "SMProjectile.h"


// add new states
enum StateName 
{
	STATE_Initialize,   // note: first enum is the starting state
	STATE_FollowObject,
    STATE_AttackObject,
    STATE_Expired
};

// add new substates
enum SubstateName 
{
    // none
};


/**
* Constructor
*/
SMProjectile::SMProjectile( GameObject* object ) :
    StateMachine( *object )
{}

/**
* Deconstructor
*/
SMProjectile::~SMProjectile(void)
{}

/**
* State machine
*/
bool SMProjectile::States( State_Machine_Event event, MSG_Object* msg, int state, int substate )
{
BeginStateMachine

	// global message responses go here

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Initialize )

    	OnEnter
            // select player

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_FollowObject )

		OnEnter

        OnUpdate

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_AttackObject )

		OnEnter

        OnUpdate

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Expired )

		OnEnter
            // MarkForDeletion()

        OnUpdate

    /*-------------------------------------------------------------------------*/
	
EndStateMachine
}