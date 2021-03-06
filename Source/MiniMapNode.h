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
#include "gameobject.h"
#include "PlayerBaseNode.h"
#include "WorldNode.h"

class MiniMapNode : public GameObject
{
    public:

        // constructor
        MiniMapNode(const LPCWSTR sMapTexture, 
                    const LPCWSTR sMapMaskTexture, 
                    const LPCWSTR sMapBorderTexture, 
                    const LPCWSTR sPlayerLocTexture, 
                    const LPCWSTR sNPCLocTexture, 
                    const WorldNode* world,
                    const int iMapsize);

        virtual ~MiniMapNode();

    protected:

        // object methods
        virtual HRESULT Initialize(IDirect3DDevice9* pd3dDevice);
	    virtual void Update();
	    virtual void Render(IDirect3DDevice9*, const RenderData* rData);

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


        /**
        * Minimap player tracking data
        */
        struct PlayerMapInfo
        {
            CustomVertex cvPlayerLocVertices[4];            // player location vertices
            int iTriangleCount;                             // number of triangles
            int iVertexCount;                               // number of vertices

            GameObject* pObject;                            // player object

            // default constructor
            PlayerMapInfo() :
                iTriangleCount(2), 
                iVertexCount(4), 
                pObject(NULL)
            {}
        };

        // update player minimap tracking
        void UpdatePlayerTracking();

        // transform texture coordinates from world coordinates
        D3DXVECTOR2 ComputeTextureCoordsFromWorld(const D3DXVECTOR3& vWorldLoc);
        void TransformTextureCoords(const D3DXVECTOR2& vTranslate, const float& fRotation, const int& iVertexCount, CustomVertex* cvVertices);

        // initialize reference vertices
        void InitializeReferenceVertices();


        CustomVertex* m_cvRefVertices;              // screen location vertices
        CustomVertex* m_cvMapVertices;              // minimap vertices
        int m_iVertexCount;                         // number of vertices
        int m_iTriangleCount;                       // number of minimap triangles
        int m_iMapSize;                             // minimap size

        int m_iWinHeight;                           // window height
        int m_iWinWidth;                            // window width

        std::vector<PlayerMapInfo> m_vPlayerMapInfo;// player map info
        const WorldNode* m_WorldNode;               // world node info

        std::wstring m_sMapTexture;                 // minimap texture file
        LPDIRECT3DTEXTURE9 m_pMiniMapTexture;       // minimap texture object

        std::wstring m_sMapMaskTexture;             // minimap mask texture file
        LPDIRECT3DTEXTURE9 m_pMiniMapMaskTexture;   // minimap mask texture object

        std::wstring m_sMapBorderTexture;           // minimap border texture file
        LPDIRECT3DTEXTURE9 m_pMiniMapBorderTexture; // minimap border texture object

        std::wstring m_sPlayerLocTexture;           // player location texture file
        LPDIRECT3DTEXTURE9 m_pPlayerLocTexture;     // player location texture object

        std::wstring m_sNPCLocTexture;              // npc location texture file
        LPDIRECT3DTEXTURE9 m_pNPCLocTexture;        // npc location texture object


        // prevent copy and assignment
        MiniMapNode(const MiniMapNode&);
        void operator=(const MiniMapNode&);
};
