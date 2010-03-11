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
* Add decal to world
*/
void WorldDecalNode::AddDecalToQuad(
    D3DXVECTOR3 vQuadP1, 
    D3DXVECTOR3 vQuadP2,
    D3DXVECTOR3 vQuadP3,
    D3DXVECTOR3 vQuadP4,
    const D3DXVECTOR3& vPos,
    const D3DXVECTOR3& vNormal)
{
    Decal d;

    // check if list full, delete first if needed
    if(m_DecalList.size() >= dMaxDecals)
        m_DecalList.erase(m_DecalList.begin());

    // compute vector from position to quad corner
    // set length to quad size
    // add new vector to point to get decal quad point
    D3DXVec3Normalize(&vQuadP1, &(vQuadP1-vPos)); vQuadP1 *= d.fSize;
    D3DXVec3Normalize(&vQuadP2, &(vQuadP2-vPos)); vQuadP2 *= d.fSize;
    D3DXVec3Normalize(&vQuadP3, &(vQuadP3-vPos)); vQuadP3 *= d.fSize;
    D3DXVec3Normalize(&vQuadP4, &(vQuadP4-vPos)); vQuadP4 *= d.fSize;
    
    // calculate decal points
    d.vPoint[0] = vPos + vQuadP1 + 0.01f*vNormal;
    d.vPoint[1] = vPos + vQuadP2 + 0.01f*vNormal;
    d.vPoint[2] = vPos + vQuadP3 + 0.01f*vNormal;
    d.vPoint[3] = vPos + vQuadP4 + 0.01f*vNormal;

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
    // update existing particles
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
            case 0:
                cvBuffer[dBufIdx].vPos = d.vPoint[0];
                cvBuffer[dBufIdx].vTexCoord = D3DXVECTOR2(0.0f, 0.0f);
                break;
            case 1:
                cvBuffer[dBufIdx].vPos = d.vPoint[1];
                cvBuffer[dBufIdx].vTexCoord = D3DXVECTOR2(0.0f, 1.0f);
                break;
            case 2:
                cvBuffer[dBufIdx].vPos = d.vPoint[2];
                cvBuffer[dBufIdx].vTexCoord = D3DXVECTOR2(1.0f, 0.0f);
                break;

            // quad triangle 2
            case 3:
                cvBuffer[dBufIdx].vPos = d.vPoint[1];
                cvBuffer[dBufIdx].vTexCoord = D3DXVECTOR2(0.0f, 1.0f);
                break;
            case 4:
                cvBuffer[dBufIdx].vPos = d.vPoint[2];
                cvBuffer[dBufIdx].vTexCoord = D3DXVECTOR2(1.0f, 1.0f);
                break;
            case 5:
                cvBuffer[dBufIdx].vPos = d.vPoint[3];
                cvBuffer[dBufIdx].vTexCoord = D3DXVECTOR2(1.0f, 0.0f);
                break;
            }

            dBufIdx++;
        }
    }

    // set standard mesh transformation matrix
    pd3dDevice->SetTransform(D3DTS_WORLD, &rData.matWorld);

    // disable lighting
    pd3dDevice->SetRenderState(D3DRS_LIGHTING, false);
    
    // set decal texture
    pd3dDevice->SetTexture(0, m_pDecalTexture);

    // set vertex declaration
    pd3dDevice->SetVertexDeclaration(m_pCVDeclaration);

    // draw
    pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, dBufIdx/3, cvBuffer, sizeof(cvBuffer[0]) );

    // cleanup temporary vertex buffer
    SAFE_DELETE(cvBuffer);

}