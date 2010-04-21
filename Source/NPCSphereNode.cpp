/*******************************************************************************
* Game Development Project
* NPCNode.h
*
* Eric Schwabe
* 2010-4-18
*
* Sphere NPC Player Node
*
*******************************************************************************/

#include "DXUT.h"
#include "NPCSphereNode.h"

// custom vertex structure definition
const D3DVERTEXELEMENT9 NPCSphereNode::m_sCustomVertexDeclaration[] =
{
    { 0, offsetof(CustomVertex, vPos            ), D3DDECLTYPE_FLOAT3,      0, D3DDECLUSAGE_POSITION,       0 },
    { 0, offsetof(CustomVertex, vNormal         ), D3DDECLTYPE_FLOAT3,      0, D3DDECLUSAGE_NORMAL,         0 },
    { 0, offsetof(CustomVertex, cDiffuse        ), D3DDECLTYPE_D3DCOLOR,    0, D3DDECLUSAGE_COLOR,          0 },
    D3DDECL_END(),
};

/**
* Constuct Sphere NPC node
*/
NPCSphereNode::NPCSphereNode(const D3DXVECTOR3& vInitialPos) :
    PlayerBaseNode(vInitialPos, g_database.GetNewObjectID(), OBJECT_NPC, "NPC"),
    m_dUpdateTime(0.0f)
{
    // set initial position off the ground (floating sphere)
    m_vPos.y += 0.5f;
}

/**
* Deconstruct NPC node
*/
NPCSphereNode::~NPCSphereNode()
{
    // cleanup custom vertex
    SAFE_RELEASE(m_pCVDeclaration);
    SAFE_DELETE_ARRAY(m_CVBuffer);

    // cleanup sphere mesh
    SAFE_RELEASE(m_pSphereMesh);
}

/**
* Initialize NPC node. Sets up data and debug line vertex.
*/
HRESULT NPCSphereNode::Initialize(IDirect3DDevice9* pd3dDevice)
{
    // generate sphere mesh
    HRESULT result = D3DXCreateSphere(
        pd3dDevice,
        0.25f,              // radius
        35,                 // slices
        35,                 // stacks
        &m_pSphereMesh,     // mesh
        NULL);

    if( SUCCEEDED(result) )
    {
        // sphere mesh vertex format
        struct SphereVertexFormat
        {
            D3DXVECTOR3 vPos;
            D3DXVECTOR3 vNormal;
        };
            
        SphereVertexFormat* pVBData = NULL;
        result = m_pSphereMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&pVBData);

        if( SUCCEEDED(result) )
        {
            // create vertex buffer
            m_CVBufferSize = m_pSphereMesh->GetNumVertices();
            m_CVTriangleCount = m_pSphereMesh->GetNumFaces();
            m_CVBuffer = new CustomVertex[m_CVBufferSize];

            // copy data
            for(DWORD i = 0; i < m_CVBufferSize; i++)
            {
                m_CVBuffer[i] = CustomVertex( pVBData[i].vPos, pVBData[i].vNormal, D3DXCOLOR(0.3f, 0.6f, 0.5f, 1) );
            }
        }
    }

    // get index buffer
    if( SUCCEEDED(result) )
    {
        result = m_pSphereMesh->LockIndexBuffer( D3DLOCK_READONLY, &m_CVIndexBuffer );
    }

    // get index buffer description
    if( SUCCEEDED(result) )
    {    
        LPDIRECT3DINDEXBUFFER9 pCVIndexBuffer;
        result = m_pSphereMesh->GetIndexBuffer(&pCVIndexBuffer);

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
* Update traversal for physics, AI, etc.
*/
void NPCSphereNode::Update()
{
    // randomly change movements
    AutoPlayerMove();

    // update position
    UpdatePlayerPosition();
}

/**
* Automatically changes the NPC movements.
*/
void NPCSphereNode::AutoPlayerMove()
{
    // check if enough time has passed
    if(g_time.GetCurTime() >= m_dUpdateTime)
    {
        // reset actions
        for(int i =0; i < kMaxMovement; i++)
            m_PlayerMovement[i] = false;

        // if collision, rotate player
        if(m_bEnvironemntCollision)
        {
            // reset collision flag
            m_bEnvironemntCollision = false;

            // choose rotate action
            switch(rand() % 2)
            {
            case 0:
                m_PlayerMovement[kRotateLeft] = true;
                break;
            case 1:
                m_PlayerMovement[kRotateRight] = true;
                break;
            
            default:
                break;
            }
        }
        else
        {
            // choose new move action
            switch(rand() % 3)
            {
            case 0:
                m_PlayerMovement[kMoveForward] = true;
                break;
            case 1:
                m_PlayerMovement[kMoveForward] = true;
                break;
            case 2:
                m_PlayerMovement[kIncreaseSpeed] = true;
                m_PlayerMovement[kMoveForward] = true;
                break;
            default:
                break;
            }
        }

        // set next update time
        m_dUpdateTime = g_time.GetCurTime() + 1.0f;
    }
}

/**
* Render traversal for drawing objects
*/
void NPCSphereNode::Render(IDirect3DDevice9* pd3dDevice, const RenderData* rData)
{
    D3DXVECTOR3 pos = GetPosition();
    D3DXMATRIX matWorld;
    D3DXMATRIX matRotate;
    D3DXMatrixTranslation(&matWorld, pos.x, pos.y, pos.z);
    D3DXMatrixRotationYawPitchRoll(&matRotate, GetYawRotation(), GetPitchRotation(), GetRollRotation());
    matWorld = matRotate * matWorld * rData->matWorld;

    // set the world space transform
    pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

    // set vertex declaration
    pd3dDevice->SetVertexDeclaration(m_pCVDeclaration);

    // enable lighting
    rData->EnableD3DLighting(pd3dDevice);

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
