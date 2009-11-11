//------------------------------------------------------------------------------
// Project: Game Development (2009)
// 
// Teapot Node
//------------------------------------------------------------------------------

#pragma once
#include "Node.h"

class TeapotNode : public Node
{
    public:

        // constructor
	    TeapotNode();
	    ~TeapotNode();

    private:

        // initialize world node
        HRESULT InitializeNode(IDirect3DDevice9* pd3dDevice);

        // update traversal for physics, AI, etc.
	    void UpdateNode(double fTime);

		// render traversal for drawing objects
	    void RenderNode(IDirect3DDevice9* pd3dDevice, D3DXMATRIX rMatWorld);


	    ID3DXMesh* pMesh;
};
