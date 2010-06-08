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
    GameObject(id, type, name)
{
    m_vPos = vInitialPos;
    m_vResetPos = vInitialPos;

    ResetPlayerMovement();
}

/**
* Constuct player base node
*/
PlayerBaseNode::~PlayerBaseNode()
{
}

/**
* Resets the player movements
*/
void PlayerBaseNode::ResetPlayerMovement()
{
    // initialize movement to default state
    for(int i =0; i < sizeof(m_PlayerMovement); i++)
        m_PlayerMovement[i] = false;
}