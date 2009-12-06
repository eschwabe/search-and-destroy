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

#pragma once
#include "PlayerNode.h"
#include "Collision.h"

class NPCNode : public PlayerNode
{
    public:

        // constructor
        NPCNode(const std::wstring& sMeshFilename, const float fScale, 
                const float fX, const float fY, const float fZ, 
                const float fXRot, const float fYRot, const float fZRot);
        virtual ~NPCNode(void);

        void SetEnemyPlayer(const PlayerNode* pPlayer) { m_pEnemyPlayer = pPlayer; }
        void SetWorldQuadList(const VecCollQuad* vQuadList) { m_pWorldQuadList = vQuadList; }

    protected:

        // initialize world node
        HRESULT InitializeNode(IDirect3DDevice9* pd3dDevice);

        // update traversal for physics, AI, etc.
        void UpdateNode(double fTime);

        // render traversal for drawing objects
	    void RenderNode(IDirect3DDevice9*, D3DXMATRIX rMatWorld);

    private:

        // automatically change player movement
        void AutoPlayerMove(double fTime);

        ID3DXLine*  m_pDebugLine;               // debug line
        D3DXVECTOR3 m_vList[2];                 // debug line vertices
        DWORD m_DebugLineColor;                 // debug line color

        const PlayerNode* m_pEnemyPlayer;       // enemy player
        const VecCollQuad* m_pWorldQuadList;    // world quad list

        bool m_bEnemyVisible;       // reset NPC movement
        bool m_bEnemyCollision;     // collided with enemy player
    
        double m_dNextUpdateTime;   // next update time
        double m_dCurrentTime;      // current time


        // prevent copy and assignment
        NPCNode(const NPCNode&);
        void operator=(const NPCNode&);
};
