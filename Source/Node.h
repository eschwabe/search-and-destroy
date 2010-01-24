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

#pragma once
#include <vector>

class Node
{
    public:

        // node reference
        typedef std::tr1::shared_ptr<Node> NodeRef;

        // constructor
        Node();
	    virtual ~Node();

        // load and unload
	    HRESULT Initialize(IDirect3DDevice9* pd3dDevice);

	    // update traversal for physics, AI, etc. (including children)
	    void Update(double fTime);

        // set effect view projection matrix
        void SetViewProjection(D3DXMATRIX mat);

        // render traversal for drawing objects (including children)
	    void Render(IDirect3DDevice9* pd3dDevice, D3DXMATRIX rMatWorld);

        // hierarchy management
	    void AddChild(Node* pNode);
	    int GetNumChildren() { return m_vChildNodes.size(); }
	    NodeRef GetChild(int iChild) { return m_vChildNodes[iChild]; }

    protected:

        /* 
        *  view projection matrix used for effects; only derived nodes that
        *  require this information should use it 
        */
        D3DXMATRIX m_matViewProj;

        // set the node view prjoection matrix
        // derived classes should override this method for alternate functionality
        virtual void SetViewProjectionNode(D3DXMATRIX mat) { m_matViewProj = mat; }


        // load and unload
	    virtual HRESULT InitializeNode(IDirect3DDevice9* pd3dDevice);

        // update node
	    virtual void UpdateNode(double fTime);

        // render traversal for drawing objects (including children)
	    virtual void RenderNode(IDirect3DDevice9* pd3dDevice, D3DXMATRIX rMatWorld);

    private:

        IDirect3DStateBlock9* m_pStateBlock;    // state block

	    std::vector<NodeRef> m_vChildNodes;     // child nodes

        // prevent copy and assignment
        Node(const Node&);
        void operator=(const Node&);
};
