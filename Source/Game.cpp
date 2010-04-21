//--------------------------------------------------------------------------------------
// Based upon MultiAnimation.cpp from the DirectX demos. 
//
// Starting point for new Direct3D applications
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "DXUT.h"
#pragma warning(disable: 4995)

#include "resource.h"
#include "DXUT\DXUTcamera.h"
#include "DXUT\DXUTsettingsdlg.h"
#include "DXUT\SDKmisc.h"
#include "DXUT\SDKsound.h"
#include "DebugCamera.h"
#include "PlayerCamera.h"
#include "Game.h"
#include "PlayerTinyNode.h"
#include "NPCSphereNode.h"
#include "WorldNode.h"
#include "WorldFile.h"
#include "MiniMapNode.h"
#include "RenderData.h"
#include "ParticleEmitter.h"
#include "database.h"
#include "msgroute.h"
#include "debuglog.h"
#include "time.h"

using namespace std;

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
ID3DXFont*              g_pFont = NULL;             // font for drawing text
ID3DXSprite*            g_pTextSprite = NULL;       // sprite for batching draw text calls
CDebugCamera            g_DebugCamera;              // debug camera
CPlayerCamera           g_PlayerCamera;             // player camera
CBaseCamera*            g_Camera = &g_PlayerCamera; // current camera (default player)
bool                    g_PlayerMode = true;        // controlling main player
CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg         g_SettingsDlg;              // device settings dialog
CDXUTDialog             g_HUD;                      // dialog for standard controls
CDXUTDialog             g_SampleUI;                 // dialog for sample specific controls
bool                    g_bShowHelp = true;         // render the UI control text if true
bool                    g_bPlaySounds = true;       // play sounds if true
Time*                   g_pTime;                    // time manager
Database*               g_pDatabase;                // game object database
MsgRoute*               g_pMsgRoute;                // message router
DebugLog*               g_pDebugLog;                // debug logger
PlayerTinyNode*         g_pMainPlayerNode;          // main player
WorldFile               g_GridData;                 // drid data loaded from file
VecCollQuad             g_vQuadList;                // collision quad list
CSoundManager*          g_pSoundManager = NULL;     // sound manager
CSound*                 g_pSoundCollision = NULL;   // collision sound
RenderData*             g_pRenderData = NULL;       // render data
ParticleEmitter*        g_pEmitter = NULL;          // particle emitter
bool                    g_bEnableFountain = false;  // enable particle fountain

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           3
#define IDC_CHANGEDEVICE        4
#define IDC_NEXTVIEW            6
#define IDC_PREVVIEW            7
#define IDC_RESETCAMERA         11
#define IDC_RESETTIME           12
#define IDC_TOGGLEFOUNTAIN      13

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------

void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );

HRESULT LoadMesh( IDirect3DDevice9* pd3dDevice, WCHAR* strFileName, ID3DXMesh** ppMesh );
void RenderText();

//--------------------------------------------------------------------------------------
// Initialize Application
//--------------------------------------------------------------------------------------
bool InitApp()
{
    // Initialize dialogs
    g_SettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );

    // Setup HUD
    int iY = 0;
    g_HUD.SetCallback( OnGUIEvent );
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN,  L"Toggle Full Screen",  35, iY += 10, 125, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF,         L"Toggle REF (F3)",     35, iY += 24, 125, 22, VK_F3 );
    g_HUD.AddButton( IDC_CHANGEDEVICE,      L"Change Device (F2)",  35, iY += 24, 125, 22, VK_F2 );

    g_HUD.SetCallback( OnGUIEvent );
    g_HUD.AddButton( IDC_NEXTVIEW,          L"Next View (N)",       35, iY += 72, 125, 22, L'N' );
    g_HUD.AddButton( IDC_PREVVIEW,          L"Previous View (P)",   35, iY += 24, 125, 22, L'P' );
    g_HUD.AddButton( IDC_RESETCAMERA,       L"Reset View (R)",      35, iY += 24, 125, 22, L'R' );
    g_HUD.AddButton( IDC_RESETTIME,         L"Reset Time",          35, iY += 24, 125, 22 );
    g_HUD.AddButton( IDC_TOGGLEFOUNTAIN,    L"Enable Fountain",     35, iY += 48, 125, 22 );

    // Add mixed vp to the available vp choices in device settings dialog.
    DXUTGetD3D9Enumeration()->SetPossibleVertexProcessingList( true, false, false, true );
    
    // setup the camera with view matrix
    D3DXVECTOR3 vEye(-5.0f, 2.0f, -5.0f);
    D3DXVECTOR3 vAt(0.0f, 0.0f, 0.0f);

    // setup debug camera movement parameters
    g_DebugCamera.SetViewParams( &vEye, &vAt );
    g_DebugCamera.SetScalers( 0.01f, 5.0f );

	return true;
}

//--------------------------------------------------------------------------------------
// Clean up the app
//--------------------------------------------------------------------------------------
void CleanupApp()
{
	// Do any sort of app cleanup here 
}

//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat,
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    // Skip backbuffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3D9Object();
    if( FAILED(pD3D->CheckDeviceFormat(
        pCaps->AdapterOrdinal, 
        pCaps->DeviceType,
        AdapterFormat, 
        D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
        D3DRTYPE_TEXTURE, 
        BackBufferFormat)) )
    {
        return false;
    }

    // Need to support ps 1.1
    if( pCaps->PixelShaderVersion < D3DPS_VERSION( 1, 1 ) )
        return false;

    // Need to support A8R8G8B8 textures
    if( FAILED(pD3D->CheckDeviceFormat( 
        pCaps->AdapterOrdinal, 
        pCaps->DeviceType,
        AdapterFormat, 
        0,
        D3DRTYPE_TEXTURE, 
        D3DFMT_A8R8G8B8)) )
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------
// This callback function is called immediately before a device is created to allow the
// application to modify the device settings. The supplied pDeviceSettings parameter
// contains the settings that the framework has selected for the new device, and the
// application can make any desired changes directly to this structure.  Note however that
// DXUT will not correct invalid device settings so care must be taken
// to return valid device settings, otherwise IDirect3D9::CreateDevice() will fail.
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext )
{
    // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW
    // then switch to SWVP.
    if( (pCaps->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
         pCaps->VertexShaderVersion < D3DVS_VERSION(1,1) )
    {
        pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }

     // If the hardware cannot do vertex blending, use software vertex processing.
    if( pCaps->MaxVertexBlendMatrices < 2 )
        pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

    // If using hardware vertex processing, change to mixed vertex processing
    // so there is a fallback.
    if( pDeviceSettings->d3d9.BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING )
        pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_MIXED_VERTEXPROCESSING;

    // Debugging vertex shaders requires either REF or software vertex processing
    // and debugging pixel shaders requires REF.
#ifdef DEBUG_VS
    if( pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF )
    {
        pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
        pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
        pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
#endif
#ifdef DEBUG_PS
    pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
#endif

    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF )
            DXUTDisplaySwitchingToREFWarning(  pDeviceSettings->ver  );
    }

    return true;
}

//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been
// created, which will happen during application initialization and windowed/full screen
// toggles. This is the best location to create D3DPOOL_MANAGED resources since these
// resources need to be reloaded whenever the device is destroyed. Resources created
// here should be released in the OnDestroyDevice callback.
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );
    V_RETURN( g_SettingsDlg.OnD3D9CreateDevice( pd3dDevice ) );
    
    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);
}

//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been
// reset, which will happen after a lost device scenario. This is the best location to
// create D3DPOOL_DEFAULT resources since these resources need to be reloaded whenever
// the device is lost. Resources created here should be released in the OnLostDevice
// callback.
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice,
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    // reset dialogs
    V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );
    V_RETURN( g_SettingsDlg.OnD3D9ResetDevice() );

    // initialize singleton objects
	g_pTime = new Time();
	g_pDatabase = new Database();
	g_pMsgRoute = new MsgRoute();
	g_pDebugLog = new DebugLog();

	// create a sprite to help batch calls when drawing many lines of text
    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pTextSprite ) );

    // initialize the font
    if( g_pFont )
        V_RETURN( g_pFont->OnResetDevice() );

    V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 
                              DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Consolas", &g_pFont ) );

    // initialize sound manager
    g_pSoundManager = new CSoundManager();
    g_pSoundManager->Initialize(DXUTGetHWND(), DSSCL_PRIORITY);
    g_pSoundManager->SetPrimaryBufferFormat(2, 22050, 16);

    // initialize collision sound
    WCHAR szSoundPath[MAX_PATH];
    DXUTFindDXSDKMediaFileCch( szSoundPath, MAX_PATH, L"alarm.wav" );
    g_pSoundManager->Create(&g_pSoundCollision, szSoundPath, 0, GUID_NULL);

    // add world object
    WorldNode* p_WorldNode = new WorldNode(L"level-collision.grd", L"asphalt-damaged.jpg", L"planks-new.jpg");
    g_database.Store(p_WorldNode);

    // add player node
    g_pMainPlayerNode = new PlayerTinyNode( L"tiny.x", D3DXVECTOR3(13.0f,0.0f,1.0f) );
    g_database.Store(g_pMainPlayerNode);
    g_PlayerCamera.SetPlayer(g_pMainPlayerNode->GetID());

    // add sphere NPCs
    for( DWORD i = 0; i < 8; i++ )
    {
        g_database.Store( new NPCSphereNode(D3DXVECTOR3(12.5f,0.0f,16.0f)) );
    }

    // add particle emitter and fire particles
    g_pEmitter = new ParticleEmitter(L"particle-point.png");
    g_pEmitter->EnableParticles(ParticleEmitter::kFire, D3DXVECTOR3(3.0f, 0.2f, 3.0f));
    g_pEmitter->EnableParticles(ParticleEmitter::kFire, D3DXVECTOR3(3.0f, 0.2f, 22.0f));
    g_pEmitter->EnableParticles(ParticleEmitter::kFire, D3DXVECTOR3(22.0f, 0.2f, 22.0f));
    g_pEmitter->EnableParticles(ParticleEmitter::kFire, D3DXVECTOR3(22.0f, 0.2f, 3.0f));
    g_database.Store(g_pEmitter);

    // add minimap node (note: draw 2D elements after rendering 3D)
    MiniMapNode* p_MiniMap = new MiniMapNode(
        L"level-collision-map.png", 
        L"level-collision-map-alpha.png", 
        L"minimap-border.png", 
        L"minimap-player.png", 
        L"minimap-npc.png", 
        200);

    p_MiniMap->SetWorldNode(p_WorldNode);
    p_MiniMap->AddPlayerTracking(g_pMainPlayerNode, MiniMapNode::PLAYER);
    //p_MiniMap->AddPlayerTracking(g_pNPCNode, MiniMapNode::NPC);
    g_database.Store(p_MiniMap);

    // initialize database objects
    if( FAILED(g_database.InitializeObjects(pd3dDevice)) )
    {
        MessageBox( NULL, L"Could not load nodes", L"UWGame", MB_OK );
        return E_FAIL;
    }

    // initialize render data
    g_pRenderData = new RenderData();
    if( FAILED(g_pRenderData->Initialize(pd3dDevice)) )
        return E_FAIL;

    // setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
    g_DebugCamera.SetProjParams( D3DX_PI/3, fAspectRatio, 0.001f, 100.0f );
    g_PlayerCamera.SetProjParams( D3DX_PI/3, fAspectRatio, 0.001f, 100.0f );

    // configure lights
    g_pRenderData->SetDirectionalLight( D3DXVECTOR4(-0.5f, 0.1f, -0.25f, 1.0f), D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f) );
    g_pRenderData->SetAmbientLight( D3DXCOLOR(0.3f, 0.3f, 0.3f, 0.3f) );

    // adjust the dialog parameters
    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width-170, 0 );
    g_HUD.SetSize( 170, 170 );
    g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width-170, pBackBufferSurfaceDesc->Height-270 );
    g_SampleUI.SetSize( 170, 220 );

    // get collision quad list
    g_vQuadList = p_WorldNode->GetCollisionQuadList();

    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);
}

//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not
// intended to contain actual rendering calls, which should instead be placed in the
// OnFrameRender callback.
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    // update time
	g_time.MarkTimeThisTick();

    // update database objects
	g_database.UpdateObjects();

    // update the camera's position based on user input
    g_Camera->FrameMove(fElapsedTime);

    // generate list of players and NPCs
    dbCompositionList pList;
    g_database.ComposeList( pList, OBJECT_NPC | OBJECT_Player );

    // check for player collisions with environment
    CollObject coll;
    for(dbCompositionList::iterator it = pList.begin(); it != pList.end(); ++it)
    {
        coll.RunWorldCollision(g_vQuadList, (*it));
    }

    // check for collisions between players
    //static bool bPlayerCollision = false;

    //if( coll.RunPlayerCollision(g_pMainPlayerNode, g_pNPCNode) )
    //{
    //    if( !bPlayerCollision )
    //    {
    //        // play sound
    //        //g_pSoundCollision->Play(0, 0);
    //        bPlayerCollision = true;
    //    }
    //}
    //else
    //{
    //    bPlayerCollision = false;
    //}
}

//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the
// rendering calls for the scene, and it will also be called if the window needs to be
// repainted. After this function has returned, DXUT will call
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    // If the settings dialog is being shown, then
    // render it instead of rendering the app's scene
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.OnRender( fElapsedTime );
        return;
    }

    // clear screen
    pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                       D3DCOLOR_ARGB( 0, 0x44, 0xAA, 0xDD ), 1.0f, 0L );

    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
        HRESULT hr;

        // set static transforms
        V( pd3dDevice->SetTransform( D3DTS_VIEW, g_Camera->GetViewMatrix()) );
        V( pd3dDevice->SetTransform( D3DTS_PROJECTION, g_Camera->GetProjMatrix()) );

        // set rendering view and projection matrix
        g_pRenderData->matProjection = *g_Camera->GetProjMatrix();
        g_pRenderData->matView = *g_Camera->GetViewMatrix();

        // set world matrix
        D3DXMATRIX matIdentity;
        D3DXMatrixIdentity( &matIdentity );
        g_pRenderData->matWorld = matIdentity;

        // render all nodes
        g_database.RenderObjects(pd3dDevice, g_pRenderData);

        // display text on hud
        RenderText();

        V( g_HUD.OnRender( fElapsedTime ) );

        pd3dDevice->EndScene();
    }
}

//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work however the
    // pFont->DrawText() will not be batched together.  Batching calls will improves performance.
    CDXUTTextHelper txtHelper( g_pFont, g_pTextSprite, 15 );
    const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetD3D9BackBufferSurfaceDesc();

    // configure text
    txtHelper.Begin();
    txtHelper.SetInsertionPos( 5, 5 );
    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );

    // frame and device stats
    //txtHelper.DrawTextLine( DXUTGetFrameStats() );
    //txtHelper.DrawTextLine( DXUTGetDeviceStats() );

    // statistics
    txtHelper.DrawFormattedTextLine( L"%-8s %.2f", L"Time:", DXUTGetGlobalTimer()->GetTime() );
    txtHelper.DrawFormattedTextLine( L"%-8s %.2f", L"FPS:", DXUTGetFPS() );

    // camera details
    txtHelper.SetInsertionPos( 5, 40 );
    txtHelper.DrawFormattedTextLine(L"%-8s (%.2f, %.2f, %.2f)", 
        L"Eye:", g_Camera->GetEyePt()->x, g_Camera->GetEyePt()->y, g_Camera->GetEyePt()->z);
    txtHelper.DrawFormattedTextLine(L"%-8s (%.2f, %.2f, %.2f)", 
        L"LookAt:", g_Camera->GetLookAtPt()->x, g_Camera->GetLookAtPt()->y, g_Camera->GetLookAtPt()->z);
    txtHelper.DrawFormattedTextLine(L"%-8s (%.2f, %.2f, %.2f)", 
        L"Player:", g_pMainPlayerNode->GetPosition().x, g_pMainPlayerNode->GetPosition().y, g_pMainPlayerNode->GetPosition().z);
    
    txtHelper.End();
}

//--------------------------------------------------------------------------------------
// Before handling window messages, DXUT passes incoming windows
// messages to the application through this callback function. If the application sets
// *pbNoFurtherProcessing to TRUE, then DXUT will not process this message.
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
    // Always allow dialog resource manager calls to handle global messages
    // so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // pass messages to camera or player depending on mode
    if(g_PlayerMode)
        g_pMainPlayerNode->HandleMessages( hWnd, uMsg, wParam, lParam );
    else
        g_Camera->HandleMessages( hWnd, uMsg, wParam, lParam );
    
    return 0;
}

//--------------------------------------------------------------------------------------
// As a convenience, DXUT inspects the incoming windows messages for
// keystroke messages and decodes the message parameters to pass relevant keyboard
// messages to the application.  The framework does not remove the underlying keystroke
// messages, which are still passed to the application's MsgProc callback.
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if( bKeyDown )
    {
        switch( nChar )
        {
            case VK_F1:
				g_bShowHelp = !g_bShowHelp;
				break;
        }
    }
}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN: 
            DXUTToggleFullScreen(); 
            break;

        case IDC_TOGGLEREF:        
            DXUTToggleREF(); 
            break;

        case IDC_CHANGEDEVICE:     
            g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() ); 
            break;

        case IDC_NEXTVIEW:
        case IDC_PREVVIEW:
            // toggle between player control mode and debug camera mode
            if(g_PlayerMode)
            {
                g_Camera = &g_DebugCamera;
                g_PlayerMode = false;
            }
            else
            {
                g_Camera = &g_PlayerCamera;
                g_PlayerMode = true;
            }
            break;

        case IDC_RESETCAMERA:
            g_Camera = &g_PlayerCamera;
            g_PlayerMode = true;
            break;

        case IDC_RESETTIME:
            DXUTGetGlobalTimer()->Reset();
            break;

        case IDC_TOGGLEFOUNTAIN:
            g_bEnableFountain = !g_bEnableFountain;
            if(g_bEnableFountain)
                g_pEmitter->EnableParticles(ParticleEmitter::kFountain, D3DXVECTOR3(12.5f, 0.75f, 12.5f));
            else
                g_pEmitter->DisableParticles(ParticleEmitter::kFountain);

            break;
    }
}

//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has
// entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
// in the OnResetDevice callback should be released here, which generally includes all
// D3DPOOL_DEFAULT resources. See the "Lost Devices" section of the documentation for
// information about lost devices.
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9LostDevice();
    g_SettingsDlg.OnD3D9LostDevice();

    if( g_pFont )
        g_pFont->OnLostDevice();

    SAFE_RELEASE( g_pTextSprite );

    // cleanup game singletons and objects
	delete g_pTime;
	delete g_pDatabase;
	delete g_pMsgRoute;
	delete g_pDebugLog;

    // cleanup render data
    delete g_pRenderData;

    // cleanup font
    SAFE_RELEASE(g_pFont);

    // cleanup sound manager
    SAFE_DELETE(g_pSoundManager);
}

//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has
// been destroyed, which generally happens as a result of application termination or
// windowed/full screen toggles. Resources created in the OnCreateDevice callback
// should be released here, which generally includes all D3DPOOL_MANAGED resources.
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9DestroyDevice();
    g_SettingsDlg.OnD3D9DestroyDevice();
}
