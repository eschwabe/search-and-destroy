/*******************************************************************************
* Game Development Project
* NPCNode.h
*
* Eric Schwabe
* 2010-4-17
*
* NPC Player Node
*
*******************************************************************************/

#include "DXUT.h"
#include "PlayerBaseNode.h"

/**
* Constuct player base node
*/
PlayerBaseNode::PlayerBaseNode(const D3DXVECTOR3& vInitialPos, objectID id, unsigned int type, char* name) :
    GameObject(id, type, name),
    m_lineVertexBuffer(NULL)
{
    m_vPos = vInitialPos;
        
    // initialize movement to default state
    for(int i =0; i < sizeof(m_PlayerMovement); i++)
        m_PlayerMovement[i] = false;
}

/**
* Constuct player base node
*/
PlayerBaseNode::~PlayerBaseNode()
{
    SAFE_RELEASE(m_lineVertexBuffer);
}

/**
* Initialize player base node
*/
HRESULT PlayerBaseNode::InitializeLines(IDirect3DDevice9* pd3dDevice)
{
    // create the vertex buffer
    HRESULT result = pd3dDevice->CreateVertexBuffer( 
        kNumLineVertices*sizeof(CustomVertex),
        0, 
        D3DFVF_CUSTOMVERTEX,
        D3DPOOL_DEFAULT, 
        &m_lineVertexBuffer, 
        NULL);

    return result;
}

/**
* Notifies the player that it has collided with the environment. The event provides
* the position delta required to resolve the collision. Modifies the player 
* position by the specified delta.
*/
void PlayerBaseNode::EnvironmentCollisionEvent(const D3DXVECTOR3& vPosDelta)
{
    // set collision flag
    m_bEnvironemntCollision = true;

    // move player position
    m_vPos += vPosDelta;
}

/**
* Update player position, velocity and acceleration
*/
void PlayerBaseNode::UpdatePlayerPosition()
{
    // update velocity and determine position delta
    m_fVelocity += m_fAccel * g_time.GetElapsedTime();
    D3DXVECTOR3 vPosDelta = m_vDirection * m_fVelocity * g_time.GetElapsedTime();
    
    // update player position
    m_vPos += vPosDelta;
}

/**
* Draw a line
*/
void PlayerBaseNode::DrawLine(IDirect3DDevice9* pd3dDevice, const RenderData* rData)
{
    // verify buffer was initialized
    assert(m_lineVertexBuffer);

    // update color
    for(int i = 0; i < kNumLineVertices; i++)
    {
        m_vList[i].color = m_DebugLineColor;
    }

    // lock the vertex buffer
    VOID* pVertices;
    HRESULT result = m_lineVertexBuffer->Lock(0, kNumLineVertices*sizeof(CustomVertex), (void**)&pVertices, 0);

    if(SUCCEEDED(result))
    {
        // fill and unlock the buffer
        memcpy(pVertices, m_vList, kNumLineVertices*sizeof(CustomVertex));
        m_lineVertexBuffer->Unlock();

        // set the texture (or unset)
	    pd3dDevice->SetTexture(0, NULL);

        // set the world space transform
        pd3dDevice->SetTransform(D3DTS_WORLD, &rData->matWorld);

        // turn off D3D lighting, since we are providing our own vertex colors
        pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

        // set vertex type
        pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );

        // set stream source and draw
        pd3dDevice->SetStreamSource( 0, m_lineVertexBuffer, 0, sizeof(CustomVertex) );
        pd3dDevice->DrawPrimitive( D3DPT_LINELIST, 0, kNumLineVertices/2 );
    }
}
