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
        PlayerNode(const std::wstring& sMeshFilename, const float fScale, 
                       const float fX, const float fY, const float fZ, 
                       const float fXRot, const float fYRot, const float fZRot);
        virtual ~PlayerNode();

        // handle user controls
        LRESULT HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        // get player information
        D3DXVECTOR3 GetPlayerPosition() const;
        float GetPlayerHeight() const;
        float GetPlayerRotation() const;

    private:

        // defines the supported camera movements
        enum Movement
        {
            kRotateLeft = 0,    
            kRotateRight,
            kMoveForward,
            kMoveBackward,
            kMaxMovement,
            kUnknown = 0xFF
        };

        // initialize world node
        HRESULT InitializeNode(IDirect3DDevice9* pd3dDevice);

        // update traversal for physics, AI, etc.
	    void UpdateNode(double fTime);

        // render traversal for drawing objects
	    void RenderNode(IDirect3DDevice9*, D3DXMATRIX rMatWorld);

        // setup helpers
        void SetupBoneMatrices(EXTD3DXFRAME *pFrame);

        // render helpers
        void UpdateFrameTransforms(EXTD3DXFRAME* pFrame, D3DXMATRIX rMatWorld);
        void UpdateBoneMatricesBuffer(DWORD NumBones);
        void DrawFrame(IDirect3DDevice9* pd3dDevice, EXTD3DXFRAME* pFrame);
        void DrawMeshContainer(IDirect3DDevice9* pd3dDevice, EXTD3DXFRAME* pFrame, EXTD3DXMESHCONTAINER* pMeshContainer);

        // user input helper
        Movement GetPlayerMovement(const UINT&);

        bool m_PlayerMovement[kMaxMovement];    // player movements currently requested
        
        std::wstring m_sMeshFilename;   // mesh file to load
        float m_fPlayerScale;           // player scaling
        float m_fPlayerYawRotation;     // player rotation (y-axis)
        float m_fPlayerPitchRotation;   // player rotation (z-axis)
        float m_fPlayerRollRotation;    // player rotation (x-axis)
        D3DXVECTOR3 vPlayerPos;         // player position

        D3DXMATRIX m_matPlayer;         // player transform matrix

        DWORD m_NumBoneMatrices;        // size of bone matrices buffer
        D3DXMATRIXA16 *m_pBoneMatrices; // bone matrices for software skinned mesh rendering
        LPD3DXFRAME m_FrameRoot;        // frame root

        LPD3DXANIMATIONCONTROLLER m_AnimationController;    // animation controller

        // prevent copy and assignment
        PlayerNode(const PlayerNode&);
        void operator=(const PlayerNode&);
};
