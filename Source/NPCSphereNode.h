/*******************************************************************************
* Game Development Project
* NPCNode.h
*
* Eric Schwabe
* 2009-12-05
*
* NPC Player Node
*
*******************************************************************************/

#pragma once
#include "PlayerBaseNode.h"
#include "Collision.h"

class NPCSphereNode : public PlayerBaseNode
{
    public:

        // constructor
        NPCSphereNode(const D3DXVECTOR3& vInitialPos);
        virtual ~NPCSphereNode();
    
    protected:

        // object methods
        virtual HRESULT Initialize(IDirect3DDevice9* pd3dDevice);
	    virtual void Update();
	    virtual void Render(IDirect3DDevice9*, const RenderData* rData);

    private:

        /**
        * Custom vertex type. Specifies a custom vertex that can be written
        * to the verticies buffer for rendering.
        */
	    struct CustomVertex
	    {
            D3DXVECTOR3 vPos;           // untransformed, 3D position for the vertex
            D3DXVECTOR3 vNormal;        // vertex normal
            D3DCOLOR cDiffuse;          // diffuse color

            // default constructor
            CustomVertex() {}

            // constructor
            CustomVertex(const D3DXVECTOR3& ivPos, const D3DXVECTOR3& ivNormal, const D3DCOLOR& icDiffuse) :
                vPos(ivPos), 
                vNormal(ivNormal), 
                cDiffuse(icDiffuse)
            {}
	    };

        static const D3DVERTEXELEMENT9 m_sCustomVertexDeclaration[];    // custom vertex structure definition
        LPDIRECT3DVERTEXDECLARATION9 m_pCVDeclaration;                  // custom vertex declaration

        LPD3DXMESH m_pSphereMesh;       // sphere mesh data

        CustomVertex* m_CVBuffer;       // custom vertex buffer
        DWORD m_CVBufferSize;           // custom vertex buffer size
        DWORD m_CVTriangleCount;        // custom vertex triangle count

        void* m_CVIndexBuffer;              // custom vertex index buffer
        D3DINDEXBUFFER_DESC m_CVIndexDesc;  // custom vertex index description


        // automatically change player movement
        void AutoPlayerMove();
    
        float m_dUpdateTime;   // update time

        // prevent copy and assignment
        NPCSphereNode(const NPCSphereNode&);
        void operator=(const NPCSphereNode&);
};
