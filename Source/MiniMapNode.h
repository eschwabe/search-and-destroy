/*******************************************************************************
* Game Development Project
* MiniMapNode.h
*
* Eric Schwabe
* 2010-01-22
*
* Minimap Node
*
*******************************************************************************/

#pragma once
#include "Node.h"
#include "PlayerNode.h"

class MiniMapNode : public Node
{
    public:

        enum PlayerType
        {
            PC,     // human
            NPC     // computer
        };

        // constructor
        MiniMapNode(const LPCWSTR sMapTexture, const LPCWSTR sMapMaskTexture);
        virtual ~MiniMapNode();

        void AddPlayer(const PlayerNode* pPlayer) { m_pPlayer =  pPlayer; }

    protected:

        // load and unload
	    virtual HRESULT InitializeNode(IDirect3DDevice9* pd3dDevice);

        // update node
	    virtual void UpdateNode(double fTime);

        // render traversal for drawing objects (including children)
	    virtual void RenderNode(IDirect3DDevice9* pd3dDevice, D3DXMATRIX rMatWorld);

    private:

        // custom FVF, which describes the custom vertex structure
        static const DWORD D3DFVF_CUSTOMVERTEX = (D3DFVF_XYZRHW | D3DFVF_TEX2 );

        /**
        * Custom vertex type. Specifies a custom vertex that can be written
        * to the verticies buffer for rendering.
        */
	    struct CustomVertex
	    {
            D3DXVECTOR3 vPos;   // transformed, 2D position for the vertex (z unused?)
            float rhw;          // set to 1 (prevent pipeline from transforming vertices)
            float u1,v1;        // texture 1 coordinates
            float u2,v2;        // texture 2 coordinates

            // default constructor
            CustomVertex() :
                vPos(0.0f, 0.0f, 0.0f), rhw(1.0f), u1(0.0f), v1(0.0f), u2(0.0f), v2(0.0f)
            {}

            // initialization constructor
            CustomVertex(D3DXVECTOR3 ivPos, float iu1, float iv1, float iu2, float iv2) :
                vPos(ivPos), rhw(1.0), u1(iu1), v1(iv1), u2(iu2), v2(iv2)
            {}
	    };

        IDirect3DStateBlock9* m_pStateBlock;        // state block

        CustomVertex* m_cvVertices;                 // minimap vertices
        int m_iVertexCount;                         // number of vertices
        int m_iTriangleCount;                       // number of minimap triangles

        std::wstring m_sMapTexture;                 // texture file
        LPDIRECT3DTEXTURE9 m_pMiniMapTexture;       // minimap texture object

        std::wstring m_sMapMaskTexture;             // texture file
        LPDIRECT3DTEXTURE9 m_pMiniMapMaskTexture;   // minimap texture object

        const PlayerNode* m_pPlayer;                // player object

        // prevent copy and assignment
        MiniMapNode(const MiniMapNode&);
        void operator=(const MiniMapNode&);
};
