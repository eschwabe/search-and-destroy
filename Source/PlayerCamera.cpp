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

/**
* Constructor
*/
CPlayerCamera::CPlayerCamera() :
    m_pPlayerNode(NULL)
{
}

/**
* Deconstructor
*/
CPlayerCamera::~CPlayerCamera()
{}

/**
* Set player node for computing camera position
*
* @param pPlayerNode player node
*/
void CPlayerCamera::SetPlayerNode(const PlayerNode* pPlayerNode)
{
    m_pPlayerNode = pPlayerNode;
}

/**
* Move and update camera based on player movement.
*
* @param fElapsedTime amount of time elapsed since last update
*/
void CPlayerCamera::FrameMove( FLOAT fElapsedTime )
{
    // default player info
    D3DXVECTOR3 vPlayerPos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    float fPlayerHeight = 1.0f;
    float fPlayerRotation = 0.0f;
    
    // get player position, height, and rotation
    if(m_pPlayerNode)
    {
        vPlayerPos = m_pPlayerNode->GetPlayerPosition();
        fPlayerHeight = m_pPlayerNode->GetPlayerHeight();
        fPlayerRotation = m_pPlayerNode->GetPlayerRotation();
    }

    // compute player head position (look at)
    m_vLookAt = vPlayerPos + D3DXVECTOR3(0.0f, fPlayerHeight, 0.0f);

    // compute camera position delta based on player rotation
    D3DXVECTOR3 vEyeDelta = D3DXVECTOR3(0.0f, fPlayerHeight/2, -fPlayerHeight*2);
    
    D3DXMATRIX mMoveRot;
    D3DXMatrixRotationYawPitchRoll( &mMoveRot, fPlayerRotation, 0, 0 );
    D3DXVec3TransformCoord( &vEyeDelta, &vEyeDelta, &mMoveRot);

    // apply offset to player head position to get eye position
    m_vEye = m_vLookAt + vEyeDelta;

    // update the view matrix
    D3DXVECTOR3 vWorldUp = D3DXVECTOR3(0,1,0);
    D3DXMatrixLookAtLH( &m_mView, &m_vEye, &m_vLookAt, &vWorldUp );

    // set camera world matrix
    D3DXMatrixInverse( &m_mCameraWorld, NULL, &m_mView );
}

