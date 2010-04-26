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
#include "gameobject.h"

class PlayerBaseNode : public GameObject
{
    public:

        PlayerBaseNode(const D3DXVECTOR3& vInitialPos, objectID id, unsigned int type, char* name);
        virtual ~PlayerBaseNode();

    protected:

        // draw lines
        HRESULT InitializeLines(IDirect3DDevice9* pd3dDevice);
        void DrawLine(IDirect3DDevice9* pd3dDevice, const RenderData* rData);

        // update player position
        void UpdatePlayerPosition();

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
