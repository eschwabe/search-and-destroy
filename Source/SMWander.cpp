/*******************************************************************************
* Game Development Project
* SMWander.cpp
*
* Eric Schwabe
* 2010-04-24
*
* Wander state machine
*
*******************************************************************************/

#include "DXUT.h"
#include "SMWander.h"


// add new states
enum StateName 
{
	STATE_Initialize,           // note: first enum is the starting state
	STATE_Wander,
    STATE_AdjustDirection,
    STATE_Blocked
};

// add new substates
enum SubstateName 
{
    // none
};


/**
* Constructor
*/
SMWander::SMWander( GameObject* object, GameObjectCollision* coll ) :
    StateMachine( *object ),
    m_objColl( coll ),
    kFrontFeelerLength(3.0f),
    m_vFrontFeelerPos( 0.0f, 0.0f, 0.0f )
{}

/**
* Deconstructor
*/
SMWander::~SMWander()
{}

/**
* State machine
*/
bool SMWander::States( State_Machine_Event event, MSG_Object* msg, int state, int substate )
{
BeginStateMachine

	// global message responses go here
    // if attacked, switch to seek player state machine

    /*-------------------------------------------------------------------------*/

    DeclareState(STATE_Initialize)
    
        OnEnter

            m_owner->SetDirection( D3DXVECTOR3(12.5f, 0.0f, 12.5) - m_owner->GetPosition() );
            ChangeState(STATE_Wander);

    /*-------------------------------------------------------------------------*/
	
    DeclareState( STATE_Wander )

        DeclareStatePointerVector3(vLastPos)
		
        OnEnter

            // set velocity
            m_owner->SetVelocity(5.0f);

            // set initial last position
            vLastPos = new D3DXVECTOR3(0.0f, 0.0f, 0.0f);

        OnUpdate

            // check if moved since last update
            D3DXVECTOR3 vPosChange = m_owner->GetPosition() - (*vLastPos);
            if( D3DXVec3Length(&vPosChange) < 0.01f )
            {
                // if object stuck, change state
                ChangeState( STATE_Blocked );
            }

            // update last position
            (*vLastPos) = m_owner->GetPosition();

            // update object feelers
            UpdateFeelers();

            // check for future collisions
            CollOutput output;
            
            if( m_objColl->RunLineCollision(m_owner->GetPosition(), m_vFrontFeelerPos, &output) )
            {                
                // change object direction
                D3DXVECTOR3 vDir = m_owner->GetDirection();
                float fYawRotate = D3DXVec3Dot(&output.normal, &vDir) * (output.length / kFrontFeelerLength);
                m_owner->SetDirection( RotateVector(vDir, fYawRotate) );
            }
           
        OnTimeInState(2.0f)
        
            // adjust direction after a duration
            ChangeState( STATE_AdjustDirection );

        OnExit

            // reset velocity
            m_owner->SetVelocity(0.0f);

            delete vLastPos;

    /*-------------------------------------------------------------------------*/

    DeclareState( STATE_AdjustDirection )

        OnEnter

            // set velocity
            m_owner->SetVelocity(5.0f);

            // randomly adjust direction by a small amount
            float fYawRotate = D3DX_PI/3.0f * (1 - rand() % 3);
            m_owner->SetDirection( RotateVector(m_owner->GetDirection(), fYawRotate) );
            ChangeStateDelayed( 0.1f, STATE_Wander );
        
        OnExit
            
            // reset velocity
            m_owner->SetVelocity(0.0f);

    /*-------------------------------------------------------------------------*/

    DeclareState( STATE_Blocked )

    	OnEnter
            
            // change object direction
            D3DXVECTOR3 vDir = m_owner->GetDirection();
            m_owner->SetDirection( RotateVector(vDir, -D3DX_PI) );
            
            // change state back to move after a few seconds
            ChangeStateDelayed(1.0f, STATE_Wander);

    /*-------------------------------------------------------------------------*/


EndStateMachine
}

/**
* Recomputes feelers for object.
*/
void SMWander::UpdateFeelers()
{
    // get object position and direction
    D3DXVECTOR3 vDir = m_owner->GetDirection();
    D3DXVECTOR3 vPos = m_owner->GetPosition();
    vDir.y = vPos.y;
    
    // create front feeler
    D3DXVec3Scale(&vDir, &vDir, kFrontFeelerLength);
    m_vFrontFeelerPos = vPos + vDir;
    
    // re-scale direction
    //D3DXMATRIX mxRotate;
    //D3DXVec3Scale(&vDir, &vDir, 1.0f);

    // create left feeler            
    //m_vLeftFeeler = vPos + vDir;
    //D3DXMatrixRotationYawPitchRoll(&mxRotate, -D3DX_PI/4.0f, 0.0f, 0.0f);
    //m_vLeftFeeler = m_vLeftFeeler * mxRotate;

    // create right feeler
    //m_vRightFeeler = vPos + vDir;
    //D3DXMatrixRotationYawPitchRoll(&mxRotate, D3DX_PI/4.0f, 0.0f, 0.0f);
    //m_vRightFeeler = m_vRightFeeler * mxRotate;
}

/**
* Rotate direction
*/
D3DXVECTOR3 SMWander::RotateVector(const D3DXVECTOR3& vVec, const float& fYaw)
{
    D3DXMATRIX mxRotate;
    D3DXVECTOR4 vNewVec4;
    D3DXVECTOR3 vNewVec3;

    // create rotation
    D3DXMatrixRotationYawPitchRoll(&mxRotate, fYaw, 0.0f, 0.0f);
    
    // transform
    D3DXVec3Transform(&vNewVec4, &vVec, &mxRotate);

    vNewVec3.x = vNewVec4.x;
    vNewVec3.y = vNewVec4.y;
    vNewVec3.z = vNewVec4.z;

    return vNewVec3;
}