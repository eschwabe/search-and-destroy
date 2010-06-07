/*******************************************************************************
* Game Development Project
* GameController.h
*
* Eric Schwabe
* 2010-05-05
*
* Game Controller
*
*******************************************************************************/

#include "DXUT.h"
#include "GameController.h"

#include <sstream>
#include <iomanip>

#include "DXUT\DXUTsettingsdlg.h"
#include "DXUT\SDKmisc.h"
#include "global.h"
#include "statemch.h"
#include "WorldData.h"

/**
* Constructor
*/
GameController::GameController(objectID pid) :
    GameObject(g_database.GetNewObjectID(), OBJECT_GameControl, "GAMECONTROL"),
    m_pid(pid),
    m_PlayerObject(NULL),
    m_CurrentCamera(CAMERA_WORLDVIEW),
    m_pSoundManager(NULL),
    m_pSoundCollision(NULL),
    m_pDebugFont(NULL),
    m_pGameFontLarge(NULL),
    m_pGameFontSmall(NULL),
    m_pTextSprite(NULL),
    m_titleScreen(true),
    m_debugInfo(false)
{
    // setup the debug camera with view matrix
    D3DXVECTOR3 vEye(12.5f, 2.0f, 0.0f);
    D3DXVECTOR3 vAt(12.5f, 1.0f, 12.5f);

    // setup debug camera movement parameters
    m_DebugCamera.SetViewParams( &vEye, &vAt );
    m_DebugCamera.SetScalers( 0.01f, 5.0f );

    // setup world view camera
    m_WorldViewCamera.SetLookAt( D3DXVECTOR3(12.5f, 1.0f, 12.5f) );
    m_WorldViewCamera.SetDistance(25.0f);
    m_WorldViewCamera.SetPitchAngle(D3DX_PI/8.0f);
    m_WorldViewCamera.SetSpeed(D3DX_PI/25.0f);
}

/**
* Deconstructor
*/
GameController::~GameController(void)
{ 
    // cleanup fonts
    if(m_pDebugFont)
        m_pDebugFont->OnLostDevice();
    SAFE_RELEASE(m_pDebugFont);

    if(m_pGameFontLarge)
        m_pGameFontLarge->OnLostDevice();
    SAFE_RELEASE(m_pGameFontLarge);

    if(m_pGameFontSmall)
        m_pGameFontSmall->OnLostDevice();
    SAFE_RELEASE(m_pGameFontSmall);

    // cleanup text sprite
    SAFE_RELEASE(m_pTextSprite);

    // cleanup sound manager
    SAFE_DELETE(m_pSoundManager);
}

/**
* Configure cameras
*/
void GameController::ConfigureCameras(const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
    // setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
    m_DebugCamera.SetProjParams( D3DX_PI/3, fAspectRatio, 0.001f, 100.0f );
    m_PlayerCamera.SetProjParams( D3DX_PI/3, fAspectRatio, 0.001f, 100.0f );
}

/**
* Get the current camera
*/
CBaseCamera* GameController::GetCurrentCamera()
{
    switch(m_CurrentCamera)
    {
        case CAMERA_DEBUG:
            return &m_DebugCamera;
        case CAMERA_PLAYER:
            return &m_PlayerCamera;
        case CAMERA_WORLDVIEW:
        default:
            return &m_WorldViewCamera;
    }
}

/**
* Set the camera type
*/
void GameController::SetCameraType(const CameraType type)   
{
    // get current camera
    CBaseCamera* pCamera = GetCurrentCamera();

    // set new camera
    m_CurrentCamera = type;

    // copy camera eye position
    GetCurrentCamera()->SetEyeFromCamera(pCamera);
}

/**
* Get the current camera view matrix
*/
const D3DXMATRIX* GameController::GetCameraViewMatrix()
{
    return GetCurrentCamera()->GetViewMatrix();
}

/**
* Get the current camera projection matrix
*/
const D3DXMATRIX* GameController::GetCameraProjMatrix()
{
    return GetCurrentCamera()->GetProjMatrix();
}

/**
* Handle user input messages. Pass on to appropriate object based on game state
*/
LRESULT GameController::HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    // handle game menu actions
    switch( uMsg )
    {
        // key down message
        case WM_KEYDOWN:

            switch( (UINT)wParam )
            {
                case VK_RETURN:
                    g_database.SendMsgFromSystem(GetID(), MSG_GameStart); 
                    break;

                case 'L':
                    g_database.SendMsgFromSystem(GetID(), MSG_DebugMode); 
            }
    }

    // send messages on to player and/or camera
    switch(m_CurrentCamera)
    {
        case CAMERA_DEBUG:
            return GetCurrentCamera()->HandleMessages( hWnd, uMsg, wParam, lParam );
        case CAMERA_PLAYER:
            return m_PlayerObject->HandleMessages( hWnd, uMsg, wParam, lParam );
    }

    return TRUE;
}

/**
* Render the help and statistics text. This function uses the ID3DXFont interface for
* efficient text rendering.
*/
void GameController::RenderDebugText()
{
    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work however the
    // pFont->DrawText() will not be batched together.  Batching calls will improves performance.
    CDXUTTextHelper txtHelper( m_pDebugFont, m_pTextSprite, 15 );
    const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetD3D9BackBufferSurfaceDesc();

    // configure text
    txtHelper.Begin();
    txtHelper.SetInsertionPos( 5, 5 );
    std::wstringstream stream;

    // statistics
    stream << DXUTGetFrameStats() << std::endl;
    stream << DXUTGetDeviceStats() << std::endl << std::endl;
    stream << std::setprecision(4) << std::setw(4);
    stream << "Time: " << DXUTGetGlobalTimer()->GetTime() << std::endl;
    stream << "FPS:  " << DXUTGetFPS() << std::endl << std::endl;

    // camera details
    stream << "Eye:    (" << GetCurrentCamera()->GetEyePt()->x << ", " << GetCurrentCamera()->GetEyePt()->y << ", " << GetCurrentCamera()->GetEyePt()->z << ")" << std::endl;
    stream << "LookAt: (" << GetCurrentCamera()->GetLookAtPt()->x << ", " << GetCurrentCamera()->GetLookAtPt()->y << ", " << GetCurrentCamera()->GetLookAtPt()->z << ")" << std::endl;
    stream << "Player: (" << m_PlayerObject->GetPosition().x << ", " << m_PlayerObject->GetPosition().y << ", " << m_PlayerObject->GetPosition().z << ")" << std::endl;
    stream << std::endl;
    
    // terrain analysis
    stream << "TerrainAnalysis: " << g_world.GetTerrainAnalysisName() << std::endl;
    stream << std::endl;

    // state details
	dbCompositionList list;
	g_database.ComposeList( list, OBJECT_Ignore_Type );
	dbCompositionList::iterator i;
	for( i=list.begin(); i!=list.end(); ++i )
	{
		StateMachine* pStateMachine = (*i)->GetStateMachineManager()->GetStateMachine(STATE_MACHINE_QUEUE_0);
		if( pStateMachine )
		{
			char* name = (*i)->GetName();
			char* statename = pStateMachine->GetCurrentStateNameString();
			char* substatename = pStateMachine->GetCurrentSubstateNameString();
			TCHAR* unicode_name = new TCHAR[strlen(name)+1];
			TCHAR* unicode_statename = new TCHAR[strlen(statename)+1];
			TCHAR* unicode_substatename = new TCHAR[strlen(substatename)+1];
			mbstowcs(unicode_name, name, strlen(name)+1);
			mbstowcs(unicode_statename, statename, strlen(statename)+1);
			mbstowcs(unicode_substatename, substatename, strlen(substatename)+1);
			if( substatename[0] != 0 )
			{
                stream << unicode_name << ":   " << unicode_statename << unicode_substatename << std::endl;
			}
			else
			{
                stream << unicode_name << ":   " << unicode_statename << std::endl;
			}
			delete unicode_name;
			delete unicode_statename;
			delete unicode_substatename;
		}
	}

    // draw text shadow
    POINT pt = txtHelper.GetInsertPos();
	txtHelper.SetForegroundColor( D3DXCOLOR( 0.0f, 0.0f, 0.0f, 1.0f ) );
    txtHelper.SetInsertionPos( 4, pt.y );
    txtHelper.DrawTextLine( stream.str().c_str() );

    // reset and draw text
	txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
    txtHelper.SetInsertionPos( 5, pt.y );
    txtHelper.DrawTextLine( stream.str().c_str() );

    txtHelper.End();
}

/**
* Render the game menu text.
*/
void GameController::RenderGameTitleText()
{
    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work however the
    // pFont->DrawText() will not be batched together.  Batching calls will improves performance.
    CDXUTTextHelper txtHelper( m_pGameFontLarge, m_pTextSprite, 40 );
    const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetD3D9BackBufferSurfaceDesc();

    // determine font position 
    int row = 40;
    int col = 40;

    // configure text
    txtHelper.Begin();
    txtHelper.SetInsertionPos( col, row );
    std::wstringstream stream;

    stream << m_titleText << std::endl << std::endl << std::endl;

    // reset and draw text
	txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
    txtHelper.DrawTextLine( stream.str().c_str() );

    txtHelper.End();
}

/**
* Render the game menu text.
*/
void GameController::RenderGameHelpText()
{
    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work however the
    // pFont->DrawText() will not be batched together.  Batching calls will improves performance.
    CDXUTTextHelper txtHelper( m_pGameFontSmall, m_pTextSprite, 40 );
    const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetD3D9BackBufferSurfaceDesc();

    // determine font position based on window size
    RECT rWinSize = DXUTGetWindowClientRect();
    int row = 120;
    int col = 40;

    // configure text
    txtHelper.Begin();
    txtHelper.SetInsertionPos( col, row );
    std::wstringstream stream;

    stream << "Press ENTER to play" << std::endl;
    stream << "Press ESC to exit" << std::endl;

    // reset and draw text
	txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
    txtHelper.DrawTextLine( stream.str().c_str() );

    txtHelper.End();
}

/**
* Initialize
*/
HRESULT GameController::Initialize(IDirect3DDevice9* pd3dDevice)
{
    HRESULT hr = MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);

    // create a sprite to help batch calls when drawing many lines of text
    V_RETURN( D3DXCreateSprite( pd3dDevice, &m_pTextSprite ) );

    // create fonts
    V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 
                              DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Consolas", &m_pDebugFont ) );
    V_RETURN( D3DXCreateFont( pd3dDevice, 40, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 
                              DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &m_pGameFontLarge ) );
    V_RETURN( D3DXCreateFont( pd3dDevice, 24, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 
                              DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &m_pGameFontSmall ) );

    // initialize sound manager
    m_pSoundManager = new CSoundManager();
    m_pSoundManager->Initialize(DXUTGetHWND(), DSSCL_PRIORITY);
    m_pSoundManager->SetPrimaryBufferFormat(2, 22050, 16);

    // initialize collision sound
    WCHAR szSoundPath[MAX_PATH];
    DXUTFindDXSDKMediaFileCch( szSoundPath, MAX_PATH, L"alarm.wav" );
    m_pSoundManager->Create(&m_pSoundCollision, szSoundPath, 0, GUID_NULL);

    // setup player camera
    m_PlayerCamera.SetPlayer(m_pid);

    // find player object
    m_PlayerObject = g_database.Find(m_pid);

    return hr;
}

/**
* Update
*/
void GameController::Update()
{
    // update the camera's position
    GetCurrentCamera()->FrameMove(g_time.GetElapsedTime());

    // determine window size
    RECT rWinSize = DXUTGetWindowClientRect();
    float fWinHeight = (float)rWinSize.bottom;
    float fWinWidth = (float)rWinSize.right;
    
    // compute title screen background vertices
    D3DXCOLOR cScreenColor = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.5f);
    m_cvScreenVertices[0] = CustomVertex( D3DXVECTOR3(0.0f,         0.0f,       1.0f), cScreenColor);
    m_cvScreenVertices[1] = CustomVertex( D3DXVECTOR3(0.0f,         fWinHeight, 1.0f), cScreenColor);
    m_cvScreenVertices[2] = CustomVertex( D3DXVECTOR3(fWinWidth,    0.0f,       1.0f), cScreenColor);
    m_cvScreenVertices[3] = CustomVertex( D3DXVECTOR3(fWinWidth,    fWinHeight, 1.0f), cScreenColor);
}

/**
* Render
*/
void GameController::Render(IDirect3DDevice9* pd3dDevice, const RenderData* rData)
{
    // render debug text if enabled
    if(m_debugInfo)
    {
        RenderDebugText();
    }

    // render title screen if enabled
    if(m_titleScreen)
    {
        // disable z-buffering
        pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

        // enable alpha-blending
        pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

        // disable pixel shader
        pd3dDevice->SetPixelShader(NULL);

        // disable culling
        pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

        // Set vertex type
        pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

        // draw minimap primitive
        pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, m_cvScreenVertices, sizeof(m_cvScreenVertices[0]) );

        // render title screen text
        RenderGameTitleText();
        RenderGameHelpText();
    }
}

