/*******************************************************************************
* Game Development Project
* PlayerCamera.h
*
* Eric Schwabe
* 2009-11-13
*
* Player Camera: Third person view
*
*******************************************************************************/

#pragma once
#include "DXUT\DXUTcamera.h"
#include "database.h"

class CPlayerCamera : public CBaseCamera
{
    public:

        // constructors
        CPlayerCamera();
        virtual ~CPlayerCamera();

        // set player for camera
        void SetPlayer(objectID id);

        // move and update camera
        virtual void FrameMove( float fElapsedTime );

        // get camera state
        const D3DXMATRIX* GetWorldMatrix()          { return &m_mCameraWorld; }
        const D3DXVECTOR3* GetWorldRight() const    { return (D3DXVECTOR3*)&m_mCameraWorld._11; }
        const D3DXVECTOR3* GetWorldUp() const       { return (D3DXVECTOR3*)&m_mCameraWorld._21; }
        const D3DXVECTOR3* GetWorldAhead() const    { return (D3DXVECTOR3*)&m_mCameraWorld._31; }
        
    private:

        // DATA
        D3DXMATRIX m_mCameraWorld;              // world matrix of the camera (inverse of the view matrix)  
        const GameObject* m_objPlayer;          // player database object

        // prevent copy and assignment
        CPlayerCamera(const CPlayerCamera&);
        void operator=(const CPlayerCamera&);
};

