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

// add new states
enum StateName 
{
	STATE_Initialize,   // note: first enum is the starting state
	STATE_PursuePlayer,
	STATE_LostPlayer,
    //STATE_Flee,
    //STATE_CapturePlayer,
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

            // set velocity and acceleration
            m_owner->SetVelocity(1.5f);
            m_owner->SetAcceleration(0.0f);

        OnUpdate

            // follow player
            GameObject* player = g_database.Find(m_idPlayer);
            D3DXVECTOR2 vNewDir = player->GetGridPosition() - m_owner->GetGridPosition();
            m_owner->SetGridDirection(vNewDir);

            // change state if player out of range
            if( D3DXVec2Length(&vNewDir) > 5.0f )
                ChangeState( STATE_LostPlayer );

        OnExit

            // reset object
            m_owner->ResetMovement();

	/*-------------------------------------------------------------------------*/
	/*
    DeclareState( STATE_Flee )

		OnEnter

        OnUpdate
    */
	/*-------------------------------------------------------------------------*/
	/*
    DeclareState( STATE_CapturePlayer )

		OnEnter

        OnUpdate
    */
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

            // stop object from moving
            m_owner->StopMovement();

        OnUpdate

            // check health, dead if 0
            if( m_owner->GetHealth() <= 0 )
            {
                // enable movement
                m_owner->ResumeMovement();
                ChangeState( STATE_Dying );
            }

            // stunned, rotate or shake each update

        OnTimeInState(5.0f)

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
        
            // can no longer move (or do anything)
            m_owner->StopMovement();

    /*-------------------------------------------------------------------------*/

EndStateMachine
}
