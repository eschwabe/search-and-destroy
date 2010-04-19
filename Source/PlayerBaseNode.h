/*******************************************************************************
* Game Development Project
* NPCNode.h
*
* Eric Schwabe
* 2010-4-17
*
* Player Base Node (common player functions)
*
*******************************************************************************/

#pragma once
#include "Node.h"

class PlayerBaseNode : public Node
{
    public:

        PlayerBaseNode(const D3DXVECTOR3& vInitialPos);
        virtual ~PlayerBaseNode();

        // draw lines
        void DrawLine(IDirect3DDevice9* pd3dDevice, const RenderData& rData);

        // get player information
        D3DXVECTOR3 GetPlayerPosition() const;
        D3DXVECTOR3 GetPlayerVelocity() const;
        float GetPlayerRotation() const;

        // collision events
        virtual void EnvironmentCollisionEvent(const D3DXVECTOR3& vPosDelta);

    protected:

        // initialize player base node
        HRESULT InitializeNode(IDirect3DDevice9* pd3dDevice);

        // update player position
        void PlayerBaseNode::UpdatePlayerPosition(double fTime);

        D3DXVECTOR3 m_vPlayerPos;       // player position
        D3DXVECTOR3 m_vPlayerVelocity;  // player velocity
        D3DXVECTOR3 m_vPlayerAccel;     // player acceleration
        
        float m_fPlayerYawRotation;     // player rotation (y-axis)
        float m_fPlayerPitchRotation;   // player rotation (z-axis)
        float m_fPlayerRollRotation;    // player rotation (x-axis)

        // defines the supported player movements
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

    private:

        // custom FVF, which describes the custom vertex structure
        static const DWORD D3DFVF_CUSTOMVERTEX = (D3DFVF_XYZ|D3DFVF_DIFFUSE);

        /**
        * Custom vertex type. Specifies a custom vertex that can be written
        * to the verticies buffer for rendering.
        */
	    struct CustomVertex
	    {
            D3DXVECTOR3 vPos;   // untransformed, 3D position for the vertex
            DWORD color;        // vertex color
        };

        static const int kNumLineVertices = 2;
        CustomVertex m_vList[kNumLineVertices];     // debug line vertices
        LPDIRECT3DVERTEXBUFFER9 m_lineVertexBuffer; // debug line vertex buffer
        DWORD m_DebugLineColor;                     // debug line color

        // prevent copy and assignment
        PlayerBaseNode(const PlayerBaseNode&);
        void operator=(const PlayerBaseNode&);
};
