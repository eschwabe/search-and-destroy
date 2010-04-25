/*******************************************************************************
* Game Development Project
* SMWander.cpp
*
* Eric Schwabe
* 2010-04-24
*
* Wander state machine
*
*******************************************************************************/

#include "DXUT.h"
#include "SMWander.h"


// add new states
enum StateName 
{
	STATE_Move,   // note: first enum is the starting state
	STATE_AvoidObject,
    STATE_Idle,
};

// add new substates
enum SubstateName 
{
    // none
};


/**
* Constructor
*/
SMWander::SMWander( GameObject* object ) :
    StateMachine( *object )
{}

/**
* Deconstructor
*/
SMWander::~SMWander()
{}

/**
* State machine
*/
bool SMWander::States( State_Machine_Event event, MSG_Object* msg, int state, int substate )
{
BeginStateMachine

	// global message responses go here

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Move )

		OnEnter
            //// set object to patrol to position
            //m_owner->SetTargetPosition( m_vPos );
            //m_owner->SetVelocity(1.0f);
            //m_owner->SetAcceleration(0.25f);

        OnUpdate
         //   // determine if the object has arrived
         //   D3DXVECTOR3 vTarget = m_owner->GetTargetPosition() - m_owner->GetPosition();
         //   vTarget.y = 0;

	        //if( D3DXVec3Length( &vTarget ) < 0.01f )
         //   {
         //       m_owner->HoldPosition();
         //       ChangeState( STATE_Idle );
         //   }

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_AvoidObject )

		OnEnter

        //OnTimeInState(2.0f)
        //    // requeue the state machine to the end of the patrol list
        //    RequeueStateMachine();

    /*-------------------------------------------------------------------------*/

    DeclareState( STATE_Idle )


EndStateMachine
}
