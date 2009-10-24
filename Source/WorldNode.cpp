#include "DXUT.h"
//#include "Game.h"
#include "WorldNode.h"

////////////////////////////////////////////////////////////////////////////


WorldNode::WorldNode(IDirect3DDevice9* pd3dDevice) :
	Node(pd3dDevice)
,	m_pVB(0)
{
    // Initialize three vertices for rendering a triangle
    CUSTOMVERTEX Vertices[] =
    {
        { -1.0f,-1.0f, 0.0f, 0xffff0000, },
        {  1.0f,-1.0f, 0.0f, 0xff0000ff, },
        {  0.0f, 1.0f, 0.0f, 0xffffffff, },
    };

    // Create the vertex buffer.
    if( FAILED( pd3dDevice->CreateVertexBuffer( 3*sizeof(CUSTOMVERTEX),
                                                  0, D3DFVF_CUSTOMVERTEX,
                                                  D3DPOOL_DEFAULT, &m_pVB, NULL ) ) )
    {
        return;	// E_FAIL;
    }

    // Fill the vertex buffer.
    VOID* pVertices;
    if( FAILED( m_pVB->Lock( 0, sizeof(Vertices), (void**)&pVertices, 0 ) ) )
        return;	// E_FAIL;
    memcpy( pVertices, Vertices, sizeof(Vertices) );
    m_pVB->Unlock();

    return;	// S_OK;
#if 0
    // Initialize three Vertices for rendering a triangle
    CUSTOMVERTEX Vertices[] =
    {
        { 150.0f,  50.0f, 0.5f, 1.0f, 0xffff0000, }, // x, y, z, rhw, color
        { 250.0f, 250.0f, 0.5f, 1.0f, 0xff00ff00, },
        {  50.0f, 250.0f, 0.5f, 1.0f, 0xff00ffff, },
    };

    // Create the vertex buffer. Here we are allocating enough memory
    // (from the default pool) to hold all our 3 custom Vertices. We also
    // specify the FVF, so the vertex buffer knows what data it contains.
    if( FAILED( gpDisplay->GetDevice()->CreateVertexBuffer( 3*sizeof(CUSTOMVERTEX),
                                                  0, D3DFVF_CUSTOMVERTEX,
                                                  D3DPOOL_DEFAULT, &m_pVB, NULL ) ) )
    {
        return;	// E_FAIL;
    }

    // Now we fill the vertex buffer. To do this, we need to Lock() the VB to
    // gain access to the Vertices. This mechanism is required because vertex
    // buffers may be in device memory.
    VOID* pVertices;
    if( FAILED( m_pVB->Lock( 0, sizeof(Vertices), (void**)&pVertices, 0 ) ) )
        return;	// E_FAIL;
    memcpy( pVertices, Vertices, sizeof(Vertices) );
    m_pVB->Unlock();
#endif
}


WorldNode::~WorldNode()
{
    m_pVB->Release();
}

void
WorldNode::Update(double /* fTime */)
{
}


void
WorldNode::Render(IDirect3DDevice9* pd3dDevice, D3DXMATRIX matWorld)
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
    pd3dDevice->SetStreamSource( 0, m_pVB, 0, sizeof(CUSTOMVERTEX) );
    pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
    pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 1 );

	// Call base class
	Node::Render(pd3dDevice, matWorld);
}
