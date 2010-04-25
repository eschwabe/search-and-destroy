/*******************************************************************************
* Game Development Project
* SMPatrol.h
*
* Eric Schwabe
* 2010-04-20
*
* Patrol state machine
*
*******************************************************************************/

#include "DXUT.h"
#include "SMPatrol.h"
#include "database.h"

// add new states
enum StateName 
{
	STATE_PatrolToPosition,   // note: first enum is the starting state
	STATE_Idle
};

// add new substates
enum SubstateName 
{
    // none
};

/**
* Constructor
*/
SMPatrol::SMPatrol( GameObject* object, const D3DXVECTOR3& pos ) :
    StateMachine( *object ),
    m_vPos(pos)
{}

/**
* Deconstructor
*/
SMPatrol::~SMPatrol()
{}

/**
* State machine
*/
bool SMPatrol::States( State_Machine_Event event, MSG_Object* msg, int state, int substate )
{
BeginStateMachine

	// global message responses go here

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_PatrolToPosition )

		OnEnter
            // set object to patrol to position
            m_owner->SetTargetPosition( m_vPos );
            m_owner->SetVelocity(1.0f);
            m_owner->SetAcceleration(0.25f);

        OnUpdate
            // determine if the object has arrived
            D3DXVECTOR3 vTarget = m_owner->GetTargetPosition() - m_owner->GetPosition();
            vTarget.y = 0;

	        if( D3DXVec3Length( &vTarget ) < 0.01f )
            {
                m_owner->HoldPosition();
                ChangeState( STATE_Idle );
            }

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Idle )

		OnEnter

        OnTimeInState(2.0f)
            // requeue the state machine to the end of the patrol list
            RequeueStateMachine();


EndStateMachine
}
