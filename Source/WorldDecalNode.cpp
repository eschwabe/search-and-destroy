/*******************************************************************************
* Game Development Project
* WorldDecalNode.cpp
*
* Eric Schwabe
* 2010-03-09
*
* World Decals
*
*******************************************************************************/

#include "DXUT.h"
#include "WorldDecalNode.h"
#include "DXUT\SDKmisc.h"

// custom vertex structure definition
const D3DVERTEXELEMENT9 WorldDecalNode::m_sCustomVertexDeclaration[] =
{
    { 0, offsetof(CustomVertex, vPos        ), D3DDECLTYPE_FLOAT3  , 0, D3DDECLUSAGE_POSITION, 0 },
    { 0, offsetof(CustomVertex, vTexCoord   ), D3DDECLTYPE_FLOAT2  , 0, D3DDECLUSAGE_TEXCOORD, 0 },
    D3DDECL_END(),
};

/**
* Constructor
*/
WorldDecalNode::WorldDecalNode(const LPCWSTR sDecalFilename) :
    m_sDecalFilename(sDecalFilename),
    m_pDecalTexture(NULL),
    m_pCVDeclaration(NULL)
{
}

/**
* Deconstructor
*/
WorldDecalNode::~WorldDecalNode()
{
    // cleanup vertex declaration
    SAFE_RELEASE(m_pCVDeclaration);

    // cleanup texture
    SAFE_RELEASE(m_pDecalTexture);
}

/**
* Add new player tracking
*/
void WorldDecalNode::AddPlayerTracking(const PlayerNode* player)
{
    PlayerData pData;
    pData.pPlayer = player;
    m_PlayerList.push_back(pData);
}

/**
* Add floor decal to world
*/
void WorldDecalNode::AddFloorDecal(const D3DXVECTOR3& vPos)
{
    Decal d;

    // check if list full, delete first if needed
    if(m_DecalList.size() >= dMaxDecals)
        m_DecalList.erase(m_DecalList.begin());

    // set position
    d.vPos = vPos;

    // add decal
    m_DecalList.push_back(d);
}

/**
* Initialize world node
*/
HRESULT WorldDecalNode::InitializeNode(IDirect3DDevice9* pd3dDevice)
{
    // create vertex declaration
    HRESULT result = pd3dDevice->CreateVertexDeclaration(m_sCustomVertexDeclaration, &m_pCVDeclaration);

    // search and load floor texture
    if( SUCCEEDED(result) )
    {
        WCHAR wsNewPath[ MAX_PATH ];
        DXUTFindDXSDKMediaFileCch(wsNewPath, sizeof(wsNewPath), m_sDecalFilename.c_str());
        result = D3DXCreateTextureFromFile(pd3dDevice, wsNewPath, &m_pDecalTexture);
    }

    return result;
}

/**
* Update traversal for physics, AI, etc.
*/
void WorldDecalNode::UpdateNode(double fTime)      
{
    // check players for updates
    for(DWORD i = 0; i < m_PlayerList.size(); i++)
    {
        float fDecalDistance = abs( D3DXVec3Length( &(m_PlayerList[i].pPlayer->GetPlayerPosition() - m_PlayerList[i].vLastDecalPos) ) );

        if( fDecalDistance > 0.2f )
        {
            AddFloorDecal(m_PlayerList[i].pPlayer->GetPlayerPosition());
            m_PlayerList[i].vLastDecalPos = m_PlayerList[i].pPlayer->GetPlayerPosition();
        }
    }

    // update existing decals
    std::list<Decal>::iterator it = m_DecalList.begin();

    while(it != m_DecalList.end())
    {
        Decal& d = *it;

        // update decal life
        d.fLife += fTime;

        // remove decal if at end of life
        if(d.fLife > d.fTotalLife)
        {
            std::list<Decal>::iterator itDelete = it;
            ++it;
            m_DecalList.erase(itDelete);
            continue;
        }

        // move to next decal
        ++it;
    }
}

/**
* Render traversal for drawing objects
*/
void WorldDecalNode::RenderNode(IDirect3DDevice9* pd3dDevice, const RenderData& rData) 
{
    // build vertex buffer
    CustomVertex* cvBuffer = new CustomVertex[m_DecalList.size() * dDecalVertexCount];

    // form vertices for each decal
    DWORD dBufIdx = 0;
    for( std::list<Decal>::iterator it = m_DecalList.begin(); it != m_DecalList.end(); ++it ) 
    {
        Decal& d = *it;

        for(int i = 0; i < dDecalVertexCount; i++)
        {
            switch(i)
            {
            // quad triangle 1
            case 0: // V1
                cvBuffer[dBufIdx].vPos = D3DXVECTOR3(d.vPos.x - d.fSize, d.vPos.y, d.vPos.z - d.fSize);
                cvBuffer[dBufIdx].vTexCoord = D3DXVECTOR2(0.0f, 1.0f);
                break;
            case 1: // V2
                cvBuffer[dBufIdx].vPos = D3DXVECTOR3(d.vPos.x - d.fSize, d.vPos.y, d.vPos.z + d.fSize);
                cvBuffer[dBufIdx].vTexCoord = D3DXVECTOR2(0.0f, 0.0f);
                break;
            case 2: // V3
                cvBuffer[dBufIdx].vPos = D3DXVECTOR3(d.vPos.x + d.fSize, d.vPos.y, d.vPos.z + d.fSize);
                cvBuffer[dBufIdx].vTexCoord = D3DXVECTOR2(1.0f, 0.0f);
                break;

            // quad triangle 2
            case 3: // V1
                cvBuffer[dBufIdx].vPos = D3DXVECTOR3(d.vPos.x - d.fSize, d.vPos.y, d.vPos.z - d.fSize);
                cvBuffer[dBufIdx].vTexCoord = D3DXVECTOR2(0.0f, 1.0f);
                break;
            case 4: // V3
                cvBuffer[dBufIdx].vPos = D3DXVECTOR3(d.vPos.x + d.fSize, d.vPos.y, d.vPos.z + d.fSize);
                cvBuffer[dBufIdx].vTexCoord = D3DXVECTOR2(1.0f, 0.0f);
                break;
            case 5: // V4
                cvBuffer[dBufIdx].vPos = D3DXVECTOR3(d.vPos.x + d.fSize, d.vPos.y, d.vPos.z - d.fSize);
                cvBuffer[dBufIdx].vTexCoord = D3DXVECTOR2(1.0f, 1.0f);
                break;
            }

            dBufIdx++;
        }
    }

    if(dBufIdx > 0)
    {
        // set standard mesh transformation matrix
        pd3dDevice->SetTransform(D3DTS_WORLD, &rData.matWorld);

        // disable lighting and enable alpha blending
        pd3dDevice->SetRenderState(D3DRS_LIGHTING, false);

        // set depth bias (prevent z-fighting)
        float fDepthBias = 0.00f;
        float fDepthBiasSlope = -1.0f;
        pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
        pd3dDevice->SetRenderState(D3DRS_DEPTHBIAS, *((DWORD*)&fDepthBias));
        pd3dDevice->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, *((DWORD*)&fDepthBiasSlope));

        // set decal texture
        pd3dDevice->SetTexture(0, m_pDecalTexture);
            
        // enable alpha-blending
        pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

        // set vertex declaration
        pd3dDevice->SetVertexDeclaration(m_pCVDeclaration);

        // draw
        pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, dBufIdx/3, cvBuffer, sizeof(cvBuffer[0]) );
    }

    // cleanup temporary vertex buffer
    SAFE_DELETE_ARRAY(cvBuffer);

}