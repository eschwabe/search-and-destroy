/*******************************************************************************
* Game Development Project
* WorldDecalNode.h
*
* Eric Schwabe
* 2010-03-09
*
* World Decals
*
*******************************************************************************/

#pragma once
#include "Node.h"
#include "PlayerBaseNode.h"
#include <list>

class WorldDecalNode : public Node
{
    public:

        // constructor
        WorldDecalNode(const LPCWSTR sDecalFilename);
        virtual ~WorldDecalNode();

        // add new player tracking
        void AddPlayerTracking(const PlayerBaseNode* player);

        // add floor decal
        void AddFloorDecal(const D3DXVECTOR3& vPos);

    protected:

        HRESULT InitializeNode(IDirect3DDevice9* pd3dDevice);                   // initialize world node
	    void UpdateNode(double fTime);                                          // update traversal for physics, AI, etc.
	    void RenderNode(IDirect3DDevice9* pd3dDevice, const RenderData& rData); // render traversal for drawing objects

    private:

	    /**
        * Custom vertex type. Specifies a custom vertex that can be written
        * to the verticies buffer for rendering.
        */
	    struct CustomVertex
	    {
            D3DXVECTOR3 vPos;       // untransformed, 3D position for the vertex
            D3DXVECTOR2 vTexCoord;  // texture coordinates

            // default constructor
            CustomVertex() :
                vPos(0.0f, 0.0f, 0.0f),
                vTexCoord(0.0f, 0.0f)
            {}

            // initialization constructor
            CustomVertex(const D3DXVECTOR3& ivPos, const D3DXVECTOR2& ivTexCoord) :
                vPos(ivPos),
                vTexCoord(ivTexCoord)
            {}
	    };

        static const D3DVERTEXELEMENT9 m_sCustomVertexDeclaration[];    // custom vertex structure definition
        LPDIRECT3DVERTEXDECLARATION9 m_pCVDeclaration;                  // custom vertex declaration

        /**
        * Decal data
        */
        struct Decal
        {
            D3DXVECTOR3 vPos;       // decal quad points
            float fSize;            // size
            double fLife;           // current life (seconds)
            double fTotalLife;      // total life (seconds)

            // default constructor
            Decal() :
                fSize(0.05f),
                fTotalLife(5.0f),
                fLife(0.0f)
            {}
        };

        /**
        * Player data
        */
        struct PlayerData
        {
            const PlayerBaseNode* pPlayer;
            D3DXVECTOR3 vLastDecalPos;

            PlayerData() :
                vLastDecalPos( D3DXVECTOR3(0.0f, 0.0f, 0.0f) )
            {}
        };

        // DATA
        static const DWORD dDecalVertexCount = 6;
        static const DWORD dMaxDecals = 100;
        std::list<Decal> m_DecalList;                   // list of decals
        std::vector<PlayerData> m_PlayerList;           // list of players

        LPDIRECT3DTEXTURE9 m_pDecalTexture;
        std::wstring m_sDecalFilename;

        // prevent copy and assignment
        WorldDecalNode(const WorldDecalNode&);
        void operator=(const WorldDecalNode&);
};
