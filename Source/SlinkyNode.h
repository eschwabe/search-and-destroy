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

    private:

        /**
        * Custom vertex type. Specifies a custom vertex that can be written
        * to the verticies buffer for rendering.
        */
	    struct CustomVertex
	    {
            D3DXVECTOR3 vPos;       // untransformed, 3D position for the vertex
            D3DXVECTOR3 vNormal;    // vertex normal
            D3DXCOLOR cDiffuse;     // diffuse color

            // default constructor
            CustomVertex() :
                vPos(0.0f, 0.0f, 0.0f), 
                vNormal(0.0f, 0.0f, 0.0f), 
                cDiffuse(0.0f, 0.0f, 0.0f, 1)
            {}

            // initialization constructor
            CustomVertex(const D3DXVECTOR3& ivPos, const D3DXVECTOR3& ivNormal, const D3DXCOLOR& icDiffuse) :
                vPos(ivPos), 
                vNormal(ivNormal), 
                cDiffuse(icDiffuse)
            {}
	    };

        static const D3DVERTEXELEMENT9 m_sCustomVertexDeclaration[];    // custom vertex structure definition
        LPDIRECT3DVERTEXDECLARATION9 m_pCVDeclaration;                  // custom vertex declaration

        // DATA
        LPD3DXMESH m_pCylMesh;          // cylinder mesh

        CustomVertex* m_CVBuffer;       // custom vertex buffer
        DWORD m_CVBufferSize;           // custom vertex buffer size
        DWORD m_CVTriangleCount;        // custom vertex triangle count

        void* m_CVIndexBuffer;              // custom vertex index buffer
        D3DINDEXBUFFER_DESC m_CVIndexDesc;  // custom vertex index description

};
