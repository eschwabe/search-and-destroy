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

        // collision events
        void EnvironmentCollisionEvent(const D3DXVECTOR3& vPosDelta);
        void PlayerCollisionEvent() { m_bPlayerCollision = true; }

        // get player information
        D3DXVECTOR3 GetPlayerPosition() const;
        D3DXVECTOR3 GetPlayerVelocity() const;
        float GetPlayerHeight() const;
        float GetPlayerRotation() const;

protected:

        // initialize world node
        virtual HRESULT InitializeNode(IDirect3DDevice9* pd3dDevice);

        // update traversal for physics, AI, etc.
	    virtual void UpdateNode(double fTime);

        // render traversal for drawing objects
	    virtual void RenderNode(IDirect3DDevice9*, const RenderData& rData);

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

        bool m_PlayerMovement[kMaxMovement];    // player movements currently requested
        bool m_bEnvironemntCollision;           // indicates the player collided with the environment
        bool m_bPlayerCollision;                // indicates the player collided with another player

        float m_fPlayerYawRotation;     // player rotation (y-axis)
        float m_fPlayerPitchRotation;   // player rotation (z-axis)
        float m_fPlayerRollRotation;    // player rotation (x-axis)

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
        void UpdateAnimation(double fTime);
        void ComputeTransform();
        void UpdateFrameTransforms(EXTD3DXFRAME* pFrame, D3DXMATRIX rMatWorld);
        void UpdateBoneMatricesBuffer(DWORD NumBones);
        void DrawFrame(IDirect3DDevice9* pd3dDevice, EXTD3DXFRAME* pFrame, const RenderData& rData, const bool bShadowDraw);
        void DrawMeshContainer(IDirect3DDevice9* pd3dDevice, EXTD3DXFRAME* pFrame, EXTD3DXMESHCONTAINER* pMeshContainer, const RenderData& rData, const bool bShadowDraw);

        // user input helper
        Movement GetPlayerMovement(const UINT&);
 
        std::wstring m_sMeshFilename;   // mesh file to load
        float m_fPlayerScale;           // player scaling
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
