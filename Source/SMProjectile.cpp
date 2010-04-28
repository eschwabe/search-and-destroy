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
#include "Collision.h"

// add new states
enum StateName 
{
	STATE_Initialize,   // note: first enum is the starting state
	//STATE_FollowObject,
    STATE_FollowPath,
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
SMProjectile::SMProjectile( GameObject* object, const float fVel, const float fAccel, const D3DXVECTOR3 vDir, const bool bSeek ) :
    StateMachine( *object ),
    m_fVel(fVel),
    m_fAccel(fAccel),
    m_vDir(vDir),
    m_bSeek(bSeek),
    m_pID(INVALID_OBJECT_ID)
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

            // set projectile parameters
            m_owner->SetVelocity(m_fVel);
            m_owner->SetAcceleration(m_fAccel);

            if(false /*bSeek*/)
            {
                // select nearest NPC
                //dbCompositionList list;
                //g_database.ComposeList(list, OBJECT_NPC);
                //for(dbCompositionList::iterator it = list.begin(); it < list.end(); ++it)
                //{
                //    D3DXVECTOR3 vPlayerDist = m_owner->GetPosition() - (*it)->GetPosition();
                //    if( D3DXVec3Length( &vPlayerDist ) <= 3.0f )
                //    {
                //    }
                //}

                //// follow selected NPC
                //ChangeState( STATE_FollowObject );
            }
            else
            {
                // set direction and follow it
                m_owner->SetDirection(m_vDir);
                ChangeState( STATE_FollowPath );
            }

    /*-------------------------------------------------------------------------*/
	/*
    DeclareState( STATE_FollowObject )

		OnEnter

        OnUpdate
    */
    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_FollowPath )

		OnEnter

        OnUpdate

            // check for NPC collisions
            dbCompositionList list;
            g_database.ComposeList(list, OBJECT_NPC);
            for(dbCompositionList::iterator it = list.begin(); it < list.end(); ++it)
            {
                if( g_objcollision.RunObjectCollision(m_owner, (*it)) )
                {
                    // send damage message on collision and expire
                    SendMsgDelayed(0.1f, MSG_Damaged, (*it)->GetID(), MSG_Data(50));
                    ChangeState( STATE_Expired );
                    break;
                }
            }

        OnTimeInState(2.5f)

            ChangeState( STATE_Expired );

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Expired )

		OnEnter

            // stop movement
            m_owner->ResetMovement();

            // play explosion animation (set health to 0?)

        OnTimeInState(1.0f)

            // delete projectile object
            MarkForDeletion();

    /*-------------------------------------------------------------------------*/
	
EndStateMachine
}