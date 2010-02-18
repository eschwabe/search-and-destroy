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
	    void RenderNode(IDirect3DDevice9*, const RenderData& rData);

    private:

        // custom FVF, which describes the custom vertex structure
        static const DWORD D3DFVF_CUSTOMVERTEX = (D3DFVF_XYZ|D3DFVF_DIFFUSE);

        /**
        * Custom vertex type. Specifies a custom vertex that can be written
        * to the verticies buffer for rendering.
        */
	    struct CustomVertex
	    {
            D3DXVECTOR3 vPos;   // untransformed, 3D position for the vertex
            DWORD color;        // vertex color
        };

        // automatically change player movement
        void AutoPlayerMove(double fTime);

        static const int kNumLineVertices = 2;
        CustomVertex m_vList[kNumLineVertices];     // debug line vertices
        LPDIRECT3DVERTEXBUFFER9 m_lineVertexBuffer; // debug line vertex buffer
        DWORD m_DebugLineColor;                     // debug line color

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
