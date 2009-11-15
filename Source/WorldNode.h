/*******************************************************************************
* Game Development Project
* WorldNode.h
*
* Eric Schwabe
* 2009-11-13
*
* World Node (i.e. Level)
*
*******************************************************************************/

#pragma once
#include "Node.h"

/**
* World Node
*/
class WorldNode : public Node
{
    public:

        // constructor
        WorldNode::WorldNode(
            const LPCWSTR sGridFilename, 
            const LPCWSTR sFloorFilename, 
            const LPCWSTR sWallFilename);

	    ~WorldNode();

    private:

	    /**
        * Custom vertex type. Specifies a custom vertex that can be written
        * to the verticies buffer for rendering.
        */
	    struct CustomVertex
	    {
		    FLOAT x, y, z;  // untransformed, 3D position for the vertex
            FLOAT u1,v1;    // texture coordinates

            // default constructor
            CustomVertex() :
                x(0.0f), y(0.0f), z(0.0f), u1(0.0f), v1(0.0f)
            {}

            // initialization constructor
            CustomVertex(float ix, float iy, float iz, float iu1, float iv1) :
                x(ix), y(iy), z(iz), u1(iu1), v1(iv1)
            {}
	    };

        /**
        * Defines the wall sides for a give coordinate location. 
        */
        enum CubeSide
        {
            kTop,       // top plane of cube
            kBottom,    // bottom plane of cube
            kLeft,      // left side
            kRight,     // right side
            kUpper,     // upper side
            kLower      // lower side
        };

        /**
        * Defines the types of tiles available. Vertices are added to the
        * proper buffer based on the type (texture).
        */
        enum TileType
        {
            kFloor,
            kWall
        };

        // METHODS
        HRESULT InitializeNode(IDirect3DDevice9* pd3dDevice);                   // initialize world node
	    void UpdateNode(double fTime);                                          // update traversal for physics, AI, etc.
	    void RenderNode(IDirect3DDevice9* pd3dDevice, D3DXMATRIX rMatWorld);    // render traversal for drawing objects

        // draw cube tile
        void DrawTile(float x, float y, float z, float size, CubeSide side, TileType type);

        // add plane to the appropriate buffer
        void AddPlane(const CustomVertex& v1, const CustomVertex& v2, const CustomVertex& v3, const TileType& type);

        // creates D3D vertex buffers for rendering
        HRESULT WorldNode::CreateRenderBuffer(
            IDirect3DDevice9* pd3dDevice, 
            CustomVertex* pCustomVertices,
            int iVertexCount,
            LPDIRECT3DVERTEXBUFFER9* ppVertexBuffer);

        // checks if a buffer is large enough
        void WorldNode::CheckBufferSize(int iCVCount, int* iCVBufferSize, CustomVertex** buffer);

        // DATA
        std::wstring m_sGridFilename;               // world filename
        std::wstring m_sFloorFilename;              // floor texture filename
        std::wstring m_sWallFilename;               // wall texture filename

        // FLOOR
        LPDIRECT3DTEXTURE9 m_pFloorTexture;
        int m_iFloorTriangleCount;                    // number of triangles
	    LPDIRECT3DVERTEXBUFFER9 m_pFloorVertexBuffer; // vertices buffer

        int m_iFloorCVCount;                   // number of vertices in the buffer
        int m_iFloorCVBufferSize;              // size of custom vertex buffer (in CVs)
        CustomVertex* m_FloorCVBuffer;         // custom vertex buffer

        // WALLS
        LPDIRECT3DTEXTURE9 m_pWallTexture;
        int m_iWallTriangleCount;                    // number of triangles
	    LPDIRECT3DVERTEXBUFFER9 m_pWallVertexBuffer; // vertices buffer

        int m_iWallCVCount;                 // number of vertices in the buffer
        int m_iWallCVBufferSize;            // size of custom vertex buffer  (in CVs)
        CustomVertex* m_WallCVBuffer;       // custom vertex buffer

        // prevent copy and assignment
        WorldNode(const WorldNode&);
        void operator=(const WorldNode&);
};

