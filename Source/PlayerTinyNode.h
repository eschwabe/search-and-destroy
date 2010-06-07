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
#include "PlayerBaseNode.h"
#include "MeshHierarchyBuilder.h"

class PlayerTinyNode : public PlayerBaseNode
{
    public:

        // constructor
        PlayerTinyNode(const std::wstring& sMeshFilename, const D3DXVECTOR3& vInitialPos);
        virtual ~PlayerTinyNode();

        // handle user controls
        virtual LRESULT HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    protected:

        // object methods
        virtual HRESULT Initialize(IDirect3DDevice9* pd3dDevice);
	    virtual void Update();
	    virtual void Render(IDirect3DDevice9*, const RenderData* rData);

    private:

        // defines the support player animations
        // note: animation values are specific to tiny model
        enum Animation
        {
            kWait = 3,
            kWalk = 2,
            kRun = 1
        };

        // setup helpers
        void SetupBoneMatrices(EXTD3DXFRAME *pFrame);

        // render helpers        
        void UpdateAnimation();
        void ComputeTransform();
        void UpdateFrameTransforms(EXTD3DXFRAME* pFrame, D3DXMATRIX rMatWorld);
        void UpdateBoneMatricesBuffer(DWORD NumBones);
        void DrawFrame(IDirect3DDevice9* pd3dDevice, EXTD3DXFRAME* pFrame, const RenderData* rData, const bool bShadowDraw);
        void DrawMeshContainer(IDirect3DDevice9* pd3dDevice, EXTD3DXFRAME* pFrame, EXTD3DXMESHCONTAINER* pMeshContainer, const RenderData* rData, const bool bShadowDraw);

        // user input helper
        PlayerActions GetPlayerMovement(const UINT&);
 
        std::wstring m_sMeshFilename;   // mesh file to load
        
        float m_fPlayerScale;           // player scaling
        Animation m_ePlayerAnimation;   // player animation
        int m_iPlayerAnimationTrack;    // player animation track

        D3DXMATRIX m_matPlayer;         // player transform matrix

        DWORD m_NumBoneMatrices;        // size of bone matrices buffer
        D3DXMATRIX *m_pBoneMatrices;    // bone matrices for software skinned mesh rendering
        LPD3DXFRAME m_FrameRoot;        // frame root
        LPD3DXEFFECT m_pEffect;         // effects

        LPD3DXANIMATIONCONTROLLER m_AnimationController;    // animation controller

        // prevent copy and assignment
        PlayerTinyNode(const PlayerTinyNode&);
        void operator=(const PlayerTinyNode&);
};
