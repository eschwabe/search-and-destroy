/*******************************************************************************
* Game Development Project
* RotationCamera.h
*
* Eric Schwabe
* 2010-6-6
*
* Rotation Camera: rotates around a point in the world at a specified rate
*
*******************************************************************************/

#pragma once
#include "DXUT\DXUTcamera.h"

class CRotationCamera : public CBaseCamera
{
    public:

        CRotationCamera();
        virtual ~CRotationCamera();

        // move and update camera
        virtual void FrameMove( float fElapsedTime );

        void SetLookAt(const D3DXVECTOR3& vLookAt);
        void SetDistance(float fDist);
        void SetPitchAngle(float fAngle);
        void SetSpeed(float fVel);

    private:

        // prevent copy and assignment
        CRotationCamera(const CRotationCamera&);
        void operator=(const CRotationCamera&);

        float m_fDist;          // camera distance from lookat point
        float m_fPitchAngle;    // camera pitch angle towards lookat point
        float m_fVelocity;      // camera circling velocity around point

        float m_fCurrentYawAngle;
};
