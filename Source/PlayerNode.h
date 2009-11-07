//------------------------------------------------------------------------------
// Project: Game Development (2009)
// 
// Player Node
//------------------------------------------------------------------------------

#pragma once
#include "Node.h"

// Displays a player object
class PlayerNode : public Node
{
    public:

        // constructor
        PlayerNode(IDirect3DDevice9*);
        virtual ~PlayerNode();

        // update traversal for physics, AI, etc.
	    void Update(double fTime);

        // render traversal for drawing objects
	    void Render(IDirect3DDevice9*, D3DXMATRIX);

    private:

        LPD3DXMESH m_pMesh;                     // mesh object
        D3DMATERIAL9* m_pMeshMaterials;         // materials buffer
        LPDIRECT3DTEXTURE9* m_pMeshTextures;    // textures buffer
        DWORD m_dwNumMaterials;                 // number of mesh materials

};
