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

#include "DXUT.h"
#include "TeapotNode.h"

/**
* Constuct teapot node.
*/
TeapotNode::TeapotNode() :
    pMesh(NULL)
{}

/**
* Destroy teapot node.
*/
TeapotNode::~TeapotNode()
{
	pMesh->Release();
}

/**
* Initialize
*/
HRESULT TeapotNode::InitializeNode(IDirect3DDevice9* pd3dDevice)
{
    return D3DXCreateTeapot(pd3dDevice, &pMesh, NULL);
}

/**
* Update
*/
void TeapotNode::UpdateNode(double /* fTime */)
{
}

/**
* Render
*/
void TeapotNode::RenderNode(IDirect3DDevice9* pd3dDevice, D3DXMATRIX rMatWorld)
{
    assert(pMesh);

	pd3dDevice->SetTransform(D3DTS_WORLD, &rMatWorld);
	pMesh->DrawSubset(0);
}
