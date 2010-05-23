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
#include "WorldData.h"

// add new states
enum StateName 
{
    STATE_ComputePath,      // note: first enum is the starting state 
	STATE_PatrolToPosition,   
	STATE_Idle,
    STATE_SwitchToCombat
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
	
    DeclareState( STATE_ComputePath )

        OnEnter

            // start path computation request
            g_world.AddPathRequest(m_owner->GetGridPosition(), m_vPatrolPos, m_owner->GetID());

        OnMsg(MSG_PathComputed)
            
            // move based on waypoint list
            ChangeState( STATE_PatrolToPosition );

    /*-------------------------------------------------------------------------*/

    DeclareState( STATE_PatrolToPosition )

		OnEnter
          
            // set object speed
            m_owner->SetVelocity(2.0f);
            m_owner->SetAcceleration(0.5f);

        OnUpdate

            PathWaypointList* waypointList = g_world.GetWaypointList(m_owner->GetID());

            // check if player nearby               
            D3DXVECTOR3 vPlayerDist = m_owner->GetPosition() - g_database.Find(m_idPlayer)->GetPosition();
            if( D3DXVec3Length( &vPlayerDist ) <= 3.0f )
            {
                // change state
                ChangeState( STATE_SwitchToCombat );
            }

            // if out of waypoints, switch to idle
            else if(waypointList->empty())
            {
                ChangeState( STATE_Idle );
            }
          
            // move towards next waypoint
            else
            {                
                // determine direction (ignore height)
                D3DXVECTOR2 vDirection = (*waypointList->begin()) - m_owner->GetGridPosition();

                // determine if the object has arrived
	            if( D3DXVec2Length( &vDirection ) < 0.1f )
                {
                    // pop off waypoint
                    waypointList->pop_front();
                }
                else
                {
                    // set object direction towards position
                    D3DXVec2Normalize(&vDirection, &vDirection);
                    m_owner->SetGridDirection(vDirection);
                }
            }

        OnExit

            // remove any waypoints
            g_world.ClearWaypointList(m_owner->GetID());

            // stop object
            m_owner->ResetMovement();
            

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Idle )

        OnEnter

            // required

        OnTimeInState(2.0f)
            
            // requeue the state machine to the end of the patrol list
            RequeueStateMachine();

	/*-------------------------------------------------------------------------*/

    DeclareState( STATE_SwitchToCombat )

        OnEnter

            // push seek player state machine
            PushStateMachine( *new SMCombat( m_owner, g_database.Find(m_idPlayer)->GetID(), false) );

	/*-------------------------------------------------------------------------*/

EndStateMachine
}
