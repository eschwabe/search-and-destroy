/*******************************************************************************
* Game Development Project
* SlinkyNode.cpp
*
* Eric Schwabe
* 2009-02-20
*
* Slinky Node
*
*******************************************************************************/

#include "DXUT.h"
#include "SlinkyNode.h"

// custom vertex structure definition
const D3DVERTEXELEMENT9 SlinkyNode::m_sCustomVertexDeclaration[] =
{
    { 0, offsetof(CustomVertex, vPos        ), D3DDECLTYPE_FLOAT3  , 0, D3DDECLUSAGE_POSITION, 0 },
    { 0, offsetof(CustomVertex, vNormal     ), D3DDECLTYPE_FLOAT3  , 0, D3DDECLUSAGE_NORMAL  , 0 },
    { 0, offsetof(CustomVertex, cDiffuse    ), D3DDECLTYPE_D3DCOLOR, 0, D3DDECLUSAGE_COLOR   , 0 },
    D3DDECL_END(),
};

/**
* Constructor
*/
SlinkyNode::SlinkyNode(void) :
    m_CVBuffer(NULL)
{
}

/**
* Deconstructor
*/
SlinkyNode::~SlinkyNode(void)
{
    SAFE_RELEASE(m_pCVDeclaration);
    SAFE_DELETE_ARRAY(m_CVBuffer);
    SAFE_RELEASE(m_pCylMesh);
}

/**
* Initialize node
*/
HRESULT SlinkyNode::InitializeNode(IDirect3DDevice9* pd3dDevice)
{
    
    // build cylinder
    HRESULT result = D3DXCreateCylinder(
        pd3dDevice,     // d3d device
        2.0f,           // radius1
        2.0f,           // radius2
        10.0f,          // length
        25,             // slices
        25,             // stacks
        &m_pCylMesh,    // mesh (output)
        NULL            // buffer (output)
    );

    // copy cylinder vertices into new vertex buffer
    if( SUCCEEDED(result) )
    {
        // lock and get entire buffer
        struct CylMeshFormat
        {
            D3DXVECTOR3 vPos;
            D3DXVECTOR3 vNormal;
        };

        CylMeshFormat* pVBData = NULL;
        result = m_pCylMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&pVBData);

        if( SUCCEEDED(result) )
        {
            // create vertex buffer
            m_CVBufferSize = m_pCylMesh->GetNumVertices();
            m_CVTriangleCount = m_pCylMesh->GetNumFaces();
            m_CVBuffer = new CustomVertex[m_CVBufferSize];

            // copy data
            for(DWORD i = 0; i < m_CVBufferSize; i++)
            {
                m_CVBuffer[i] = CustomVertex( pVBData[i].vPos, pVBData[i].vNormal, D3DXCOLOR(0.5f, 0.5f, 0.5f, 1) );
            }
        }
    }

    // get index buffer
    if( SUCCEEDED(result) )
    {
        result = m_pCylMesh->LockIndexBuffer( D3DLOCK_READONLY, &m_CVIndexBuffer );
    }

    // get index buffer description
    if( SUCCEEDED(result) )
    {    
        LPDIRECT3DINDEXBUFFER9 pCVIndexBuffer;
        result = m_pCylMesh->GetIndexBuffer(&pCVIndexBuffer);

        if( SUCCEEDED(result) )
        {
            result = pCVIndexBuffer->GetDesc(&m_CVIndexDesc);  
        }

        SAFE_RELEASE(pCVIndexBuffer);
    }
   
    // create vertex declaration
    if( SUCCEEDED(result) )
    {
        result = pd3dDevice->CreateVertexDeclaration(m_sCustomVertexDeclaration, &m_pCVDeclaration);
    }

    return result;
}

/**
* Update node
*/
void SlinkyNode::UpdateNode(double fTime)
{
}

/**
* Render node
*/
void SlinkyNode::RenderNode(IDirect3DDevice9* pd3dDevice, const RenderData& rData)
{
    // compute slinky world matrix
    //D3DXMATRIX matRotate;
    //D3DXMATRIX matSlinkyWorld;
    //D3DXMatrixTranslation(&matSlinkyWorld, 17.5, 5, 10);
    //D3DXMatrixRotationYawPitchRoll(&matRotate, -D3DX_PI/2.0f, 0.0f, 0.0f);
    //matSlinkyWorld = matRotate * rData.matWorld * matSlinkyWorld ;

    // set standard mesh transformation matricies
/*    pd3dDevice->SetTransform(D3DTS_VIEW, &rData.matView);
    pd3dDevice->SetTransform(D3DTS_PROJECTION, &rData.matProjection);
    pd3dDevice->SetTransform(D3DTS_WORLD, &matSlinkyWorld);*/

    // setup shader lighting
    rData.EnableShaders(pd3dDevice);
    //rData.EnableD3DLighting(pd3dDevice);

    // set vertex declaration
    pd3dDevice->SetVertexDeclaration(m_pCVDeclaration);

    // draw indexed primitive
    pd3dDevice->DrawIndexedPrimitiveUP(
        D3DPT_TRIANGLELIST,
        0,
        m_CVBufferSize,
        m_CVTriangleCount,
        m_CVIndexBuffer,
        m_CVIndexDesc.Format,
        m_CVBuffer,
        sizeof(m_CVBuffer[0]));
}
