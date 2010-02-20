/*******************************************************************************
* Game Development Project
* SlinkyNode.h
*
* Eric Schwabe
* 2009-02-20
*
* Slinky Node
*
*******************************************************************************/

#pragma once
#include "Node.h"

class SlinkyNode : public Node
{
    public:

        // constructor
        SlinkyNode(void);
        virtual ~SlinkyNode(void);

    protected:

        // initialize node
	    virtual HRESULT InitializeNode(IDirect3DDevice9* pd3dDevice);

        // update node
	    virtual void UpdateNode(double fTime);

        // render traversal for drawing objects (including children)
	    virtual void RenderNode(IDirect3DDevice9* pd3dDevice, const RenderData& rData);

};
