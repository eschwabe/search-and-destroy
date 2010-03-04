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
SlinkyNode::SlinkyNode(const float& x, const float& y, const float &z) :
    m_CVBuffer(NULL),
    m_vPos(x, y, z)
{
    // set cylinder length
    m_fCylLength = 15.0f;

    // allocate bone matrices
    m_dNumBones = 10;
    m_pBoneData = new BoneData[m_dNumBones];
    m_pBoneMatrices = new D3DXMATRIX[m_dNumBones];

    // setup bones
    D3DXVECTOR3 vCurrentPos = D3DXVECTOR3(0, 0, 0);

    for(DWORD i = 0; i < m_dNumBones; i++)
    {
        // set bone origin (based on parent bone)
        m_pBoneData[i].vPos = vCurrentPos;

        // build bone translation matrix
        D3DXMatrixTranslation(&m_pBoneData[i].matBoneTranslation, vCurrentPos.x, vCurrentPos.y, vCurrentPos.z);
        D3DXMatrixIdentity(&m_pBoneData[i].matBoneTransform);
        
        // compute next bone origin (only root bone has a z position of 0)
        // all bones are evenly distributed
        vCurrentPos.z = m_fCylLength/m_dNumBones;
    }
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
    SAFE_DELETE_ARRAY(m_pBoneData);
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

    // set basic vertex info, set bottom of cylinder to world origin
    cv.vPos = vPos;
    cv.vPos.z += m_fCylLength/2;
    cv.vNormal = vNormal;
    cv.cDiffuse = D3DXCOLOR(0.3f, 0.6f, 0.5f, 1);

    // cylinder (by default) is aligned to z axis with center at world origin
    // break cylinder into pieces equal to number of bones
    // assign bottom of cylinder as root bone

    // create bone vector along z axis
    D3DXVECTOR3 vBone = D3DXVECTOR3(0, 0, 1.0f);
    float fVertexLength = D3DXVec3Dot(&cv.vPos, &vBone);

    // determine length of each bone
    float fBoneLength = m_fCylLength / m_dNumBones;

    // assign vertex to bone
    for(DWORD iBoneIndex = 0; iBoneIndex < m_dNumBones; iBoneIndex++)
    {
        // check if vertex within this bone
        if(fVertexLength >= fBoneLength*iBoneIndex &&
           fVertexLength < fBoneLength*(iBoneIndex+1))
        {
            // assign bone indices
            // ensure indices do not reference non-existant bones
            float prevIndex = 0.0f; 
            if(iBoneIndex > 0) 
                prevIndex = iBoneIndex-1.0f;
            
            float nextIndex = iBoneIndex+1.0f; 
            if(nextIndex >= m_dNumBones) 
                nextIndex = m_dNumBones-1.0f;

            cv.fBlendIndicies = D3DXVECTOR4(prevIndex, (float)iBoneIndex, nextIndex, 0.0f);

            // assign bone weights
            // if vertex before bone mid-point, distribute weight between assigned and previous bone
            // if vertex after bone mid-point, distribute weight between assigned and next bone
            float fAdjBoneWeight = (fVertexLength - fBoneLength*iBoneIndex) / fBoneLength;
            float fAssignBoneWeight = 1.0f;
            float fPrevBoneWeight = 0.0f;
            float fNextBoneWeight = 0.0f;

            if(fAdjBoneWeight < 0.5f)
            {
                fAssignBoneWeight = fAdjBoneWeight + 0.5f;
                fPrevBoneWeight = 1.0f - fAssignBoneWeight;
            }
            else if(fAdjBoneWeight > 0.5f)
            {
                fAssignBoneWeight = (1.0f - fAdjBoneWeight) + 0.5f;
                fNextBoneWeight = 1.0f - fAssignBoneWeight;
            }
            else
            {
                fAssignBoneWeight = 0.0f;
            }

            cv.fBlendWeights = D3DXVECTOR4(fPrevBoneWeight, fAssignBoneWeight, fNextBoneWeight, 0.0f);

            break;
        }
    }

    return cv;
}

/**
* Update node
*/
void SlinkyNode::UpdateNode(double fTime)
{
    // determine rotation (per bone)
    static double fTotalTime = 0.0;
    fTotalTime += fTime;
    
    // create rotation matrix (adjusts over time)
    D3DXMATRIX matRotation;
    D3DXMatrixRotationYawPitchRoll(&matRotation, cos((float)fTotalTime/3.0f)/6.0f, sin((float)fTotalTime)/6.0f, 0.0f);

    // update all bone matrices
    // each bone matrix must be applied to it's children
    // each cylinder bone is a child of the previous bone (except root)
    // |root|--|--|--|--|--|--|
    D3DXMATRIX matIdentity;
    D3DXMatrixIdentity(&matIdentity);

    // matrices to store current transforms for each bone
    D3DXMATRIX matInvTrans = matIdentity;
    D3DXMATRIX matTrans = matIdentity;

    // update each successive bone with previous bone's transform matricies
    // add additional rotation at each joint
    for(DWORD i = 0; i < m_dNumBones; i++)
    {
        D3DXMATRIX matBoneInvTrans;
        D3DXMATRIX matBoneTrans = m_pBoneData[i].matBoneTranslation;
        D3DXMatrixInverse(&matBoneInvTrans, NULL, &matBoneTrans);

        matInvTrans = matInvTrans * matBoneInvTrans;
        matTrans = matRotation * matBoneTrans * matTrans;
        
        m_pBoneData[i].matBoneTransform = matInvTrans * matTrans;
    }
}

/**
* Render node
*/
void SlinkyNode::RenderNode(IDirect3DDevice9* pd3dDevice, const RenderData& rData)
{
    // compute slinky world matrix
    // move and rotate cylinder to stand on world grid
    D3DXMATRIX matRotate;
    D3DXMATRIX matSlinkyWorld;
    D3DXMatrixTranslation(&matSlinkyWorld, m_vPos.x, m_vPos.y, m_vPos.z);
    D3DXMatrixRotationYawPitchRoll(&matRotate, 0.0f, -D3DX_PI/2.0f, 0.0f);
    matSlinkyWorld = matRotate * matSlinkyWorld;

    // setup shader bone matrices
    for(DWORD i = 0; i < m_dNumBones; i++)
    {
        m_pBoneMatrices[i] = m_pBoneData[i].matBoneTransform;
    }

    // enable skinning shaders
    rData.EnableSkinShaders(pd3dDevice, matSlinkyWorld, m_pBoneMatrices, m_dNumBones);

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
