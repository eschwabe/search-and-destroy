/*******************************************************************************
* Game Development Project
* MiniMapNode.cpp
*
* Eric Schwabe
* 2010-01-22
*
* Minimap Node
*
*******************************************************************************/

#include "DXUT.h"
#include "MiniMapNode.h"
#include "DXUT\SDKmisc.h"

/**
* Constuct minimap node
*/
MiniMapNode::MiniMapNode(const LPCWSTR sMapTexture, 
                         const LPCWSTR sMapMaskTexture, 
                         const LPCWSTR sMapBorderTexture, 
                         const LPCWSTR sPlayerLocTexture, 
                         const LPCWSTR sNPCLocTexture,                    
                         const WorldNode* world,
                         const int iMapsize) :
    GameObject(g_database.GetNewObjectID(), OBJECT_Map, "MAP"),
    m_sMapTexture(sMapTexture),
    m_sMapMaskTexture(sMapMaskTexture),
    m_sMapBorderTexture(sMapBorderTexture),
    m_sPlayerLocTexture(sPlayerLocTexture),
    m_sNPCLocTexture(sNPCLocTexture),
    m_WorldNode(world),
    m_iMapSize(iMapsize),
    m_iWinHeight(0),
    m_iWinWidth(0)
{
    // create vertices
    m_iTriangleCount = 2;
    m_iVertexCount = 4;
    m_cvMapVertices = new CustomVertex[m_iVertexCount];
    m_cvRefVertices = new CustomVertex[m_iVertexCount];
}

/**
* Deconstructor
*/
MiniMapNode::~MiniMapNode()
{
    // cleanup vertices
    delete m_cvMapVertices;
    delete m_cvRefVertices;

    SAFE_RELEASE(m_pMiniMapTexture);
    SAFE_RELEASE(m_pMiniMapMaskTexture);
    SAFE_RELEASE(m_pMiniMapBorderTexture);
    SAFE_RELEASE(m_pPlayerLocTexture);
    SAFE_RELEASE(m_pNPCLocTexture);
}

/**
* Initialize minimap
*/
HRESULT MiniMapNode::Initialize(IDirect3DDevice9* pd3dDevice)
{
    WCHAR wsNewPath[ MAX_PATH ];

    // search and load minimap texture
    DXUTFindDXSDKMediaFileCch(wsNewPath, sizeof(wsNewPath), m_sMapTexture.c_str());
    if( FAILED(D3DXCreateTextureFromFile(
        pd3dDevice,
        wsNewPath,
        &m_pMiniMapTexture)) )
    {
        return E_FAIL;
    }

    // search and load minimap mask texture
    DXUTFindDXSDKMediaFileCch(wsNewPath, sizeof(wsNewPath), m_sMapMaskTexture.c_str());
    if( FAILED(D3DXCreateTextureFromFile(
        pd3dDevice,
        wsNewPath,
        &m_pMiniMapMaskTexture)) )
    {
        return E_FAIL;
    }

    // search and load minimap border texture
    DXUTFindDXSDKMediaFileCch(wsNewPath, sizeof(wsNewPath), m_sMapBorderTexture.c_str());
    if( FAILED(D3DXCreateTextureFromFile(
        pd3dDevice,
        wsNewPath,
        &m_pMiniMapBorderTexture)) )
    {
        return E_FAIL;
    }

    // search and load player location texture
    DXUTFindDXSDKMediaFileCch(wsNewPath, sizeof(wsNewPath), m_sPlayerLocTexture.c_str());
    if( FAILED(D3DXCreateTextureFromFile(
        pd3dDevice,
        wsNewPath,
        &m_pPlayerLocTexture)) )
    {
        return E_FAIL;
    }

    // search and load npc location texture
    DXUTFindDXSDKMediaFileCch(wsNewPath, sizeof(wsNewPath), m_sNPCLocTexture.c_str());
    if( FAILED(D3DXCreateTextureFromFile(
        pd3dDevice,
        wsNewPath,
        &m_pNPCLocTexture)) )
    {
        return E_FAIL;
    }

    return S_OK;
}

/**
* Initializes the reference vertices based on screen size. Screen size not accurate during
* object initialization.
*/
void MiniMapNode::InitializeReferenceVertices()
{
    // determine window size
    RECT rWinSize = DXUTGetWindowClientRect();
    m_iWinHeight = rWinSize.bottom;
    m_iWinWidth = rWinSize.right;

    // distance from side of screen
    int offset = 30;
    
    // compute reference vertices
    m_cvRefVertices[0] = CustomVertex( D3DXVECTOR3((float)m_iWinWidth-offset-m_iMapSize,  (float)m_iWinHeight-offset-m_iMapSize,  1),   0, 0,  0, 0);
    m_cvRefVertices[1] = CustomVertex( D3DXVECTOR3((float)m_iWinWidth-offset-m_iMapSize,  (float)m_iWinHeight-offset,             1),   0, 1,  0, 1);
    m_cvRefVertices[2] = CustomVertex( D3DXVECTOR3((float)m_iWinWidth-offset,             (float)m_iWinHeight-offset-m_iMapSize,  1),   1, 0,  1, 0);
    m_cvRefVertices[3] = CustomVertex( D3DXVECTOR3((float)m_iWinWidth-offset,             (float)m_iWinHeight-offset,             1),   1, 1,  1, 1);
}

/**
* Add player tracking. Displays player location on minimap.
*/
void MiniMapNode::UpdatePlayerTracking() 
{ 
    // clear player list
    m_vPlayerMapInfo.clear();

    // generate list of players and NPCs
    dbCompositionList pList;
    g_database.ComposeList( pList, OBJECT_NPC | OBJECT_Player );

    // check for player collisions with environment
    for(dbCompositionList::iterator it = pList.begin(); it != pList.end(); ++it)
    {
        // setup player map info
        PlayerMapInfo info;
        info.pObject = *it;

        // add info to list
        m_vPlayerMapInfo.push_back(info);
    }
}

/**
* Update minimap
*/
void MiniMapNode::Update()
{
    // find and update all players and NPCs in world
    UpdatePlayerTracking();

    // initialize reference vertices if needed   
    if(m_iWinHeight == 0 || m_iWinWidth == 0)
        InitializeReferenceVertices();

    // default player location
    D3DXVECTOR3 vPlayerLoc = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    float fPlayerRot = 0.0f;

    // find player
    for(UINT i = 0; i < m_vPlayerMapInfo.size(); i++)
    {
        if( m_vPlayerMapInfo[i].pObject->GetType() == OBJECT_Player )
        {
            // save location and rotation, no transform required
            vPlayerLoc = m_vPlayerMapInfo[i].pObject->GetPosition();
            fPlayerRot = m_vPlayerMapInfo[i].pObject->GetYawRotation();
            break;
        }
    }

    // reset minimap vertices
    m_cvMapVertices[0] = m_cvRefVertices[0];
    m_cvMapVertices[1] = m_cvRefVertices[1];
    m_cvMapVertices[2] = m_cvRefVertices[2];
    m_cvMapVertices[3] = m_cvRefVertices[3];

    // transform minimap vertices
    D3DXVECTOR2 vMapTextureCoords = ComputeTextureCoordsFromWorld(vPlayerLoc);
    TransformTextureCoords(
        vMapTextureCoords, 
        fPlayerRot,
        m_iVertexCount,
        m_cvMapVertices);

    // update player vertices
    for(UINT i = 0; i < m_vPlayerMapInfo.size(); i++)
    {
        // reset player vertices
        m_vPlayerMapInfo[i].cvPlayerLocVertices[0] = m_cvRefVertices[0];
        m_vPlayerMapInfo[i].cvPlayerLocVertices[1] = m_cvRefVertices[1];
        m_vPlayerMapInfo[i].cvPlayerLocVertices[2] = m_cvRefVertices[2];
        m_vPlayerMapInfo[i].cvPlayerLocVertices[3] = m_cvRefVertices[3];

        // transform npc vertices
        if( m_vPlayerMapInfo[i].pObject->GetType() == OBJECT_NPC )
        {           
            // transform NPC texture using same parameters as minimap texture
            // (orients NPC texture to player location and rotation)
            TransformTextureCoords(
                vMapTextureCoords, 
                fPlayerRot,
                m_vPlayerMapInfo[i].iVertexCount,
                m_vPlayerMapInfo[i].cvPlayerLocVertices);
            
            // translate NPC texture to NPC location
            D3DXVECTOR2 vNPCTextureCoords = ComputeTextureCoordsFromWorld(m_vPlayerMapInfo[i].pObject->GetPosition());

            // invert coordinates (fixes inverted result?)
            vNPCTextureCoords.x = -vNPCTextureCoords.x;
            vNPCTextureCoords.y = -vNPCTextureCoords.y;

            // transform
            TransformTextureCoords(
                vNPCTextureCoords, 
                0.0f,
                m_vPlayerMapInfo[i].iVertexCount,
                m_vPlayerMapInfo[i].cvPlayerLocVertices);
        }
    }
}

/**
* Converts world coordinates into texture (0,0) coordinates. World coords are located in
* the x-z plane.
*/
D3DXVECTOR2 MiniMapNode::ComputeTextureCoordsFromWorld(const D3DXVECTOR3& vWorldLoc)
{
    D3DXVECTOR2 vTextureCoords;

    // translate map based on player position (x-z plane)
    // convert player position into minimap uv coordinates (normalize and reverse z axis)
    vTextureCoords.x = vWorldLoc.x/m_WorldNode->GetWorldWidth();
    vTextureCoords.y = 1.0f-(vWorldLoc.z/m_WorldNode->GetWorldHeight());

    // determine texture upper left corner uv coordinate (0,0)
    vTextureCoords.x -= 0.5f;
    vTextureCoords.y -= 0.5f;

    return vTextureCoords;
}

/**
* Transform texture coordinates based on the specified translation and rotation.  
*/
void MiniMapNode::TransformTextureCoords(const D3DXVECTOR2& vTranslate, const float& fRotation, const int& iVertexCount, CustomVertex* cvVertices)
{
    // default rotation center
    D3DXVECTOR2 vRotationCenter = D3DXVECTOR2(0.5f, 0.5f);

    // create transform matrix
    D3DXMATRIX mxMapTransform;
    D3DXMatrixTransformation2D( 
        &mxMapTransform,            // output matrix
        NULL,                       // scaling center (Vec2)
        1.0f,                       // scaling rotation
        NULL,                       // scaling (Vec2)
        &vRotationCenter,           // rotation center (Vec2)
        fRotation,                  // rotation
        &vTranslate);               // translation
    
    // transform vertices
    for(int i = 0; i < iVertexCount; i++)
    {
        // copy current uv
        D3DXVECTOR2 vTextureCoords;
        vTextureCoords.x = cvVertices[i].u1;
        vTextureCoords.y = cvVertices[i].v1;

        // transform uv
        D3DXVECTOR4 vOutput; 
        D3DXVec2Transform(&vOutput, &vTextureCoords, &mxMapTransform);

        // update vertex
        cvVertices[i].u1 = vOutput.x;
        cvVertices[i].v1 = vOutput.y;
    }
}

/**
* Render minimap
*/
void MiniMapNode::Render(IDirect3DDevice9* pd3dDevice, const RenderData* rData)
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

    // set texture wrapping to use the border color
    pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
    pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
    pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSW, D3DTADDRESS_BORDER);

    // Set vertex type
    pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

    // set minimap texture and mask texture
    pd3dDevice->SetTexture(0, m_pMiniMapTexture);
    pd3dDevice->SetTexture(1, m_pMiniMapMaskTexture);

    // setup first texture stage state (select texture 0 alpha and color)
    //pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1); // use first alpha argument without modification (output = arg1)
    //pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);   // set first alpha argument as texture
    pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1); // use first color argument without modification (output = arg1)
    //pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);   // set first color argument as texture

    // setup second texture stage state (replace alpha with texture 1 alpha)
    pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1); // use first alpha argument without modification (output = arg1)
    //pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);   // set first alpha argument as texture
    pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1); // use first color argument without modification (output = arg1)
    pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);   // set first color argument as previous stage

    // draw minimap primitive
    pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, m_iTriangleCount, m_cvMapVertices, sizeof(m_cvMapVertices[0]) );

    // blend alpha channels for player locations (with minimap mask texture)
    pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_BLENDTEXTUREALPHA);  // blend alpha
    pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);           // set first alpha argument as previous texture
    pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);           // set second alpha argument as texture

    // draw player locations
    for(UINT i = 0; i < m_vPlayerMapInfo.size(); i++)
    {
        if(m_vPlayerMapInfo[i].pObject->GetType() == OBJECT_Player)
        {
            // set player location texture
            pd3dDevice->SetTexture(0, m_pPlayerLocTexture);
        }
        else
        {
            // set npc location texture
            pd3dDevice->SetTexture(0, m_pNPCLocTexture);
        }

        // draw location
        pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, m_iTriangleCount, 
            m_vPlayerMapInfo[i].cvPlayerLocVertices, sizeof(m_vPlayerMapInfo[i].cvPlayerLocVertices[0]) );
    }

    // draw minimap border
    pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE); // disable alpha overwriting
    pd3dDevice->SetTexture(0, m_pMiniMapBorderTexture);
    pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, m_iTriangleCount, m_cvRefVertices, sizeof(m_cvRefVertices[0]) );
}
