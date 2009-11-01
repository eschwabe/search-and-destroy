//------------------------------------------------------------------------------
// Project: Game Development (2009)
// 
// Debug Camera
// Handles keyboard input and camera movement
//------------------------------------------------------------------------------

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

        // Modify camera behavior
        //void SetRotateButtons( bool bLeft, bool bMiddle, bool bRight, bool bRotateWithoutButtonDown = false );

        // Get camera state
        const D3DXMATRIX* GetWorldMatrix()          { return &m_mCameraWorld; }
        const D3DXVECTOR3* GetWorldRight() const    { return (D3DXVECTOR3*)&m_mCameraWorld._11; }
        const D3DXVECTOR3* GetWorldUp() const       { return (D3DXVECTOR3*)&m_mCameraWorld._21; }
        const D3DXVECTOR3* GetWorldAhead() const    { return (D3DXVECTOR3*)&m_mCameraWorld._31; }
        
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

        // methods
        Movement GetCameraMovement(const UINT&);

        bool m_CameraMovement[kMaxMovement];    // camera movements currently requested

        D3DXMATRIX m_mCameraWorld;              // world matrix of the camera (inverse of the view matrix)  
};
