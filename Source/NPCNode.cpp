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
    m_pDebugLine(NULL),
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
    SAFE_RELEASE(m_pDebugLine);
}

/**
* Initialize NPC node. Sets up data and debug info.
*/
HRESULT NPCNode::InitializeNode(IDirect3DDevice9* pd3dDevice)
{
    HRESULT result = D3DXCreateLine(pd3dDevice, &m_pDebugLine);

    // create debug line
    if( SUCCEEDED(result) )
    {
        m_pDebugLine->SetWidth(1.0f);

        // call base player initialize
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
    m_vList[0] = GetPlayerPosition();
    m_vList[0].y = GetPlayerHeight();

    // set enemy player location
    m_vList[1] = m_pEnemyPlayer->GetPlayerPosition();
    m_vList[1].y = m_pEnemyPlayer->GetPlayerHeight();

    // check if npc can see player
    CollLineOfSight collLOS;
    bool bEnemyVisibleUpdate = !(collLOS.RunLineOfSightCollision(*m_pWorldQuadList, m_vList[0], m_vList[1]));
    
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
    // render debug line
    m_pDebugLine->DrawTransform(m_vList, 2, &m_matViewProj, m_DebugLineColor);

    // call base player render
    PlayerNode::RenderNode(pd3dDevice, rMatWorld);
}
