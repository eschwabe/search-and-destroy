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
            
            // enable movement
            m_owner->ResumeMovement();

        OnMsg( MSG_FireProjectile )
            
            // create projectile
            ProjectileParticles* projectile = new ProjectileParticles(m_owner->GetPosition(), ProjectileParticles::kFire);
            projectile->InitializeObject(m_pd3dDevice);
            g_database.Store(projectile);

            // change state
            ChangeState(STATE_FireProjectile);

        OnMsg( MSG_FireBigProjectile )
            
            // change state
            ChangeState(STATE_FireBigProjectile);

        OnExit

            // disable movement
            m_owner->StopMovement();

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_FireProjectile )

		OnEnter
            
            // create projectile
            ChangeStateDelayed(0.25f, STATE_Move);

	/*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_FireBigProjectile )

		OnEnter
            
            // create projectile
            ChangeStateDelayed(0.5f, STATE_Move);

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