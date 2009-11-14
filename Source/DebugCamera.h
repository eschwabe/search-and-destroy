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
        
    private:

        // defines the supported camera movements
        enum Movement
        {
            kRotateLeft = 0,    
            kRotateRight,
            kMoveForward,
            kMoveBackward,
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
        bool m_CameraMovement[kMaxMovement];    // camera movements currently requested

        // prevent copy and assignment
        CDebugCamera(const CDebugCamera&);
        void operator=(const CDebugCamera&);
};

