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
	    WorldNode(const LPCWSTR sFilename);
	    ~WorldNode();

    private:

	    /**
        * Custom vertex type. Specifies a custom vertex that can be written
        * to the verticies buffer for rendering.
        */
	    struct CustomVertex
	    {
		    FLOAT x, y, z;  // untransformed, 3D position for the vertex
		    DWORD color;    // vertex color

            // default constructor
            CustomVertex() :
                x(0.0f), y(0.0f), z(0.0f), color(0x00000000)
            {}

            // initialization constructor
            CustomVertex(const FLOAT& ix, const FLOAT& iy, const FLOAT& iz, const DWORD& icolor) :
                x(ix), y(iy), z(iz), color(icolor)
            {}
	    };

        // METHODS

        // initialize world node
        HRESULT InitializeNode(IDirect3DDevice9* pd3dDevice);

        // update traversal for physics, AI, etc.
	    void UpdateNode(double fTime);

		// render traversal for drawing objects
	    void RenderNode(IDirect3DDevice9* pd3dDevice, D3DXMATRIX rMatWorld);

        // draw verticies into buffer
        void DrawBufferCube(
            const float& col, const float& row, const float& h, const DWORD& tbcolor, const DWORD& scolor,
            const bool& lside, const bool& rside, const bool& uside, const bool& dside,
            CustomVertex* vertices, int max_vertices, int* current_vertex );

        // draw triangles into buffer
        void DrawBufferTriangle(const CustomVertex& p1, const CustomVertex& p2, const CustomVertex& p3, 
            CustomVertex* vertices, int max_vertices, int* current_vertex );

        // DATA
        std::wstring m_sWorldFilename;              // world filename
        int m_iTriangleCount;                       // number of triangles
	    LPDIRECT3DVERTEXBUFFER9 m_pVerticesBuffer;  // vertices buffer

        // prevent copy and assignment
        WorldNode(const WorldNode&);
        void operator=(const WorldNode&);
};

