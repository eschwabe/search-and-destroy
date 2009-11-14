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

#include "DXUT.h"
#include "DebugCamera.h"

/**
* Constructor
*/
CDebugCamera::CDebugCamera()
{
    // initialize camera movements
    for(int i =0; i < sizeof(m_CameraMovement); i++)
        m_CameraMovement[i] = false;

    // initialize mouse button mask
    m_nActiveMouseButtonMask = MOUSE_LEFT_BUTTON | MOUSE_MIDDLE_BUTTON | MOUSE_RIGHT_BUTTON;
}

/**
* Deconstructor
*/
CDebugCamera::~CDebugCamera()
{}

/**
* Handle user input messages. 
*
* @param hWnd Handle to the window procedure to receive the message
* @param uMsg Specifies the message type (i.e. key down or key up)
* @param wParam Specifies additional message-specific information. The contents 
*               of this parameter depend on the value of the Msg parameter.
*               In this context, the param is the associated keyboard key with the event.
* @param lParam Specifies additional message-specific information. The contents 
*               of this parameter depend on the value of the Msg parameter.
*/
LRESULT CDebugCamera::HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // notify base camera also (mouse movement)
    CBaseCamera::HandleMessages(hWnd, uMsg, wParam, lParam);

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
    }

    return TRUE;
}

/**
* Map keyboard key to camera movement
*
* @return camera movement 
*/
CDebugCamera::Movement CDebugCamera::GetCameraMovement(const UINT& key)
{
    switch(key)
    {
        case VK_LEFT:   return kRotateLeft;     // rotate camera left
        case VK_RIGHT:  return kRotateRight;    // rotate camera right
        case VK_UP:     return kMoveForward;    // move camera forward
        case VK_DOWN:   return kMoveBackward;   // move camera backward

        case 'A':       return kStrafeLeft;     // rotate camera left
        case 'D':       return kStrafeRight;    // rotate camera right
        case 'W':       return kMoveForward;    // move camera forward
        case 'S':       return kMoveBackward;   // move camera backward

        case VK_PRIOR:  return kPitchUp;        // tilt camera up
        case VK_NEXT:   return kPitchDown;      // tilt camera down
        case 'Z':       return kMoveUp;         // raise camera (+y)
        case 'X':       return kMoveDown;       // lower camera (-y)

        case VK_SPACE:  return kMoveUp;         // raise camera (+y)

        default:        return kUnknown;        // unsupported
    }
}

/**
* Move and update camera
*
* @param fElapsedTime amount of time elapsed since last update
*/
void CDebugCamera::FrameMove( FLOAT fElapsedTime )
{
    // override elapsed time if timer stopped
    if( DXUTGetGlobalTimer()->IsStopped() )
        fElapsedTime = 1.0f / DXUTGetFPS();

    // get mouse input if active and update camera angles
    if( m_nActiveMouseButtonMask & m_nCurrentButtonMask )
    {
        UpdateMouseDelta();

        // note: mouse rotation velocity 2D vector
        m_fCameraPitchAngle += m_vRotVelocity.y;
        m_fCameraYawAngle   += m_vRotVelocity.x;
    }

    // update yaw and pitch angles from keyboard input
    float fYawDelta = 0.0f;
    float fPitchDelta = 0.0f;

    if( m_CameraMovement[kRotateLeft] )
        fYawDelta -= 0.03f;
    if( m_CameraMovement[kRotateRight] )
        fYawDelta += 0.03f;
    if( m_CameraMovement[kPitchUp] )
        fPitchDelta -= 0.03f;
    if( m_CameraMovement[kPitchDown] )
        fPitchDelta += 0.03f;

    m_fCameraPitchAngle += fPitchDelta;
    m_fCameraYawAngle   += fYawDelta;

    // limit pitch to straight up or straight down
    // note: the normal to the x-z plane causes everything to disappear
    m_fCameraPitchAngle = __max( (float)-D3DX_PI/2.0f+.01f,  m_fCameraPitchAngle );
    m_fCameraPitchAngle = __min( (float)+D3DX_PI/2.0f-.01f,  m_fCameraPitchAngle );

    // reset position delta
    D3DXVECTOR3 vPosDelta = D3DXVECTOR3(0,0,0);

    // create movement rotation matrix
    D3DXMATRIX mMoveRot;
    D3DXMatrixRotationYawPitchRoll( &mMoveRot, m_fCameraYawAngle, m_fCameraPitchAngle, 0 );

    // update x-z plane movement
    if( m_CameraMovement[kMoveForward] ) 
        vPosDelta.z += 1.5f;
    if( m_CameraMovement[kMoveBackward] )
        vPosDelta.z += -1.5f;
    if( m_CameraMovement[kStrafeRight] ) 
        vPosDelta.x += 1.5f;
    if( m_CameraMovement[kStrafeLeft] )
        vPosDelta.x += -1.5f;

    // rotate position delta
    D3DXVec3TransformCoord( &vPosDelta, &vPosDelta, &mMoveRot );

    // update y axis movement (independant of other motions)
    if( m_CameraMovement[kMoveUp] )
        vPosDelta.y += 1.5f;
    if( m_CameraMovement[kMoveDown] )
        vPosDelta.y -= 1.5f;

    // scale position change by elapsed time and movement scalar
    vPosDelta = vPosDelta * fElapsedTime * m_fMoveScaler;

    // create rotation matrix based on yaw and pitch
    D3DXMATRIX mLookAtRot;
    D3DXMatrixRotationYawPitchRoll( &mLookAtRot, m_fCameraYawAngle, m_fCameraPitchAngle, 0 );

    // transform vectors based on camera rotation matrix
    D3DXVECTOR3 vWorldUp;
    D3DXVECTOR3 vWorldAhead;
    D3DXVECTOR3 vLocalUp    = D3DXVECTOR3(0,1,0);
    D3DXVECTOR3 vLocalAhead = D3DXVECTOR3(0,0,1);

    // transform local up to world up (equal due to no roll rotation)
    vWorldUp = vLocalUp;
    //D3DXVec3TransformCoord( &vWorldUp, &vLocalUp, &mLookAtRot );

    // transform look ahead direction via yaw and pitch
    D3DXVec3TransformCoord( &vWorldAhead, &vLocalAhead, &mLookAtRot );

    // move eye position: the eye does not need to transformed by the rotation matrix
    // since the yaw and pitch rotations occur independently of the current camera position
    m_vEye += vPosDelta;

    // update lookAt position based on the eye position 
    m_vLookAt = m_vEye + vWorldAhead;

    // update the view matrix
    D3DXMatrixLookAtLH( &m_mView, &m_vEye, &m_vLookAt, &vWorldUp );

    // set camera world matrix
    D3DXMatrixInverse( &m_mCameraWorld, NULL, &m_mView );
}

