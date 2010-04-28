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
#include "SMCombat.h"

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
SMPatrol::SMPatrol( GameObject* object, const D3DXVECTOR2& vPatrolPos, objectID pid ) :
    StateMachine( *object ),
    m_vPatrolPos(vPatrolPos),
    m_idPlayer(pid)
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

	// global message responses
    OnMsg(MSG_Damaged)

        // update health and seek player
        m_owner->SetHealth( m_owner->GetHealth() - msg->GetIntData() );
        PushStateMachine( *new SMCombat( m_owner, g_database.Find(m_idPlayer)->GetID(), true) );

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_PatrolToPosition )

		OnEnter

            // set object speed
            m_owner->SetVelocity(2.0f);
            m_owner->SetAcceleration(0.5f);

        OnUpdate

            // check if player nearby               
            D3DXVECTOR3 vPlayerDist = m_owner->GetPosition() - g_database.Find(m_idPlayer)->GetPosition();
            if( D3DXVec3Length( &vPlayerDist ) <= 3.0f )
            {
                // push seek player state machine
                PushStateMachine( *new SMCombat( m_owner, g_database.Find(m_idPlayer)->GetID(), false) );
            }

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
            m_owner->ResetMovement();
            

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Idle )

        OnEnter

            // required

        OnTimeInState(2.0f)
            
            // requeue the state machine to the end of the patrol list
            RequeueStateMachine();

EndStateMachine
}
