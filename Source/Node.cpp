/*******************************************************************************
* Game Development Project
* Node.cpp
*
* Eric Schwabe
* 2009-11-13
*
* Base node
*
*******************************************************************************/

#include "DXUT.h"
#include "Node.h"

/**
* Constuct base node
*/
Node::Node() :
    m_pStateBlock(NULL)
{}

/**
* Destroy base node (and all children)
*/
Node::~Node()
{
    SAFE_RELEASE(m_pStateBlock);
}

/**
* Initialize node for updating and rendering (including children)
*
* @param pd3dDevice Direct 3D Device
*/
HRESULT Node::Initialize(IDirect3DDevice9* pd3dDevice)
{
    // setup state block
    HRESULT result = pd3dDevice->CreateStateBlock(D3DSBT_ALL, &m_pStateBlock);

    // initialize node
    if( SUCCEEDED(result) )
    {
        result = InitializeNode(pd3dDevice);
    }

	// recurse initialize on children
    if( SUCCEEDED(result) )
    {
	    std::vector<NodeRef>::const_iterator it;

	    for(it = m_vChildNodes.begin(); it != m_vChildNodes.end(); ++it)
	    {
		    result = (*it)->Initialize(pd3dDevice);

            if( FAILED(result) )
                break;
	    }
    }

    return result;
}

/**
* Initialize node for updating and rendering
*
* Derived classes should override this method to perform class specific
* initialization.
*/
HRESULT Node::InitializeNode(IDirect3DDevice9* /*pd3dDevice*/)
{
	return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);
}

/**
* Update traversal for physics, AI, etc. (including children)
*/
void Node::Update(double fTime)
{
    // update node
    UpdateNode(fTime);

	// recurse update on children
	std::vector<NodeRef>::const_iterator it;

	for(it = m_vChildNodes.begin(); it != m_vChildNodes.end(); ++it)
	{
		(*it)->Update(fTime);
	}
}

/**
* Update node
*
* Derived classes should override this method to perform class specific
* update functionality.
*/
void Node::UpdateNode(double fTime)
{}

/**
* Set effect view projection matrix for all children
*/
void Node::SetViewProjection(D3DXMATRIX mat)
{
    // render node
    SetViewProjectionNode(mat);

	// recurse render on children
	std::vector<NodeRef>::const_iterator it;

	for(it = m_vChildNodes.begin(); it != m_vChildNodes.end(); ++it)
	{
		(*it)->SetViewProjection(mat);
	}
}

/**
* Render traversal for drawing objects (including children)
*/
void Node::Render(IDirect3DDevice9* pd3dDevice, D3DXMATRIX rMatWorld)
{
    // capture state
    m_pStateBlock->Capture();

    // render node
    RenderNode(pd3dDevice, rMatWorld);

    // restore state
    m_pStateBlock->Apply();

	// recurse render on children
	std::vector<NodeRef>::const_iterator it;

	for(it = m_vChildNodes.begin(); it != m_vChildNodes.end(); ++it)
	{
		(*it)->Render(pd3dDevice, rMatWorld);
	}
}

/**
* Render traversal for drawing objects (including children)
*
* Derived classes should override this method to perform class specific
* rendering.
*/
void Node::RenderNode(IDirect3DDevice9* pd3dDevice, D3DXMATRIX rMatWorld)
{}

/**
* Add a child node to this node. The node pointer will be deleted automatically
* by this node.
*/
void Node::AddChild(Node* pNode)
{
	m_vChildNodes.push_back(NodeRef(pNode));
}
