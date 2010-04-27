/*******************************************************************************
* Game Development Project
*
* Eric Schwabe
* 2010-04-25
*
* Player state machine
*
*******************************************************************************/

#include "DXUT.h"
#include "SMPlayer.h"
#include "SMProjectile.h"
#include "ProjectileParticles.h"

// add new states
enum StateName 
{
	STATE_Initialize,   // note: first enum is the starting state
	STATE_Move,
    STATE_FireProjectile,
    STATE_FireBigProjectile,
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
SMPlayer::SMPlayer( GameObject* object, IDirect3DDevice9* pd3dDevice ) :
    StateMachine( *object ),
    m_pd3dDevice(pd3dDevice)
{}

/**
* Deconstructor
*/
SMPlayer::~SMPlayer(void)
{}

/**
* State machine
*/
bool SMPlayer::States( State_Machine_Event event, MSG_Object* msg, int state, int substate )
{
BeginStateMachine

	// global message responses go here

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Initialize )

    	OnEnter

            ChangeState( STATE_Move );

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Move )

		OnEnter
            
        OnMsg( MSG_FireProjectile )
            
            // change state
            ChangeState(STATE_FireProjectile);

        OnMsg( MSG_FireBigProjectile )
            
            // change state
            ChangeState(STATE_FireBigProjectile);

        OnExit

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_FireProjectile )

		OnEnter

            // create projectile
            D3DXVECTOR3 vInitPos = m_owner->GetPosition();
            vInitPos.y = m_owner->GetHeight();            
            ProjectileParticles* projectile = new ProjectileParticles(vInitPos, ProjectileParticles::kLightBall);

            // initialize 
            projectile->InitializeObject(m_pd3dDevice);

            // create state machine and add to db
            SMProjectile* smProj = new SMProjectile(projectile, 3.5f, 0.3f, m_owner->GetDirection(), false); 
            projectile->GetStateMachineManager()->PushStateMachine( *smProj, STATE_MACHINE_QUEUE_0, TRUE );
            g_database.Store(projectile);

            // create projectile
            ChangeStateDelayed(0.1f, STATE_Move);

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_FireBigProjectile )

		OnEnter

            // create projectile
            D3DXVECTOR3 vInitPos = m_owner->GetPosition();
            vInitPos.y = m_owner->GetHeight();            
            ProjectileParticles* projectile = new ProjectileParticles(vInitPos, ProjectileParticles::kBigLightBall);

            // initialize
            projectile->InitializeObject(m_pd3dDevice);

            // create state machine and add to db
            SMProjectile* smProj = new SMProjectile(projectile, 1.5f, 0.2f, m_owner->GetDirection(), false); 
            projectile->GetStateMachineManager()->PushStateMachine( *smProj, STATE_MACHINE_QUEUE_0, TRUE );
            g_database.Store(projectile);
            
            // create projectile
            ChangeStateDelayed(1.0f, STATE_Move);

    /*-------------------------------------------------------------------------*/
	    
    DeclareState( STATE_Stunned )

		OnEnter

        OnUpdate

    /*-------------------------------------------------------------------------*/

    DeclareState( STATE_Dead )

		OnEnter

        OnUpdate

    /*-------------------------------------------------------------------------*/

EndStateMachine
}