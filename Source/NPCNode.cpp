/*******************************************************************************
* Game Development Project
* NPCNode.h
*
* Eric Schwabe
* 2009-12-05
*
* NPC Player Node
*
*******************************************************************************/

#include "DXUT.h"
#include "NPCNode.h"

/**
* Constuct NPC node
* Specify player scale, x,y,z coordinates, and x,y,z rotations (in radians)
*/
NPCNode::NPCNode(const std::wstring& sMeshFilename, const float fScale, 
                 const float fX, const float fY, const float fZ, 
                 const float fXRot, const float fYRot, const float fZRot) :
    PlayerNode(sMeshFilename, fScale, fX, fY, fZ, fXRot, fYRot, fZRot),
    m_pEnemyPlayer(NULL),
    m_bEnemyVisible(false),
    m_bEnemyCollision(false),
    m_dNextUpdateTime(0.0f),
    m_dCurrentTime(0.0f)
{
}

/**
* Deconstruct NPC node
*/
NPCNode::~NPCNode()
{
    SAFE_RELEASE(m_lineVertexBuffer);
}

/**
* Initialize NPC node. Sets up data and debug line vertex.
*/
HRESULT NPCNode::InitializeNode(IDirect3DDevice9* pd3dDevice)
{
    // create the vertex buffer
    HRESULT result = pd3dDevice->CreateVertexBuffer( 
        kNumLineVertices*sizeof(CustomVertex),
        0, 
        D3DFVF_CUSTOMVERTEX,
        D3DPOOL_DEFAULT, 
        &m_lineVertexBuffer, 
        NULL);

    // call base player initialize
    if( SUCCEEDED(result) )
    {
        result = PlayerNode::InitializeNode(pd3dDevice);
    }

    return result;
}

/**
* Update traversal for physics, AI, etc.
*/
void NPCNode::UpdateNode(double fTime)
{
    assert(m_pEnemyPlayer);
    assert(m_pWorldQuadList);

    // set npc location
    m_vList[0].vPos = GetPlayerPosition();
    m_vList[0].vPos.y = GetPlayerHeight();

    // set enemy player location
    m_vList[1].vPos = m_pEnemyPlayer->GetPlayerPosition();
    m_vList[1].vPos.y = m_pEnemyPlayer->GetPlayerHeight();

    // check if npc can see player
    CollLineOfSight collLOS;
    bool bEnemyVisibleUpdate = !(collLOS.RunLineOfSightCollision(*m_pWorldQuadList, m_vList[0].vPos, m_vList[1].vPos));
    
    if( bEnemyVisibleUpdate )
    {
        // reset actions, stop movement
        for(int i =0; i < kMaxMovement; i++)
            m_PlayerMovement[i] = false;

        // move towards player if enemy visible
        if( m_bEnemyVisible )
        {
            // check if collision with player
            if( !m_bPlayerCollision )
            {
                // rotate towards player and move forward
                float xPos = m_pEnemyPlayer->GetPlayerPosition().x - GetPlayerPosition().x;
                float zPos = m_pEnemyPlayer->GetPlayerPosition().z - GetPlayerPosition().z;
                m_fPlayerYawRotation = atan2(xPos, zPos);
                m_PlayerMovement[kMoveForward] = true;
            }
            else
            {
                // reset player collision check
                m_bPlayerCollision = false;
            }
        }

        // set debug line color
        m_DebugLineColor = 0xFFFF0000;
    }
    else
    {
        // enemy just become hidden, force auto move update
        if( m_bEnemyVisible == true )
            m_dNextUpdateTime = m_dCurrentTime;

        // enemy not visable, randomly change movements
        AutoPlayerMove(fTime);

        // set debug line color
        m_DebugLineColor = 0xFFFFFF00;
    }

    // update enemy status
    m_bEnemyVisible = bEnemyVisibleUpdate;

    // call base player update
    PlayerNode::UpdateNode(fTime);
}

/**
* Automatically changes the NPC movements.
*/
void NPCNode::AutoPlayerMove(double fTime)
{
    // update time
    m_dCurrentTime += fTime;

    // check if enough time has passed
    if(m_dCurrentTime >= m_dNextUpdateTime)
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
        m_dNextUpdateTime += 1.0;
    }
}

/**
* Render traversal for drawing objects
*/
void NPCNode::RenderNode(IDirect3DDevice9* pd3dDevice, D3DXMATRIX rMatWorld)
{
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
	    pd3dDevice->SetTransform(D3DTS_WORLD, &rMatWorld);

        // turn off D3D lighting, since we are providing our own vertex colors
        pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

        // set vertex type
        pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );

        // set stream source and draw
        pd3dDevice->SetStreamSource( 0, m_lineVertexBuffer, 0, sizeof(CustomVertex) );
        pd3dDevice->DrawPrimitive( D3DPT_LINELIST, 0, kNumLineVertices/2 );
    }

    // call base player render
    PlayerNode::RenderNode(pd3dDevice, rMatWorld);
}
