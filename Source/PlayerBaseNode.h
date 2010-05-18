/*******************************************************************************
* Game Development Project
* NPCNode.h
*
* Eric Schwabe
* 2010-4-17
*
* Player Base Node (common player functions)
*
*******************************************************************************/

#pragma once
#include "gameobject.h"

class PlayerBaseNode : public GameObject
{
    public:

        PlayerBaseNode(const D3DXVECTOR3& vInitialPos, objectID id, unsigned int type, char* name);
        virtual ~PlayerBaseNode();
        
    protected:

        // defines the supported player movements
        enum PlayerActions
        {
            kRotateLeft = 0,    
            kRotateRight,
            kMoveForward,
            kMoveBackward,
            kIncreaseSpeed,
            kFireProjectile,
            kFireBigProjectile,
            kMaxMovement,
            kUnknown = 0xFF
        };

        bool m_PlayerMovement[kMaxMovement];    // player movements currently requested

    private:

        // prevent copy and assignment
        PlayerBaseNode(const PlayerBaseNode&);
        void operator=(const PlayerBaseNode&);
};
