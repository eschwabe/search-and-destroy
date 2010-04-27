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
#include "SMSeekPlayer.h"


// add new states
enum StateName 
{
	STATE_Initialize,   // note: first enum is the starting state
	STATE_Pursue,
    STATE_Flee,
    STATE_CapturePlayer,
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
SMSeekPlayer::SMSeekPlayer( GameObject* object, objectID pid ) :
    StateMachine( *object ),
    m_idPlayer(pid)
{
    if(m_idPlayer == INVALID_OBJECT_ID)
    {
        // if invalid, find the player
        dbCompositionList list;
        g_database.ComposeList(list, OBJECT_Player);
        m_idPlayer = (*list.begin())->GetID();
    }
}

/**
* Deconstructor
*/
SMSeekPlayer::~SMSeekPlayer(void)
{}

/**
* State machine
*/
bool SMSeekPlayer::States( State_Machine_Event event, MSG_Object* msg, int state, int substate )
{
BeginStateMachine

	// global message responses

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Initialize )

    	OnEnter

            ChangeState( STATE_Pursue );

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Pursue )

		OnEnter

            // set velocity and acceleration
            m_owner->SetVelocity(1.5f);
            m_owner->SetAcceleration(0.0f);

        OnUpdate

            // follow player
            GameObject* player = g_database.Find(m_idPlayer);
            D3DXVECTOR3 vNewDir = player->GetPosition() - m_owner->GetPosition();
            m_owner->SetDirection(player->GetPosition() - m_owner->GetPosition());

            // change state if player out of range
            if( D3DXVec3Length(&vNewDir) > 5.0f )
                ChangeState( STATE_Stunned );

        OnExit

            // reset object
            m_owner->ResetMovement();

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Flee )

		OnEnter

        OnUpdate

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_CapturePlayer )

		OnEnter

        OnUpdate

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Stunned )

		OnEnter

            // stop object from moving
            m_owner->StopMovement();

        OnUpdate

            // rotate or shake each update

        OnTimeInState(5.0f)

            // enable movement
            m_owner->ResumeMovement();

            // end state machine after duration
            PopStateMachine();

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Dead )

		OnEnter

        OnUpdate

    /*-------------------------------------------------------------------------*/

EndStateMachine
}