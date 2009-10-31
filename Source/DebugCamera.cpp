//------------------------------------------------------------------------------
// Project: Game Development (2009)
// 
// World Node
//------------------------------------------------------------------------------

#include "DXUT.h"
#include "DebugCamera.h"

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
CDebugCamera::CDebugCamera()
{
    for(int i =0; i < sizeof(m_CameraMovement); i++)
        m_CameraMovement[i] = false;
}

//------------------------------------------------------------------------------
// Deconstructor
//------------------------------------------------------------------------------
CDebugCamera::~CDebugCamera()
{}

//------------------------------------------------------------------------------
// Handle user input messages. 
//
// @param hWnd Handle to the window procedure to receive the message
// @param uMsg Specifies the message
// @param wParam Specifies additional message-specific information. The contents 
//               of this parameter depend on the value of the Msg parameter.
// @param lParam Specifies additional message-specific information. The contents 
//               of this parameter depend on the value of the Msg parameter.
//------------------------------------------------------------------------------
LRESULT CDebugCamera::HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER( hWnd );
    UNREFERENCED_PARAMETER( lParam );

    switch( uMsg )
    {
        // Key down message
        case WM_KEYDOWN:
        {
            Movement move = GetCameraMovement( (UINT)wParam );
            if( move != kUnknown )
            {
                m_CameraMovement[move] = true;  
            }
            break;
        }

        // Key up message
        case WM_KEYUP:
        {
            Movement move = GetCameraMovement( (UINT)wParam );
            if( move != kUnknown )
            {
                m_CameraMovement[move] = false;  
            }
            break; 
        }

        // Unsupported message
        default:
            return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
// Map keyboard key to camera movement
//
// @return camera movement 
//------------------------------------------------------------------------------
CDebugCamera::Movement CDebugCamera::GetCameraMovement(const UINT& key)
{
    D3DUtil_CameraKeys cam_key = CAM_UNKNOWN;

    switch(key)
    {
        case VK_LEFT:   return kRotateLeft;     // rotate camera left
        case VK_RIGHT:  return kRotateRight;    // rotate camera right
        case VK_UP:     return kMoveForward;    // move camera forward
        case VK_DOWN:   return kMoveBackward;   // move camera backward
        case VK_PRIOR:  return kPitchUp;        // tilt camera up
        case VK_NEXT:   return kPitchDown;      // tilt camera down
        case 'A':       return kMoveUp;         // raise camera (+y)
        case 'Z':       return kMoveDown;       // lower camera (-y)
        default:        return kUnknown;        // unsupported
    }
}

//------------------------------------------------------------------------------
// Move and update camera
//
// @param fElapsedTime amount of time elapsed since last update
//------------------------------------------------------------------------------
void CDebugCamera::FrameMove( FLOAT fElapsedTime )
{
    // override elapsed time if timer stopped
    if( DXUTGetGlobalTimer()->IsStopped() )
        fElapsedTime = 1.0f / DXUTGetFPS();

    // reset keyboard direction
    m_vKeyboardDirection = D3DXVECTOR3(0,0,0);

    // update acceleration vector based on keyboard state
    if( m_CameraMovement[kMoveForward] )
        m_vKeyboardDirection.z += 1.0f;
    if( m_CameraMovement[kMoveBackward] )
        m_vKeyboardDirection.z -= 1.0f;
    if( m_CameraMovement[kMoveUp] )
        m_vKeyboardDirection.y += 1.0f;
    if( m_CameraMovement[kMoveDown] )
        m_vKeyboardDirection.y -= 1.0f;

    // Get amount of velocity based on the keyboard input and drag (if any)
    UpdateVelocity( fElapsedTime );

    // Simple euler method to calculate position delta
    D3DXVECTOR3 vPosDelta = m_vVelocity * fElapsedTime;

    // Update the pitch and yaw angles
    float fYawDelta   = 0.0;
    float fPitchDelta = 0.0;

    if( m_CameraMovement[kRotateLeft] )
        fYawDelta -= 0.01;
    if( m_CameraMovement[kRotateRight] )
        fYawDelta += 0.01;
    if( m_CameraMovement[kPitchUp] )
        fPitchDelta -= 0.01;
    if( m_CameraMovement[kPitchDown] )
        fPitchDelta += 0.01;

    m_fCameraPitchAngle += fPitchDelta;
    m_fCameraYawAngle   += fYawDelta;

    // limit pitch to straight up or straight down
    m_fCameraPitchAngle = __max( -D3DX_PI/2.0f,  m_fCameraPitchAngle );
    m_fCameraPitchAngle = __min( +D3DX_PI/2.0f,  m_fCameraPitchAngle );

    // Make a rotation matrix based on the camera's yaw & pitch
    D3DXMATRIX mCameraRot;
    D3DXMatrixRotationYawPitchRoll( &mCameraRot, m_fCameraYawAngle, m_fCameraPitchAngle, 0 );

    // Transform vectors based on camera's rotation matrix
    D3DXVECTOR3 vWorldUp;
    D3DXVECTOR3 vWorldAhead;
    D3DXVECTOR3 vLocalUp    = D3DXVECTOR3(0,1,0);
    D3DXVECTOR3 vLocalAhead = D3DXVECTOR3(0,0,1);

    D3DXVec3TransformCoord( &vWorldUp, &vLocalUp, &mCameraRot );
    D3DXVec3TransformCoord( &vWorldAhead, &vLocalAhead, &mCameraRot );

    // Transform the position delta by the camera's rotation 
    D3DXVECTOR3 vPosDeltaWorld;
    D3DXVec3TransformCoord( &vPosDeltaWorld, &vPosDelta, &mCameraRot );

    // Move the eye position 
    m_vEye += vPosDeltaWorld;
    if( m_bClipToBoundary )
        ConstrainToBoundary( &m_vEye );

    // Update lookAt position based on the eye position 
    m_vLookAt = m_vEye + vWorldAhead;

    // update the view matrix
    D3DXMatrixLookAtLH( &m_mView, &m_vEye, &m_vLookAt, &vWorldUp );

    // set camera world matrix
    D3DXMatrixInverse( &m_mCameraWorld, NULL, &m_mView );
}

//------------------------------------------------------------------------------
// Modify camera behavior
//------------------------------------------------------------------------------
//void CDebugCamera::SetRotateButtons( bool bLeft, bool bMiddle, bool bRight, bool bRotateWithoutButtonDown = false)
//{}
