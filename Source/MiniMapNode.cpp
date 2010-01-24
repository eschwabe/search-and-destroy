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
MiniMapNode::MiniMapNode(const LPCWSTR sMapTexture, const LPCWSTR sMapMaskTexture) :
    m_sMapTexture(sMapTexture),
    m_sMapMaskTexture(sMapMaskTexture),
    m_pStateBlock(NULL),
    m_pPlayer(NULL)
{
    // create vertices
    m_iTriangleCount = 2;
    m_iVertexCount = 4;
    m_cvVertices = new CustomVertex[m_iVertexCount];
}

/**
* Deconstructor
*/
MiniMapNode::~MiniMapNode()
{
    delete m_cvVertices;

    SAFE_RELEASE(m_pMiniMapTexture);
    SAFE_RELEASE(m_pMiniMapMaskTexture);
    SAFE_RELEASE(m_pStateBlock);
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

    // setup state block
    if( FAILED(pd3dDevice->CreateStateBlock(D3DSBT_ALL, &m_pStateBlock)) )
    {
        return E_FAIL;
    }

    return S_OK;
}

/**
* Update minimap
*/
void MiniMapNode::UpdateNode(double fTime)
{
    // set minimap vertices
    m_cvVertices[0] = CustomVertex( D3DXVECTOR3(800,  500, 1),  0, 0,  0, 0);
    m_cvVertices[1] = CustomVertex( D3DXVECTOR3(800,  700, 1),  0, 1,  0, 1);
    m_cvVertices[2] = CustomVertex( D3DXVECTOR3(1000, 500, 1),  1, 0,  1, 0);
    m_cvVertices[3] = CustomVertex( D3DXVECTOR3(1000, 700, 1),  1, 1,  1, 1);

    // move minimap texture based on current position of player
    D3DXMATRIX mxMapTransform;
    D3DXVECTOR2 vMapTranslation;

    // translate map based on player position (x-z plane)
    // convert player position into minimap uv coordinates
    // TODO: divide by map size
    vMapTranslation.x = m_pPlayer->GetPlayerPosition().x/25;
    vMapTranslation.y = 1.0f-(m_pPlayer->GetPlayerPosition().z/25);

    // set player position to middle of minimap
    vMapTranslation.x -= 0.5f;
    vMapTranslation.y -= 0.5f;

    // set rotation center to texture mid-point 
    D3DXVECTOR2 vRotationCenter = D3DXVECTOR2(0.5f, 0.5f);

    D3DXMatrixTransformation2D( 
        &mxMapTransform,                    // output matrix
        NULL,                               // scaling center (Vec2)
        1.0f,                               // scaling rotation
        NULL,                               // scaling (Vec2)
        &vRotationCenter,                   // rotation center (Vec2)
        m_pPlayer->GetPlayerRotation(),     // rotation
        &vMapTranslation);                  // translation
    
    // transform vertices
    for(int i = 0; i < m_iVertexCount; i++)
    {
        // copy current uv
        D3DXVECTOR2 vTextureCoords;
        vTextureCoords.x = m_cvVertices[i].u1;
        vTextureCoords.y = m_cvVertices[i].v1;

        // transform uv
        D3DXVECTOR4 vOutput; 
        D3DXVec2Transform(&vOutput, &vTextureCoords, &mxMapTransform);

        // update vertex
        m_cvVertices[i].u1 = vOutput.x;
        m_cvVertices[i].v1 = vOutput.y;
    }
}

/**
* Render minimap
*/
void MiniMapNode::RenderNode(IDirect3DDevice9* pd3dDevice, D3DXMATRIX rMatWorld)
{
    // capture state
    m_pStateBlock->Capture();

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

    // set minimap texture and mask texture
    pd3dDevice->SetTexture(0, m_pMiniMapTexture);
    pd3dDevice->SetTexture(1, m_pMiniMapMaskTexture);

    // setup first texture stage state
    pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1); // use first alpha argument without modification (output = arg1)
    pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);   // set first alpha argument as texture
    pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1); // use first color argument without modification (output = arg1)
    pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);   // set first color argument as texture

    // setup second texture stage state
    pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1); // use first alpha argument without modification (output = arg1)
    pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);   // set first alpha argument as texture
    pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1); // use first color argument without modification (output = arg1)
    pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);   // set first color argument as previous stage

    // draw primitive
    pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, m_iTriangleCount, m_cvVertices, sizeof(m_cvVertices[0]) );

    // restore state
    m_pStateBlock->Apply();
}
