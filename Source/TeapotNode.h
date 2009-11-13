/*******************************************************************************
* Game Development Project
* TeapotNode.h
*
* Eric Schwabe
* 2009-11-13
*
* Teapot Node
*
*******************************************************************************/

#pragma once
#include "Node.h"

class TeapotNode : public Node
{
    public:

        // constructor
	    TeapotNode();
	    ~TeapotNode();

    private:

        // METHODS

        // initialize world node
        HRESULT InitializeNode(IDirect3DDevice9* pd3dDevice);

        // update traversal for physics, AI, etc.
	    void UpdateNode(double fTime);

		// render traversal for drawing objects
	    void RenderNode(IDirect3DDevice9* pd3dDevice, D3DXMATRIX rMatWorld);

        // DATA
	    ID3DXMesh* pMesh;

        // prevent copy and assignment
        TeapotNode(const TeapotNode&);
        void operator=(const TeapotNode&);
};

