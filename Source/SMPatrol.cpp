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
SMPatrol::SMPatrol( GameObject* object, const D3DXVECTOR2& vPatrolPos ) :
    StateMachine( *object ),
    m_vPatrolPos(vPatrolPos)
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
    // handle any attack messages

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_PatrolToPosition )

		OnEnter

            // set object speed
            m_owner->SetVelocity(2.0f);
            m_owner->SetAcceleration(0.5f);

        OnUpdate

            // determine direction (ignore height)
            D3DXVECTOR2 vDirection = m_vPatrolPos - m_owner->GetGridPosition();

            // determine if the object has arrived
	        if( D3DXVec2Length( &vDirection ) < 0.1f )
            {
                // switch to idle state
                ChangeState( STATE_Idle );
            }
            else
            {
                // set object direction towards position
                D3DXVec2Normalize(&vDirection, &vDirection);
                m_owner->SetGridDirection(vDirection);
            }

        OnExit

            // stop object
            m_owner->HoldPosition();
            

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Idle )

        OnEnter

            // required

        OnTimeInState(2.0f)
            
            // requeue the state machine to the end of the patrol list
            RequeueStateMachine();

	/*-------------------------------------------------------------------------*/

EndStateMachine
}
