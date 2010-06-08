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
#include "global.h"
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
SMProjectile::SMProjectile( GameObject* object, const float fVel, const float fAccel, const D3DXVECTOR3 vDir ) :
    StateMachine( *object ),
    m_fVel(fVel),
    m_fAccel(fAccel),
    m_fDist(0.0f),
    m_vDir(vDir),
    m_vInitialPos(0.0f, 0.0f, 0.0f),
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

            // set direction and follow it
            m_owner->SetDirection(m_vDir);
            ChangeState( STATE_FollowPath );

            // save initial object position
            m_vInitialPos = m_owner->GetPosition();

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_FollowPath )

		OnEnter

            // compute max travel distance
            D3DXVec3Normalize(&m_vDir, &m_vDir);
            D3DXVECTOR3 vFinalPos = m_vInitialPos + (m_vDir*50);
            CollOutput output;

            // find future projectile collision with world
            if( g_objcollision.RunLineCollision(m_vInitialPos, vFinalPos, &output) )
            {
                m_fDist = D3DXVec3Length( &(output.point - m_vInitialPos) );
            }
            else
            {
                m_fDist = 25.0f;
            }

        OnUpdate

            bool bExpire = false;

            // check for NPC collisions
            dbCompositionList list;
            g_database.ComposeList(list, OBJECT_NPC);
            for(dbCompositionList::iterator it = list.begin(); it < list.end(); ++it)
            {
                if( g_objcollision.RunObjectCollision(m_owner, (*it)) )
                {
                    // send damage message on collision and expire
                    SendMsgDelayed(0.1f, MSG_Damaged, (*it)->GetID(), MSG_Data(50));
                    bExpire = true;
                    break;
                }
            }

            // check if past max distance
            D3DXVECTOR3 vTravelled = m_owner->GetPosition() - m_vInitialPos;
            if( D3DXVec3Length(&vTravelled) >= m_fDist )
            {
                bExpire = true;
            }

            // expire projectile if event detected
            if(bExpire)
            {
                ChangeState( STATE_Expired );
            }

        OnTimeInState(10.0f)

            // expire after timeout (should not normally occur)
            ChangeState( STATE_Expired );

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Expired )

		OnEnter

            // stop movement (plays explosion animation)
            m_owner->StopMovement();

        OnTimeInState(0.5f)

            // delete projectile object
            MarkForDeletion();

    /*-------------------------------------------------------------------------*/
	
EndStateMachine
}