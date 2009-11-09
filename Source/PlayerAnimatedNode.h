//------------------------------------------------------------------------------
// Project: Game Development (2009)
// 
// Animated Player Node
//------------------------------------------------------------------------------

#pragma once
#include "Node.h"
#include "MeshHierarchyBuilder.h"

class PlayerAnimatedNode : public Node
{
    public:

        // constructor
        PlayerAnimatedNode(IDirect3DDevice9*);
        virtual ~PlayerAnimatedNode();

        // update traversal for physics, AI, etc.
	    void Update(double fTime);

        // render traversal for drawing objects
	    void Render(IDirect3DDevice9*, D3DXMATRIX);

    private:

        // setup helper functions
        void SetupBoneMatrices(EXTD3DXFRAME *pFrame);

        // render helper functions
        void UpdateFrameTransformMatrices(EXTD3DXFRAME* pFrame, D3DXMATRIX ParentMatrix);
        void UpdateBoneMatricesBuffer(DWORD NumBones);
        void DrawFrame(IDirect3DDevice9* pd3dDevice, EXTD3DXFRAME* pFrame);
        void DrawMeshContainer(IDirect3DDevice9* pd3dDevice, EXTD3DXFRAME* pFrame, EXTD3DXMESHCONTAINER* pMeshContainer);

        DWORD m_NumBoneMatrices;        // size of bone matrices buffer
        D3DXMATRIXA16 *m_pBoneMatrices; // bone matrices for software skinned mesh rendering
        LPD3DXFRAME m_FrameRoot;        // frame root

        LPD3DXANIMATIONCONTROLLER m_AnimationController;    // animation controller
        MeshHierarchyBuilder m_HierarchyBuilder;            // hierarchy allocator

        // prevent copy and assignment
        PlayerAnimatedNode(const PlayerAnimatedNode&);
        PlayerAnimatedNode& operator=(const PlayerAnimatedNode&);
};
