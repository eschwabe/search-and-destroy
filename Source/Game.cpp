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

#include "Game.h"
#include "GameController.h"
#include "RenderData.h"
#include "database.h"
#include "msgroute.h"
#include "debuglog.h"
#include "time.h"

#include "PlayerTinyNode.h"
#include "NPCSphereNode.h"

#include "SMPatrol.h"
#include "SMRandomPath.h"
#include "SMWander.h"
#include "SMPlayer.h"
#include "SMGame.h"

#include "WorldNode.h"
#include "WorldFile.h"
#include "WorldData.h"
#include "MiniMapNode.h"
#include "ProjectileParticles.h"

using namespace std;

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
CDXUTDialogResourceManager  g_DialogResourceManager;    // manager for shared resources of dialogs
CD3DSettingsDlg             g_SettingsDlg;              // device settings dialog
CDXUTDialog                 g_HUD;                      // dialog for standard controls
CDXUTDialog                 g_SampleUI;                 // dialog for sample specific controls
Time*                       g_pTime = NULL;             // time manager
Database*                   g_pDatabase = NULL;         // game object database
MsgRoute*                   g_pMsgRoute = NULL;         // message router
DebugLog*                   g_pDebugLog = NULL;         // debug logger
ObjectCollision*            g_objColl = NULL;           // collision object
WorldData*                  g_WorldData = NULL;         // world pathfinding
RenderData*                 g_pRenderData = NULL;       // render data
WorldFile*                  g_pWorldFile = NULL;        // world data
GameController*             g_pGameController = NULL;   // game control

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
#define IDC_DEBUGPATHING        13
#define IDC_DEBUGTERRAIN        14

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------

void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );

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
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN,  L"Full Screen (F1)",    35, iY += 10, 125, 22, VK_F1 );
    g_HUD.AddButton( IDC_CHANGEDEVICE,      L"Change Device (F2)",  35, iY += 24, 125, 22, VK_F2 );
    g_HUD.AddButton( IDC_TOGGLEREF,         L"Toggle REF (F3)",     35, iY += 24, 125, 22, VK_F3 );
    g_HUD.AddButton( IDC_RESETTIME,         L"Reset Time",          35, iY += 48, 125, 22 );
    g_HUD.AddButton( IDC_DEBUGPATHING,      L"Debug Pathing",       35, iY += 24, 125, 22 );
    g_HUD.AddButton( IDC_DEBUGTERRAIN,      L"Terrain Analysis",    35, iY += 24, 125, 22 );
    
    // Add mixed vp to the available vp choices in device settings dialog.
    DXUTGetD3D9Enumeration()->SetPossibleVertexProcessingList( true, false, false, true );
   
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

    // load world file
    g_pWorldFile = new WorldFile();
    if( !g_pWorldFile->Load(L"level.grd") )
        return E_FAIL;

    // initialize singleton objects
	g_pTime = new Time();
	g_pDatabase = new Database();
	g_pMsgRoute = new MsgRoute();
	g_pDebugLog = new DebugLog();
    g_WorldData = new WorldData(*g_pWorldFile);

    // add world object
    WorldNode* p_WorldNode = new WorldNode(*g_pWorldFile, L"asphalt-damaged.jpg", L"painted_metal.jpg");
    g_database.Store(p_WorldNode);

    // add player node
    PlayerTinyNode* pMainPlayerNode = new PlayerTinyNode( L"tiny.x", D3DXVECTOR3(12.5f,0.0f,2.0f) );
    g_database.Store(pMainPlayerNode);

    // add sphere NPCs
    NPCSphereNode* pNPC1 = new NPCSphereNode(D3DXVECTOR3(2.5f,0.0f,22.5f));
    NPCSphereNode* pNPC2 = new NPCSphereNode(D3DXVECTOR3(22.5f,0.0f,2.5f));
    NPCSphereNode* pNPC3 = new NPCSphereNode(D3DXVECTOR3(2.5f,0.0f,2.5f), D3DXCOLOR(0.3f, 0.4f, 0.7f, 1));
    NPCSphereNode* pNPC4 = new NPCSphereNode(D3DXVECTOR3(22.5f,0.0f,22.5f), D3DXCOLOR(0.3f, 0.4f, 0.7f, 1));
    g_database.Store( pNPC1 );
    g_database.Store( pNPC2 );
    g_database.Store( pNPC3 );
    g_database.Store( pNPC4 );

    // add particle emitter and fire particles
    ProjectileParticles* fb1 = new ProjectileParticles(D3DXVECTOR3(3.0f, 0.2f, 3.0f), ProjectileParticles::kFire);
    g_database.Store(fb1);
    ProjectileParticles* fb2 = new ProjectileParticles(D3DXVECTOR3(3.0f, 0.2f, 22.0f), ProjectileParticles::kFire);
    g_database.Store(fb2);
    ProjectileParticles* fb3 = new ProjectileParticles(D3DXVECTOR3(22.0f, 0.2f, 22.0f), ProjectileParticles::kFire);
    g_database.Store(fb3);
    ProjectileParticles* fb4 = new ProjectileParticles(D3DXVECTOR3(22.0f, 0.2f, 3.0f), ProjectileParticles::kFire);
    g_database.Store(fb4);

    // add world data debug drawing to database
    g_database.Store(g_WorldData);

    // add minimap node (note: draw 2D elements after rendering 3D)
    MiniMapNode* p_MiniMap = new MiniMapNode(
        L"level-collision-map.png", 
        L"level-collision-map-alpha.png", 
        L"minimap-border.png", 
        L"minimap-player.png", 
        L"minimap-npc.png", 
        p_WorldNode,
        200);
    g_database.Store(p_MiniMap);

    // create game controller
    g_pGameController = new GameController(pMainPlayerNode->GetID());
    g_pGameController->ConfigureCameras(pBackBufferSurfaceDesc);
    g_database.Store(g_pGameController);

    // initialize database objects
    if( FAILED(g_database.InitializeObjects(pd3dDevice)) )
    {
        MessageBox( NULL, L"Could not load nodes", L"UWGame", MB_OK );
        return E_FAIL;
    }

    // create collision object
    g_objColl = new ObjectCollision( p_WorldNode->GetCollQuadList() );

    // setup NPC state machines
    pNPC1->GetStateMachineManager()->PushStateMachine( *new SMRandomPath(pNPC1, pMainPlayerNode->GetID()), STATE_MACHINE_QUEUE_0, TRUE );
    pNPC2->GetStateMachineManager()->PushStateMachine( *new SMRandomPath(pNPC2, pMainPlayerNode->GetID()), STATE_MACHINE_QUEUE_0, TRUE );
    pNPC3->GetStateMachineManager()->PushStateMachine( *new SMRandomPath(pNPC3, pMainPlayerNode->GetID()), STATE_MACHINE_QUEUE_0, TRUE );
    pNPC4->GetStateMachineManager()->PushStateMachine( *new SMRandomPath(pNPC4, pMainPlayerNode->GetID()), STATE_MACHINE_QUEUE_0, TRUE );
    
    // setup player state machine
    pMainPlayerNode->GetStateMachineManager()->PushStateMachine( *new SMPlayer(pMainPlayerNode, pd3dDevice), STATE_MACHINE_QUEUE_0, TRUE );

    // setup game controller state machine
    g_pGameController->GetStateMachineManager()->PushStateMachine( *new SMGame(g_pGameController, p_MiniMap->GetID()), STATE_MACHINE_QUEUE_0, TRUE );

    // initialize render data
    g_pRenderData = new RenderData();
    if( FAILED(g_pRenderData->Initialize(pd3dDevice)) )
        return E_FAIL;

    // configure lights
    g_pRenderData->SetDirectionalLight( D3DXVECTOR4(-0.5f, 0.1f, -0.25f, 1.0f), D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f) );
    g_pRenderData->SetAmbientLight( D3DXCOLOR(0.3f, 0.3f, 0.3f, 0.3f) );

    // adjust the dialog parameters
    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width-170, 0 );
    g_HUD.SetSize( 170, 170 );
    g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width-170, pBackBufferSurfaceDesc->Height-270 );
    g_SampleUI.SetSize( 170, 220 );

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

    // generate list of players and NPCs
    dbCompositionList pList;
    g_database.ComposeList( pList, OBJECT_NPC | OBJECT_Player );

    // check for player collisions with environment
    for(dbCompositionList::iterator it = pList.begin(); it != pList.end(); ++it)
    {
        g_objColl->RunWorldCollision(*it);
    }
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
    pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB( 0, 0x22, 0x55, 0x88 ), 1.0f, 0L );

    // begin scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
        HRESULT hr;

        // set static transforms
        V( pd3dDevice->SetTransform( D3DTS_VIEW, g_pGameController->GetCameraViewMatrix()) );
        V( pd3dDevice->SetTransform( D3DTS_PROJECTION, g_pGameController->GetCameraProjMatrix()) );

        // set rendering view and projection matrix
        g_pRenderData->matView = *g_pGameController->GetCameraViewMatrix();
        g_pRenderData->matProjection = *g_pGameController->GetCameraProjMatrix();

        // set world matrix
        D3DXMATRIX matIdentity;
        D3DXMatrixIdentity( &matIdentity );
        g_pRenderData->matWorld = matIdentity;

        // render all nodes
        g_database.RenderObjects(pd3dDevice, g_pRenderData);

        // render hud if debug mode
        if( g_pGameController->DebugEnabled() )
            V( g_HUD.OnRender( fElapsedTime ) );

        pd3dDevice->EndScene();
    }
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

    // pass messages to game controller
    if(g_pGameController)
        g_pGameController->HandleMessages( hWnd, uMsg, wParam, lParam );
    
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
                // do nothing
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

        case IDC_RESETTIME:
            DXUTGetGlobalTimer()->Reset();
            break;

        case IDC_DEBUGPATHING:
            g_world.TogglePathDebug();
            break;

        case IDC_DEBUGTERRAIN:
            g_world.ToggleTerrainAnalysisType();
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

    // cleanup game singletons and objects
	delete g_pTime;
	delete g_pDatabase;
	delete g_pMsgRoute;
	delete g_pDebugLog;
    delete g_objColl;

    // cleanup render data
    delete g_pRenderData;
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
