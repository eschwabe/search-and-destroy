/*******************************************************************************
* Game Development Project
* RotationCamera.cpp
*
* Eric Schwabe
* 2010-6-6
*
* Rotation Camera: rotates around a point in the world at a specified rate
*
*******************************************************************************/

#include "DXUT.h"
#include "RotationCamera.h"

/**
* Constructor 
*/
CRotationCamera::CRotationCamera() :
    m_fDist(0.0f),
    m_fPitchAngle(0.0f),
    m_fVelocity(0.0f),
    m_fCurrentYawAngle(0.0f)
{
}

/**
* Deconstructor
*/
CRotationCamera::~CRotationCamera()
{
}

/**
* Set camera look at point
*/
void CRotationCamera::SetLookAt(const D3DXVECTOR3& vLookAt)
{
    m_vLookAt = vLookAt;
}

/**
* Set camera distance from point
*/
void CRotationCamera::SetDistance(float fDist)
{
    m_fDist = fDist;
}

/**
* Set camera angle from point
*/
void CRotationCamera::SetPitchAngle(float fAngle)
{
    m_fPitchAngle = fAngle;
}

/**
* Set camera circuling speed around point
*/
void CRotationCamera::SetSpeed(float fVel)
{
    m_fVelocity = fVel;
}

/**
* Move and update camera based on player movement.
*/
void CRotationCamera::FrameMove( FLOAT fElapsedTime )
{
    // update camera yaw angle
    m_fCurrentYawAngle += m_fVelocity*fElapsedTime;

    // create eye delta
    D3DXVECTOR3 vEyeDelta = D3DXVECTOR3(m_fDist, 0.0f, 0.0f);

    // rotate camera eye
    D3DXMATRIX mEyeRot;
    D3DXMatrixRotationYawPitchRoll( &mEyeRot, m_fCurrentYawAngle, 0.0f , m_fPitchAngle );
    D3DXVec3TransformCoord( &vEyeDelta, &vEyeDelta, &mEyeRot);

    // determine final eye position
    D3DXVECTOR3 vNewEye = m_vLookAt + vEyeDelta;

    // set eye to only a percentage of the distance between the current and new point
    m_vEye += (vNewEye - m_vEye)*0.15f;

    // update the view matrix
    D3DXVECTOR3 vWorldUp = D3DXVECTOR3(0,1,0);
    D3DXMatrixLookAtLH( &m_mView, &m_vEye, &m_vLookAt, &vWorldUp );

    // set camera world matrix
    D3DXMatrixInverse( &m_mCameraWorld, NULL, &m_mView );
}