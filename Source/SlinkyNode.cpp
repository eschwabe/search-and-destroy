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
    { 0, offsetof(CustomVertex, vPos            ), D3DDECLTYPE_FLOAT3,      0, D3DDECLUSAGE_POSITION,       0 },
    { 0, offsetof(CustomVertex, vNormal         ), D3DDECLTYPE_FLOAT3,      0, D3DDECLUSAGE_NORMAL,         0 },
    { 0, offsetof(CustomVertex, cDiffuse        ), D3DDECLTYPE_D3DCOLOR,    0, D3DDECLUSAGE_COLOR,          0 },
    { 0, offsetof(CustomVertex, fBlendWeights   ), D3DDECLTYPE_FLOAT4,      0, D3DDECLUSAGE_BLENDWEIGHT,    0 },
    { 0, offsetof(CustomVertex, fBlendIndicies  ), D3DDECLTYPE_FLOAT4,      0, D3DDECLUSAGE_BLENDINDICES,   0 },
    D3DDECL_END(),
};

/**
* Constructor
*/
SlinkyNode::SlinkyNode(void) :
    m_CVBuffer(NULL)
{
    // set cylinder length
    m_fCylLength = 15.0f;

    // allocate bone matrices
    m_dNumBones = 10;
    m_pBoneMatrices = new D3DXMATRIX[m_dNumBones];
}

/**
* Deconstructor
*/
SlinkyNode::~SlinkyNode(void)
{
    // cleanup custom vertex
    SAFE_RELEASE(m_pCVDeclaration);
    SAFE_DELETE_ARRAY(m_CVBuffer);

    // cleanup mesh
    SAFE_RELEASE(m_pCylMesh);

    // cleanup bone matrices
    SAFE_DELETE_ARRAY(m_pBoneMatrices);
}

/**
* Initialize node
*/
HRESULT SlinkyNode::InitializeNode(IDirect3DDevice9* pd3dDevice)
{
    // build cylinder
    HRESULT result = D3DXCreateCylinder(
        pd3dDevice,     // d3d device
        1.0f,           // radius1
        1.0f,           // radius2
        m_fCylLength,   // length
        50,             // slices
        50,             // stacks
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
                m_CVBuffer[i] = CreateCustomVertex( pVBData[i].vPos, pVBData[i].vNormal );
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
* Creates a custom vertex from a position and normal
*/
SlinkyNode::CustomVertex SlinkyNode::CreateCustomVertex( const D3DXVECTOR3& vPos, const D3DXVECTOR3& vNormal)
{
    CustomVertex cv;

    cv.vPos = vPos;
    cv.vNormal = vNormal;
    cv.cDiffuse = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1);

    // two bones, originating from origin, cylinder has length 10
    // bone1 = +z axis
    // bone2 = -z axis
    D3DXVECTOR3 bone1 = D3DXVECTOR3(0, 0, 7.5);   // matrix[0]
    D3DXVECTOR3 bone2 = D3DXVECTOR3(0, 0, -7.5);  // matrix[1]
    D3DXVECTOR3 bone1unit;
    D3DXVECTOR3 bone2unit;
    D3DXVec3Normalize(&bone1unit, &bone1);        
    D3DXVec3Normalize(&bone2unit, &bone2); 

    //

    // compute weights of each bone
    if(vPos.z > 0)
    {
        float weight = D3DXVec3Dot(&vPos, &bone1unit) / D3DXVec3Length(&bone1);
        cv.fBlendWeights = D3DXVECTOR4(weight, 1.0f-weight, 0, 0);
    }
    else
    {   
        float weight = D3DXVec3Dot(&vPos, &bone2unit) / D3DXVec3Length(&bone2);
        cv.fBlendWeights = D3DXVECTOR4(1.0f-weight, weight, 0, 0);
    }

    // matrix indices are same for all vertices (only two bones)
    cv.fBlendIndicies = D3DXVECTOR4(0, 1, 0, 0);

    return cv;
}

/**
* Update node
*/
void SlinkyNode::UpdateNode(double fTime)
{
    static float rot = -D3DX_PI/2.0f;
    rot += 0.001f;

    D3DXMATRIX mx;
    
    // bone 1 matrix
    D3DXMatrixRotationYawPitchRoll(&mx, rot, 0.0f, 0.0f);
    m_pBoneMatrices[0] = mx;

    // bone 2 matrix
    D3DXMatrixRotationYawPitchRoll(&mx, rot, 0.0f, 0.0f);
    m_pBoneMatrices[1] = mx;
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
    rData.EnableSkinShaders(pd3dDevice, m_pBoneMatrices);
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
