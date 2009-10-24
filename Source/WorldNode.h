#pragma once
#include "Node.h"

////////////////////////////////////////////////////////////////////////////


class WorldNode : public Node
{
public:
	WorldNode(IDirect3DDevice9* pd3dDevice);
	~WorldNode();

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

						// Update traversal for physics, AI, etc.
	virtual void		Update(double fTime);

						// Render traversal for drawing objects
	virtual void		Render(IDirect3DDevice9* pd3dDevice, D3DXMATRIX matWorld);

private:

	// A structure for our custom vertex type
	struct CUSTOMVERTEX
	{
		FLOAT x, y, z;      // The untransformed, 3D position for the vertex
		DWORD color;        // The vertex color
	};

	LPDIRECT3DVERTEXBUFFER9 m_pVB; // Buffer to hold Vertices
};

////////////////////////////////////////////////////////////////////////////
