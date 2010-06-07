/*******************************************************************************
* Game Development Project
* DebugCamera.h
*
* Eric Schwabe
* 2009-11-13
*
* Debug Camera: Watches for input and updates camera position
*
*******************************************************************************/

#pragma once
#include "DXUT\DXUTcamera.h"

class CDebugCamera : public CBaseCamera
{
    public:

        // Constructors
        CDebugCamera();
        virtual ~CDebugCamera();

        // Handle user input
        virtual LRESULT HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        // Move and update camera
        virtual void FrameMove( FLOAT fElapsedTime );

        // set view parameters
        void SetViewParams( D3DXVECTOR3* pvEyePt, D3DXVECTOR3* pvLookAtPt );
        
    private:

        // defines the supported camera movements
        enum Movement
        {
            kRotateLeft = 0,    
            kRotateRight,
            kMoveForward,
            kMoveBackward,
            kStrafeRight,
            kStrafeLeft,
            kPitchUp,
            kPitchDown,
            kMoveUp,
            kMoveDown,
            kMaxMovement,
            kUnknown = 0xFF
        };

        // METHODS
        Movement GetCameraMovement(const UINT&);

        // DATA
        int m_nActiveMouseButtonMask;           // mask to determine which mouse buttons to enable for rotation
        bool m_CameraMovement[kMaxMovement];    // camera movements currently requested
        
        D3DXVECTOR3 m_vNewEye;                  // new eye position

        // prevent copy and assignment
        CDebugCamera(const CDebugCamera&);
        void operator=(const CDebugCamera&);
};

