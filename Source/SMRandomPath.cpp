/*******************************************************************************
* Game Development Project
* SMRandomPath.cpp
*
* Eric Schwabe
* 2010-05-18
*
* Random path state machine
*
*******************************************************************************/

#include "DXUT.h"
#include "SMRandomPath.h"
#include "SMCombat.h"
#include "WorldPath.h"

// add new states
enum StateName 
{
	STATE_PickPath,           // note: first enum is the starting state
	STATE_FollowPath,
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
SMRandomPath::SMRandomPath( GameObject* object, objectID pid ) :
    StateMachine( *object ),
    m_idPlayer(pid)
{}

/**
* Deconstructor
*/
SMRandomPath::~SMRandomPath()
{}

/**
* State machine
*/
bool SMRandomPath::States( State_Machine_Event event, MSG_Object* msg, int state, int substate )
{
BeginStateMachine

	// global message responses
    OnMsg(MSG_Damaged)

        // update health and seek player
        m_owner->SetHealth( m_owner->GetHealth() - msg->GetIntData() );
        PushStateMachine( *new SMCombat( m_owner, g_database.Find(m_idPlayer)->GetID(), true) );

    /*-------------------------------------------------------------------------*/

    DeclareState( STATE_PickPath )
    
        OnEnter

            // start path computation request
            g_world.AddPathRequest(m_owner->GetGridPosition(), g_world.GetRandomMapLocation(), m_owner->GetID());

        OnMsg(MSG_PathComputed)

            // follow path
            ChangeStateDelayed(1.0f, STATE_FollowPath);

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_FollowPath )
	
        OnEnter

            // set velocity
            m_owner->SetVelocity(5.0f);
            m_owner->SetAcceleration(0.5f);

        OnUpdate

            PathWaypointList* waypointList = g_world.GetWaypointList(m_owner->GetID());

            // check if player nearby               
            D3DXVECTOR3 vPlayerDist = m_owner->GetPosition() - g_database.Find(m_idPlayer)->GetPosition();
            if( D3DXVec3Length( &vPlayerDist ) <= 3.0f )
            {
                ChangeState( STATE_SwitchToCombat );
            }

            // if out of waypoints, pick new path
            else if(waypointList->empty())
            {
                ChangeState( STATE_PickPath );
            }            
                
            // move towards waypoint
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

    DeclareState( STATE_SwitchToCombat )

    	OnEnter
            
            // push combat state machine
            PushStateMachine( *new SMCombat( m_owner, g_database.Find(m_idPlayer)->GetID(), false) );

    /*-------------------------------------------------------------------------*/

EndStateMachine
}
