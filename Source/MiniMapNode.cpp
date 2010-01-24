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
                         const int iMapsize) :
    m_sMapTexture(sMapTexture),
    m_sMapMaskTexture(sMapMaskTexture),
    m_sMapBorderTexture(sMapBorderTexture),
    m_sPlayerLocTexture(sPlayerLocTexture),
    m_sNPCLocTexture(sNPCLocTexture),
    m_WorldNode(NULL),
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

    for(UINT i = 0; i < m_vPlayerMapInfo.size(); i++)
    {
        delete m_vPlayerMapInfo[i].cvPlayerLocVertices;
    }

    SAFE_RELEASE(m_pMiniMapTexture);
    SAFE_RELEASE(m_pMiniMapMaskTexture);
    SAFE_RELEASE(m_pMiniMapBorderTexture);
    SAFE_RELEASE(m_pPlayerLocTexture);
    SAFE_RELEASE(m_pNPCLocTexture);
}

/**
* Add player tracking. Displays player location on minimap.
*/
void MiniMapNode::AddPlayerTracking(const PlayerNode* pPlayer, const PlayerType type) 
{ 
    PlayerMapInfo info;
    
    info.iTriangleCount = 2;
    info.iVertexCount = 4;
    info.cvPlayerLocVertices = new CustomVertex[info.iVertexCount];

    info.pPlayer = pPlayer;
    info.type = type;

    m_vPlayerMapInfo.push_back(info);
}

/**
* Initialize minimap
*/
HRESULT MiniMapNode::InitializeNode(IDirect3DDevice9* pd3dDevice)
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
* Update minimap
*/
void MiniMapNode::UpdateNode(double fTime)
{
    // initialize reference vertices if needed   
    if(m_iWinHeight == 0 || m_iWinWidth == 0)
        InitializeReferenceVertices();

    // default player location
    D3DXVECTOR3 vPlayerLoc = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    float fPlayerRot = 0.0f;

    // update player vertices
    for(UINT i = 0; i < m_vPlayerMapInfo.size(); i++)
    {
        // reset player vertices
        m_vPlayerMapInfo[i].cvPlayerLocVertices[0] = m_cvRefVertices[0];
        m_vPlayerMapInfo[i].cvPlayerLocVertices[1] = m_cvRefVertices[1];
        m_vPlayerMapInfo[i].cvPlayerLocVertices[2] = m_cvRefVertices[2];
        m_vPlayerMapInfo[i].cvPlayerLocVertices[3] = m_cvRefVertices[3];

        if( m_vPlayerMapInfo[i].type == PLAYER )
        {
            // PLAYER: save location and rotation, no transform required
            vPlayerLoc = m_vPlayerMapInfo[i].pPlayer->GetPlayerPosition();
            fPlayerRot = m_vPlayerMapInfo[i].pPlayer->GetPlayerRotation();
        }
        else
        {
            //D3DXVECTOR3 vNPCMapLoc = vPlayerLoc +

            // NPC: transform coordinates
            TransformTextureCoords(
                m_vPlayerMapInfo[i].pPlayer->GetPlayerPosition() - vPlayerLoc, 
                fPlayerRot, //0.0f,//m_vPlayerMapInfo[i].pPlayer->GetPlayerRotation(),
                m_vPlayerMapInfo[i].iVertexCount,
                m_vPlayerMapInfo[i].cvPlayerLocVertices);
        }
    }

    // reset minimap vertices
    m_cvMapVertices[0] = m_cvRefVertices[0];
    m_cvMapVertices[1] = m_cvRefVertices[1];
    m_cvMapVertices[2] = m_cvRefVertices[2];
    m_cvMapVertices[3] = m_cvRefVertices[3];

    // transform minimap vertices
    TransformTextureCoords(
        vPlayerLoc, 
        fPlayerRot,
        m_iVertexCount,
        m_cvMapVertices);
}

/**
* Transform texture coordinates based on the specified world location and rotation. 
* Converts world coordinates to texture coordinates. World coords are located in
* the x-z plane.
*/
void MiniMapNode::TransformTextureCoords(const D3DXVECTOR3& vWorldLoc, const float& fRotation, const int& iVertexCount, CustomVertex* cvVertices)
{
    D3DXVECTOR2 vTextureCoords;

    // translate map based on player position (x-z plane)
    // convert player position into minimap uv coordinates (normalize and reverse z axis)
    vTextureCoords.x = vWorldLoc.x/m_WorldNode->GetWorldWidth();
    vTextureCoords.y = 1.0f-(vWorldLoc.z/m_WorldNode->GetWorldHeight());

    // determine texture upper left corner uv coordinate (0,0)
    vTextureCoords.x -= 0.5f;
    vTextureCoords.y -= 0.5f;

    // set rotation center to texture mid-point 
    D3DXVECTOR2 vRotationCenter = D3DXVECTOR2(0.5f, 0.5f);

    D3DXMATRIX mxMapTransform;
    D3DXMatrixTransformation2D( 
        &mxMapTransform,            // output matrix
        NULL,                       // scaling center (Vec2)
        1.0f,                       // scaling rotation
        NULL,                       // scaling (Vec2)
        &vRotationCenter,           // rotation center (Vec2)
        fRotation,                  // rotation
        &vTextureCoords);           // translation
    
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
void MiniMapNode::RenderNode(IDirect3DDevice9* pd3dDevice, D3DXMATRIX rMatWorld)
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
    pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1); // use first alpha argument without modification (output = arg1)
    pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);   // set first alpha argument as texture
    pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1); // use first color argument without modification (output = arg1)
    pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);   // set first color argument as texture

    // setup second texture stage state (replace alpha with texture 1 alpha)
    pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1); // use first alpha argument without modification (output = arg1)
    pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);   // set first alpha argument as texture
    pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1); // use first color argument without modification (output = arg1)
    pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);   // set first color argument as previous stage

    // draw minimap primitive
    pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, m_iTriangleCount, m_cvMapVertices, sizeof(m_cvMapVertices[0]) );

    // draw minimap border
    pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE); // disable alpha overwriting
    pd3dDevice->SetTexture(0, m_pMiniMapBorderTexture);
    pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, m_iTriangleCount, m_cvRefVertices, sizeof(m_cvRefVertices[0]) );

    // draw player locations
    for(UINT i = 0; i < m_vPlayerMapInfo.size(); i++)
    {
        if(m_vPlayerMapInfo[i].type == PLAYER)
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
}
