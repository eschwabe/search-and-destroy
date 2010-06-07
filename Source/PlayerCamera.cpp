/*******************************************************************************
* Game Development Project
* PlayerCamera.cpp
*
* Eric Schwabe
* 2009-11-13
*
* Player Camera: Third person view
*
*******************************************************************************/

#include "DXUT.h"
#include "PlayerCamera.h"
#include "gameobject.h"

/**
* Constructor
*/
CPlayerCamera::CPlayerCamera() :
    m_objPlayer(NULL)
{
}

/**
* Deconstructor
*/
CPlayerCamera::~CPlayerCamera()
{}

/**
* Find player by object id for camera positioning
*/
void CPlayerCamera::SetPlayer(objectID id)
{
    m_objPlayer = g_database.Find(id);
}

/**
* Move and update camera based on player movement.
*
* @param fElapsedTime amount of time elapsed since last update
*/
void CPlayerCamera::FrameMove( FLOAT fElapsedTime )
{
    // default camera position info
    D3DXVECTOR3 vPlayerPos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    float fPlayerHeight = 1.0f;
    float fPlayerRotation = 0.0f;

    // get player position, height, and rotation
    if(m_objPlayer)
    {
        vPlayerPos = m_objPlayer->GetPosition();
        fPlayerHeight = m_objPlayer->GetHeight();
        fPlayerRotation = m_objPlayer->GetYawRotation();
    }

    // compute player head position (look at)
    m_vLookAt = vPlayerPos + D3DXVECTOR3(0.0f, fPlayerHeight, 0.0f);

    // compute camera position delta based on player rotation
    D3DXVECTOR3 vEyeDelta = D3DXVECTOR3(0.0f, fPlayerHeight, -fPlayerHeight*3);
    
    D3DXMATRIX mMoveRot;
    D3DXMatrixRotationYawPitchRoll( &mMoveRot, fPlayerRotation, 0, 0 );
    D3DXVec3TransformCoord( &vEyeDelta, &vEyeDelta, &mMoveRot);

    // determine final eye position
    D3DXVECTOR3 vNewEye = m_vLookAt + vEyeDelta;

    // move eye a percentage towards the final position
    m_vEye += (vNewEye - m_vEye)*0.15f;

    // update the view matrix
    D3DXVECTOR3 vWorldUp = D3DXVECTOR3(0,1,0);
    D3DXMatrixLookAtLH( &m_mView, &m_vEye, &m_vLookAt, &vWorldUp );

    // set camera world matrix
    D3DXMatrixInverse( &m_mCameraWorld, NULL, &m_mView );
}

