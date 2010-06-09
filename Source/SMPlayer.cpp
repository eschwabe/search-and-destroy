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
    STATE_FireProjectiles,
    STATE_FireBigProjectile,
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
    m_pd3dDevice(pd3dDevice),
    m_iProjectileCount(0)
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
    OnMsg(MSG_Damaged)

        // player damaged, dead
        ChangeState(STATE_Dead);

    OnMsg(MSG_Reset)

        // reinitialize state machine
        ChangeState( STATE_Initialize );



    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Initialize )

    	OnEnter

            ChangeState( STATE_Move );

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Move )

		OnEnter

        OnMsg( MSG_StartFireProjectiles )
                
                // change state
                ChangeState(STATE_FireProjectiles);

        OnMsg( MSG_FireBigProjectile )
                
                // change state
                ChangeState(STATE_FireBigProjectile);

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_FireProjectiles )

		OnEnter
            
            // reset projectile count
            m_iProjectileCount = 0;

        OnPeriodicTimeInState(0.10f)

            // only allow 3 projectiles to be shot consecutively
            if(m_iProjectileCount < 3)
            {
                // create new projectiles periodically
                D3DXVECTOR3 vInitPos = m_owner->GetPosition();
                vInitPos.y = m_owner->GetHeight();            
                ProjectileParticles* projectile = new ProjectileParticles(vInitPos, ProjectileParticles::kLightBall);

                // initialize 
                projectile->InitializeObject(m_pd3dDevice);

                // create state machine and add to db
                SMProjectile* smProj = new SMProjectile(projectile, 8.5f, 0.5f, 35, m_owner->GetDirection()); 
                projectile->GetStateMachineManager()->PushStateMachine( *smProj, STATE_MACHINE_QUEUE_0, TRUE );
                g_database.Store(projectile);

                ++m_iProjectileCount;
            }

        OnPeriodicTimeInState(0.8f)

            // reset projectile count
            m_iProjectileCount = 0;

        OnMsg(MSG_StopFireProjectiles)

            // stop firing
            ChangeState(STATE_Move);

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
            SMProjectile* smProj = new SMProjectile(projectile, 2.0f, 0.2f, 250, m_owner->GetDirection()); 
            projectile->GetStateMachineManager()->PushStateMachine( *smProj, STATE_MACHINE_QUEUE_0, TRUE );
            g_database.Store(projectile);
            
        OnMsg( MSG_StartFireProjectiles )
            
            // change state
            ChangeState(STATE_FireProjectiles);

        OnTimeInState(1.0f)

            // change back to move state
            ChangeStateDelayed(0.1f, STATE_Move);


    /*-------------------------------------------------------------------------*/
    
    DeclareState( STATE_Dead )

		OnEnter

            m_owner->ResetMovement();
            m_owner->StopMovement();
            m_owner->SetHealth(0);
    
    /*-------------------------------------------------------------------------*/

EndStateMachine
}