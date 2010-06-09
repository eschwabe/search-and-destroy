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
#include "SMCombat.h"
#include "global.h"
#include "WorldData.h"

// add new states
enum StateName 
{
	STATE_Initialize,   // note: first enum is the starting state
	STATE_PursuePlayer,
    STATE_FollowPlayerPath,
	STATE_LostPlayer,
    STATE_AttackPlayer,
    STATE_Damaged,
    STATE_Dying,
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
SMCombat::SMCombat( GameObject* object, objectID pid, bool damaged ) :
    StateMachine( *object ),
    m_idPlayer(pid),
    m_bDamaged(damaged)
{}

/**
* Deconstructor
*/
SMCombat::~SMCombat(void)
{}

/**
* State machine
*/
bool SMCombat::States( State_Machine_Event event, MSG_Object* msg, int state, int substate )
{
BeginStateMachine

	// global message responses
    OnMsg(MSG_Damaged)

        // update health and seek player
        m_owner->SetHealth( m_owner->GetHealth() - msg->GetIntData() );
        ChangeState( STATE_Damaged );

    OnMsg(MSG_Reset)

        // reinitialize state machine
        m_bDamaged = false;
        ChangeState( STATE_Initialize );

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Initialize )

    	OnEnter

            // if damaged, become stunned
            if(m_bDamaged)
                ChangeState( STATE_Damaged );

            // otherwise, pursue player
            else
                ChangeState( STATE_PursuePlayer );

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_PursuePlayer )

		OnEnter

            // create path to player
            GameObject* player = g_database.Find(m_idPlayer);

            // start path computation request
            g_world.AddPathRequest(m_owner->GetGridPosition(), player->GetGridPosition(), m_owner->GetID());

        OnMsg(MSG_PathComputed)

            // follow path
            ChangeStateDelayed(0.1f, STATE_FollowPlayerPath);

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_FollowPlayerPath )

        OnEnter

            // set velocity and acceleration
            m_owner->SetVelocity(2.0f);
            m_owner->SetAcceleration(0.5f);

        OnUpdate

            PathWaypointList* waypointList = g_world.GetWaypointList(m_owner->GetID());

            // check if touched player
            D3DXVECTOR2 vPlayerDist = m_owner->GetGridPosition() - g_database.Find(m_idPlayer)->GetGridPosition();
            if( D3DXVec2Length(&vPlayerDist) < 0.5f )
            {
                ChangeState(STATE_AttackPlayer);
            }

            // if out of waypoints
            else if(waypointList->empty())
            {
                // check if player out of range
                if( D3DXVec2Length(&vPlayerDist) > 5.0f )
                {
                    ChangeState(STATE_LostPlayer);
                }

                // else, pick new path
                else
                {
                    ChangeState( STATE_PursuePlayer );
                }
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

            // reset object
            m_owner->ResetMovement();

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_AttackPlayer )

		OnEnter
            
            // send damage message to player
            SendMsgDelayed(0.1f, MSG_Damaged, m_idPlayer);

            // end state machine after duration
            PopStateMachine();
    
    /*-------------------------------------------------------------------------*/

    DeclareState( STATE_LostPlayer )

		OnEnter
            
            // stop object from moving
            m_owner->StopMovement();
            
        OnTimeInState(2.0f)

            // enable movement
            m_owner->ResumeMovement();

            // end state machine after duration
            PopStateMachine();

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Damaged )

		OnEnter

            // stop object from moving (stunned)
            m_owner->StopMovement();

        OnUpdate

            // check health, dead if 0
            if( m_owner->GetHealth() <= 0 )
            {
                // enable movement
                m_owner->ResumeMovement();
                ChangeState( STATE_Dying );
            }

        OnTimeInState(1.0f)

            // enable movement
            m_owner->ResumeMovement();

            // end state machine after duration
            PopStateMachine();

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Dying )

		OnEnter

            // lower object to ground
            m_owner->SetDirection( D3DXVECTOR3(0.0f, -0.01f, 0.0f) );
            m_owner->SetVelocity( 0.1f );

        OnUpdate

            // if on ground, dead
            if( (m_owner->GetPosition().y - (m_owner->GetHeight() / 2.0f)) <= 0 )
                ChangeState( STATE_Dead );

        OnExit

            // reset 
            m_owner->ResetMovement();

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Dead )

		OnEnter
        
            // remove any waypoints
            g_world.ClearWaypointList(m_owner->GetID());

            // can no longer move (or do anything)
            m_owner->StopMovement();

    /*-------------------------------------------------------------------------*/

EndStateMachine
}
