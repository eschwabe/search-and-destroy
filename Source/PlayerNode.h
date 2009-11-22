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

        // enable NPC mode
        void SetNPCMode() { m_NPCMode = true; }

        // handle user controls
        LRESULT HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        // modify player position by delta
        void MovePlayerPosition(const D3DXVECTOR3& vPosDelta);

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
            kIncreaseSpeed,
            kMaxMovement,
            kUnknown = 0xFF
        };

        // defines the support player animations
        // note: animation values are specific to tiny model
        enum Animation
        {
            kWait = 3,
            kWalk = 2,
            kRun = 1
        };

        // initialize world node
        HRESULT InitializeNode(IDirect3DDevice9* pd3dDevice);

        // update traversal for physics, AI, etc.
	    void UpdateNode(double fTime);
        void UpdateAnimation(double fTime);

        // render traversal for drawing objects
	    void RenderNode(IDirect3DDevice9*, D3DXMATRIX rMatWorld);

        // setup helpers
        void SetupBoneMatrices(EXTD3DXFRAME *pFrame);

        // render helpers
        void ComputeTransform();
        void UpdateFrameTransforms(EXTD3DXFRAME* pFrame, D3DXMATRIX rMatWorld);
        void UpdateBoneMatricesBuffer(DWORD NumBones);
        void DrawFrame(IDirect3DDevice9* pd3dDevice, EXTD3DXFRAME* pFrame);
        void DrawMeshContainer(IDirect3DDevice9* pd3dDevice, EXTD3DXFRAME* pFrame, EXTD3DXMESHCONTAINER* pMeshContainer);

        // user input helper
        Movement GetPlayerMovement(const UINT&);

        // automatically change player movement
        void AutoPlayerMove(double fTime);

        bool m_NPCMode; // enables automatic player movement

        bool m_PlayerMovement[kMaxMovement];    // player movements currently requested
        
        std::wstring m_sMeshFilename;   // mesh file to load
        float m_fPlayerScale;           // player scaling
        float m_fPlayerYawRotation;     // player rotation (y-axis)
        float m_fPlayerPitchRotation;   // player rotation (z-axis)
        float m_fPlayerRollRotation;    // player rotation (x-axis)
        D3DXVECTOR3 m_vPlayerPos;       // player position
        D3DXVECTOR3 m_vPlayerVelocity;  // player velocity
        D3DXVECTOR3 m_vPlayerAccel;     // player acceleration
        Animation m_ePlayerAnimation;   // player animation
        int m_iPlayerAnimationTrack;    // player animation track

        D3DXMATRIX m_matPlayer;         // player transform matrix

        DWORD m_NumBoneMatrices;        // size of bone matrices buffer
        D3DXMATRIX *m_pBoneMatrices;    // bone matrices for software skinned mesh rendering
        LPD3DXFRAME m_FrameRoot;        // frame root
        LPD3DXEFFECT m_pEffect;         // effects
        bool m_playerSkinInfo;          // true if model has skin info (removes white dot under tiny...)

        LPD3DXANIMATIONCONTROLLER m_AnimationController;    // animation controller

        // prevent copy and assignment
        PlayerNode(const PlayerNode&);
        void operator=(const PlayerNode&);
};
