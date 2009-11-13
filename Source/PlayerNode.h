/*******************************************************************************
* Game Development Project
* PlayerNode.h
*
* Eric Schwabe
* 2009-11-13
*
* Animated Player Node
*
*******************************************************************************/

#pragma once
#include "Node.h"
#include "MeshHierarchyBuilder.h"

class PlayerNode : public Node
{
    public:

        // constructor
        PlayerNode::PlayerNode(const std::wstring& sMeshFilename, const float fScale, 
                       const float fX, const float fY, const float fZ, 
                       const float fXRot, const float fYRot, const float fZRot);
        virtual ~PlayerNode();

    private:

        // initialize world node
        HRESULT InitializeNode(IDirect3DDevice9* pd3dDevice);

        // update traversal for physics, AI, etc.
	    void UpdateNode(double fTime);

        // render traversal for drawing objects
	    void RenderNode(IDirect3DDevice9*, D3DXMATRIX rMatWorld);

        // setup helper functions
        void SetupBoneMatrices(EXTD3DXFRAME *pFrame);

        // render helper functions
        void UpdateFrameTransforms(EXTD3DXFRAME* pFrame, D3DXMATRIX rMatWorld);
        void UpdateBoneMatricesBuffer(DWORD NumBones);
        void DrawFrame(IDirect3DDevice9* pd3dDevice, EXTD3DXFRAME* pFrame);
        void DrawMeshContainer(IDirect3DDevice9* pd3dDevice, EXTD3DXFRAME* pFrame, EXTD3DXMESHCONTAINER* pMeshContainer);

        std::wstring m_sMeshFilename;   // mesh file to load
        D3DXMATRIX m_matPlayer;         // player transform matrix

        DWORD m_NumBoneMatrices;        // size of bone matrices buffer
        D3DXMATRIXA16 *m_pBoneMatrices; // bone matrices for software skinned mesh rendering
        LPD3DXFRAME m_FrameRoot;        // frame root

        LPD3DXANIMATIONCONTROLLER m_AnimationController;    // animation controller

        // prevent copy and assignment
        PlayerNode(const PlayerNode&);
        void operator=(const PlayerNode&);
};
