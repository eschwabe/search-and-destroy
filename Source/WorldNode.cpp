//------------------------------------------------------------------------------
// Project: Game Development (2009)
// 
// World Node
//------------------------------------------------------------------------------

#include "DXUT.h"
#include "WorldNode.h"
//#include "Game.h"

//------------------------------------------------------------------------------
// Constuct world node.
//------------------------------------------------------------------------------
WorldNode::WorldNode(const WorldFile& grid, IDirect3DDevice9* pd3dDevice) :
	Node(pd3dDevice),
	m_pVB(0),
    m_triangle_count(0)
{
    // allocate triangles for size of grid
    // (3 verticies per triangle, 2 triangles per rectangle, 6 rectangles per cube)
    int max_vertices = 3 * 2 * 6 * grid.GetHeight() * grid.GetWidth();
   
    CustomVertex* vertices = new CustomVertex[max_vertices];

    // current vertex in buffer
    int current_vertex = 0;

    // draw cubes
    for(int x = 0; x < grid.GetWidth(); x++)
    {
        for(int y = 0; y < grid.GetHeight(); y++)
        {
            switch(grid(y,x))
            {
                case WorldFile::EMPTY_CELL:
                    // short cube, black color
                    DrawBufferCube((float)x, (float)y, 0.0, 0x00222222, 0x00222222,
                        vertices, max_vertices, &current_vertex);
                    break;

                case WorldFile::OCCUPIED_CELL:
                    // tall cube, blue color
                    DrawBufferCube((float)x, (float)y, 1.0, 0x00257323, 0x0019D812,
                        vertices, max_vertices, &current_vertex);
                    break;

                default:
                    // short cube, white color
                    DrawBufferCube((float)x, (float)y, 0.0, 0xffffffff, 0xffffffff,
                        vertices, max_vertices, &current_vertex);
                    break;
            }
        }
    }

    // create the vertex buffer.
    HRESULT result = pd3dDevice->CreateVertexBuffer( 
        max_vertices*sizeof(CustomVertex),
        0, D3DFVF_CUSTOMVERTEX,
        D3DPOOL_DEFAULT, &m_pVB, NULL);

    if(SUCCEEDED(result))
    {
        // fill the vertex buffer.
        VOID* pVertices;
        result = m_pVB->Lock( 0, max_vertices*sizeof(CustomVertex), (void**)&pVertices, 0 );

        if(SUCCEEDED(result))
        {
            memcpy( pVertices, vertices, max_vertices*sizeof(CustomVertex) );
            m_pVB->Unlock();
        }
    }
}

//------------------------------------------------------------------------------
// Deconstuct world node.
//------------------------------------------------------------------------------
WorldNode::~WorldNode()
{
    if(m_pVB)
        m_pVB->Release();
}

//------------------------------------------------------------------------------
// Draw a cube into the verticies buffer. Draws the cube in the x-z plane at 
// the specified row and column with a set height and color.
//
// @param col cube column location
// @param row cube row location
// @param h cube height
// @param tbcolor top and bottom cube color
// @param scolor side cube color
//------------------------------------------------------------------------------
void WorldNode::DrawBufferCube(
    const float& col, const float& row, const float& h, const DWORD& tbcolor, const DWORD& scolor,
    CustomVertex* vertices, int max_vertices, int* current_vertex )
{
    const float s = 1.0; // side length (NOTE: cannot change unless col and row are scaled)

    // bottom side
    DrawBufferTriangle(
        CustomVertex( col,      0.0,    row,    tbcolor ),
        CustomVertex( col,      0.0,    row+s,  tbcolor ),
        CustomVertex( col+s,    0.0,    row+s,  tbcolor ),
        vertices, max_vertices, current_vertex);
    
    DrawBufferTriangle(
        CustomVertex( col,      0.0,    row,    tbcolor ),
        CustomVertex( col+s,    0.0,    row+s,  tbcolor ),
        CustomVertex( col+s,    0.0,    row,    tbcolor ),
        vertices, max_vertices, current_vertex);

    // top side
    DrawBufferTriangle(
        CustomVertex( col,      h,      row,    tbcolor ),
        CustomVertex( col,      h,      row+s,  tbcolor ),
        CustomVertex( col+s,    h,      row+s,  tbcolor ),
        vertices, max_vertices, current_vertex);
    
    DrawBufferTriangle(
        CustomVertex( col,      h,      row,    tbcolor ),
        CustomVertex( col+s,    h,      row+s,  tbcolor ),
        CustomVertex( col+s,    h,      row,    tbcolor ),
        vertices, max_vertices, current_vertex);

    // left side
    DrawBufferTriangle(
        CustomVertex( col,      0.0,    row,    scolor ),
        CustomVertex( col,      0.0,    row+s,  scolor ),
        CustomVertex( col,      h,      row+s,  scolor ),
        vertices, max_vertices, current_vertex);
    
    DrawBufferTriangle(
        CustomVertex( col,      0.0,    row,    scolor ),
        CustomVertex( col,      h,      row,    scolor ),
        CustomVertex( col,      h,      row+s,  scolor ),
        vertices, max_vertices, current_vertex);

    // right side
    DrawBufferTriangle(
        CustomVertex( col+s,    0.0,    row,    scolor ),
        CustomVertex( col+s,    0.0,    row+s,  scolor ),
        CustomVertex( col+s,    h,      row+s,  scolor ),
        vertices, max_vertices, current_vertex);
    
    DrawBufferTriangle(
        CustomVertex( col+s,    0.0,    row,    scolor ),
        CustomVertex( col+s,    h,      row,    scolor ),
        CustomVertex( col+s,    h,      row+s,  scolor ),
        vertices, max_vertices, current_vertex);

    // up side
    DrawBufferTriangle(
        CustomVertex( col,      0.0,    row+s,  scolor ),
        CustomVertex( col,      h,      row+s,  scolor ),
        CustomVertex( col+s,    0.0,    row+s,  scolor ),
        vertices, max_vertices, current_vertex);
    
    DrawBufferTriangle(
        CustomVertex( col+s,    0.0,    row+s,  scolor ),
        CustomVertex( col+s,    h,      row+s,  scolor ),
        CustomVertex( col,      h,      row+s,  scolor ),
        vertices, max_vertices, current_vertex);

    // down side
    DrawBufferTriangle(
        CustomVertex( col,      0.0,    row,    scolor ),
        CustomVertex( col,      h,      row,    scolor ),
        CustomVertex( col+s,    0.0,    row,    scolor ),
        vertices, max_vertices, current_vertex);
    
    DrawBufferTriangle(
        CustomVertex( col+s,    0.0,    row,    scolor ),
        CustomVertex( col+s,    h,      row,    scolor ),
        CustomVertex( col,      h,      row,    scolor ),
        vertices, max_vertices, current_vertex);
}

//------------------------------------------------------------------------------
// Draw a triangle into the verticies buffer. Draws a triagle with the
// three verticies. Updates number of triangles for rendering.
//------------------------------------------------------------------------------
void WorldNode::DrawBufferTriangle(
    const CustomVertex& p1, const CustomVertex& p2, const CustomVertex& p3, 
    CustomVertex* vertices, int max_vertices, int* current_vertex )
{
    if((*current_vertex) < max_vertices)
        vertices[(*current_vertex)++] = p1;
    if((*current_vertex) < max_vertices)
        vertices[(*current_vertex)++] = p2;
    if((*current_vertex) < max_vertices)
    {
        vertices[(*current_vertex)++] = p3;
        m_triangle_count++;
    }
}

//------------------------------------------------------------------------------
// Update world node.
//------------------------------------------------------------------------------
void WorldNode::Update(double /* fTime */)
{
}

//------------------------------------------------------------------------------
// Render world node.
//------------------------------------------------------------------------------
void WorldNode::Render(IDirect3DDevice9* pd3dDevice, D3DXMATRIX matWorld)
{
	// Set the world space transform
	pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

	// Set the texture
	pd3dDevice->SetTexture(0, NULL);

	// Turn off culling
	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    // Turn off D3D lighting, since we are providing our own vertex colors
    pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

    // Draw the triangles in the vertex buffer. This is broken into a few
    // steps. We are passing the Vertices down a "stream", so first we need
    // to specify the source of that stream, which is our vertex buffer. Then
    // we need to let D3D know what vertex shader to use. Full, custom vertex
    // shaders are an advanced topic, but in most cases the vertex shader is
    // just the FVF, so that D3D knows what type of Vertices we are dealing
    // with. Finally, we call DrawPrimitive() which does the actual rendering
    // of our geometry (in this case, just one triangle).
    pd3dDevice->SetStreamSource( 0, m_pVB, 0, sizeof(CustomVertex) );
    pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
    pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, m_triangle_count );

	// Call base class
	Node::Render(pd3dDevice, matWorld);
}
