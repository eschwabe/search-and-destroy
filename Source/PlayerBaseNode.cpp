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
    m_vPlayerPos(vInitialPos),
    m_vPlayerVelocity(0.0f, 0.0f, 0.0f),
    m_vPlayerAccel(0.0f, 0.0f, 0.0f),
    m_fPlayerYawRotation(0.0f),
    m_fPlayerPitchRotation(0.0f),
    m_fPlayerRollRotation(0.0f),
    m_lineVertexBuffer(NULL)
{
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
* Get the current position of the player. Position is the center of the player.
*
* @return vector with player position coordinates
*/
D3DXVECTOR3 PlayerBaseNode::GetPlayerPosition() const
{
    D3DXVECTOR3 pos = m_vPlayerPos;
    pos.y += 0.5f;
    return pos;
}

/**
* Get the current velocity of the player.
*
* @return vector with player velocity
*/
D3DXVECTOR3 PlayerBaseNode::GetPlayerVelocity() const
{
    return m_vPlayerVelocity;
}

/**
* Get the current rotation of the player.
*
* @return player rotation in radians
*/
float PlayerBaseNode::GetPlayerRotation() const
{
    return m_fPlayerYawRotation;
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
    m_vPlayerPos += vPosDelta;
}

/**
* Update player position, velocity and acceleration
*/
void PlayerBaseNode::UpdatePlayerPosition()
{
    const float kMaxSpeed = 3.0f;

    // update player acceleration
    if( m_PlayerMovement[kIncreaseSpeed] )
    {
        // set acceleration
        m_vPlayerAccel.z = 1.5f;
    }

    // update player velocity
    if( m_PlayerMovement[kMoveForward] )
    {
        // set initial velocity if not moving (+tiles per second)
        if(m_vPlayerVelocity.z == 0.0f)
            m_vPlayerVelocity.z = 1.5f;

        // check for max velocity (tiles per second)
        if(m_vPlayerVelocity.z < kMaxSpeed)
            m_vPlayerVelocity += m_vPlayerAccel * g_time.GetElapsedTime();
    }
    else if( m_PlayerMovement[kMoveBackward] )
    {
        // set initial velocity if not moving (-tiles per second)
        if(m_vPlayerVelocity.z == 0.0f)
            m_vPlayerVelocity.z = -1.5f;

        // check for max velocity (tiles per second)
        if(m_vPlayerVelocity.z > -kMaxSpeed)
            m_vPlayerVelocity -= m_vPlayerAccel * g_time.GetElapsedTime();
    }
    else
    {
        // reset velocity and acceleration
        m_vPlayerAccel = D3DXVECTOR3(0,0,0);
        m_vPlayerVelocity = D3DXVECTOR3(0,0,0);
    }

    // compute player position delta
    D3DXVECTOR3 vPosDelta = D3DXVECTOR3(0,0,0);
    vPosDelta += m_vPlayerVelocity * g_time.GetElapsedTime();

    // update player rotation (yaw) (radians)
    if( m_PlayerMovement[kRotateLeft] )
        m_fPlayerYawRotation -= 0.03f;
    if( m_PlayerMovement[kRotateRight] )
        m_fPlayerYawRotation += 0.03f;

    // create movement rotation matrix (yaw only)
    D3DXMATRIX mMoveRot;
    D3DXMatrixRotationYawPitchRoll( &mMoveRot, m_fPlayerYawRotation, 0, 0 );

    // rotate position delta based on yaw
    D3DXVec3TransformCoord( &vPosDelta, &vPosDelta, &mMoveRot );

    // move player position
    m_vPlayerPos += vPosDelta;
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
