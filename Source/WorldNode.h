//------------------------------------------------------------------------------
// Project: Game Development (2009)
// 
// World Node
//------------------------------------------------------------------------------

#pragma once
#include "Node.h"
#include "WorldFile.h"

//------------------------------------------------------------------------------
// WorldNode Class
//------------------------------------------------------------------------------
class WorldNode : public Node
{
    public:

	    WorldNode(const WorldFile&, IDirect3DDevice9*);
	    ~WorldNode();

        // custom FVF, which describes our custom vertex structure
        #define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

		// Update traversal for physics, AI, etc.
	    virtual void Update(double fTime);

		// Render traversal for drawing objects
	    virtual void Render(IDirect3DDevice9* pd3dDevice, D3DXMATRIX matWorld);

    private:

	    // custom vertex type
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

        // draw verticies into buffer
        void DrawBufferCube( const float& col, const float& row, const float& h, const DWORD& tbcolor, const DWORD& scolor,
            CustomVertex* vertices, int max_vertices, int* current_vertex );

        void DrawBufferTriangle(const CustomVertex& p1, const CustomVertex& p2, const CustomVertex& p3, 
            CustomVertex* vertices, int max_vertices, int* current_vertex );

        int m_triangle_count;           // number of triangles
	    LPDIRECT3DVERTEXBUFFER9 m_pVB;  // vertices buffer
};
