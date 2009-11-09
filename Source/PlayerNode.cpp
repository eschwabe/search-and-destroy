//------------------------------------------------------------------------------
// Project: Game Development (2009)
// 
// Player Node
// Based on meshes D3D example
//------------------------------------------------------------------------------

#include "DXUT.h"
#include "PlayerNode.h"

//------------------------------------------------------------------------------
// Constuct player node.
//------------------------------------------------------------------------------
PlayerNode::PlayerNode(IDirect3DDevice9* pd3dDevice) :
    Node(pd3dDevice)
{
    LPD3DXBUFFER pD3DXMtrlBuffer;

    // Load the mesh from the specified file
    HRESULT result = D3DXLoadMeshFromX( 
        L"tiger.x",             // [in] filename
        D3DXMESH_SYSTEMMEM,     // [in] options (D3DXMESH)
        pd3dDevice,             // [in] D3DDevice
        NULL,                   // [out] adjacency data
        &pD3DXMtrlBuffer,       // [out] materials buffer
        NULL,                   // [out] effect instances
        &m_dwNumMaterials,      // [out] number of materials
        &m_pMesh );             // [out] LPD3DXMESH pointer (loaded mesh)

    if( SUCCEEDED(result) )
    {
        // extract the material properties and texture names from the pD3DXMtrlBuffer
        D3DXMATERIAL* d3dxMaterials = ( D3DXMATERIAL* )pD3DXMtrlBuffer->GetBufferPointer();

        // allocate mesh material buffer
        m_pMeshMaterials = new D3DMATERIAL9[m_dwNumMaterials];

        // allocate mesh texture buffer
        m_pMeshTextures = new LPDIRECT3DTEXTURE9[m_dwNumMaterials];

        // check each material
        for( DWORD i = 0; i < m_dwNumMaterials; i++ )
        {
            // copy the material
            m_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;

            // set the ambient color for the material (D3DX does not do this)
            m_pMeshMaterials[i].Ambient = m_pMeshMaterials[i].Diffuse;

            // set default texture for material
            m_pMeshTextures[i] = NULL;

            // load texture from file
            if( d3dxMaterials[i].pTextureFilename != NULL &&
                lstrlenA( d3dxMaterials[i].pTextureFilename ) > 0 )
            {
                result = D3DXCreateTextureFromFileA( 
                    pd3dDevice,
                    d3dxMaterials[i].pTextureFilename,
                    &m_pMeshTextures[i] );

                if( FAILED(result) )
                {
                    MessageBox( NULL, L"Could not find texture map", L"UWGame", MB_OK );
                }
            }
        }
    }

    // Done with the material buffer
    pD3DXMtrlBuffer->Release();
}

//------------------------------------------------------------------------------
// Destroy player node.
//------------------------------------------------------------------------------
PlayerNode::~PlayerNode(void)
{
    // materials
    delete [] m_pMeshMaterials;

    // textures
    for( DWORD i = 0; i < m_dwNumMaterials; i++ )
    {
        if( m_pMeshTextures[i] )
            m_pMeshTextures[i]->Release();
    }
    delete[] m_pMeshTextures;

    // mesh
    m_pMesh->Release();
}

//------------------------------------------------------------------------------
// update traversal for physics, AI, etc.
//------------------------------------------------------------------------------
void PlayerNode::Update(double fTime)
{
}

//------------------------------------------------------------------------------
// render traversal for drawing objects
//------------------------------------------------------------------------------
void PlayerNode::Render(IDirect3DDevice9* pd3dDevice, D3DXMATRIX matWorld)
{
    // meshes are divided into subsets, one for each material. Render them in a loop
    for( DWORD i = 0; i < m_dwNumMaterials; i++ )
    {
        // Set the material and texture for this subset
        pd3dDevice->SetMaterial( &m_pMeshMaterials[i] );
        pd3dDevice->SetTexture( 0, m_pMeshTextures[i] );

        // Draw the mesh subset
        m_pMesh->DrawSubset( i );
    }
}
