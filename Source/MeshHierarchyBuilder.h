/*******************************************************************************
* Game Development Project
* MeshHierarchyBuilder.h
*
* Eric Schwabe
* 2009-11-13
*
* Creates and destorys a mesh hierarchy
*
*******************************************************************************/

#pragma once
#include <d3dx9anim.h>

/**
* Inherits from D3DXFRAME. This represents an animation frame or bone.
*/
struct EXTD3DXFRAME : public D3DXFRAME
{
    D3DXMATRIX CombinedTransformationMatrix; // frame transform matrix
};

/**
* Inherits from D3DXMESHCONTAINER. This represents a mesh object that gets 
* its vertices blended and rendered based on the frame information in 
* its hierarchy.
*/
struct EXTD3DXMESHCONTAINER : public D3DXMESHCONTAINER
{
    LPDIRECT3DTEXTURE9* ppTextures; // array of texture pointers  
	D3DMATERIAL9*       pMaterials; // array of materials

   	ID3DXMesh*          pSkinMesh;  // copy of the skin mesh

    D3DXMATRIX*         pBoneOffsetMatrices;    // bone matrix offsets, one per bone
    D3DXMATRIX**        ppBoneFrameMatrixPtrs;  // array of assocaited frame matrix ptrs
};

/**
* Provides allocation hierarchy interface
* Allocates and deallocates memory structures for mesh containers and frames
*/
class MeshHierarchyBuilder : public ID3DXAllocateHierarchy
{
    public:

        // constructor
        MeshHierarchyBuilder(IDirect3DDevice9* pd3dDevice);
        virtual ~MeshHierarchyBuilder();

        // callback to create a D3DXFRAME extended object and initialize it
        STDMETHOD( CreateFrame )(LPCSTR Name, LPD3DXFRAME *ppNewFrame);

        // callback to create a D3DXMESHCONTAINER extended object and initialize it
        STDMETHOD( CreateMeshContainer )(
            LPCSTR Name, 
            const D3DXMESHDATA* pMeshData, 
            const D3DXMATERIAL* pMaterials, 
            const D3DXEFFECTINSTANCE* pEffectInstances,
            DWORD NumMaterials, 
            const DWORD* pAdjacency, 
            LPD3DXSKININFO pSkinInfo, 
            LPD3DXMESHCONTAINER* ppNewMeshContainer);

        // callback to release a D3DXFRAME extended object
        STDMETHOD( DestroyFrame )(LPD3DXFRAME pFrame);

        // callback to release a D3DXMESHCONTAINER extended object
        STDMETHOD( DestroyMeshContainer )(LPD3DXMESHCONTAINER pMeshContainer);

    private:

        // duplicates a string
        CHAR* StringCopy(const CHAR* pString);

        IDirect3DDevice9* m_pd3dDevice;     // d3d device pointer

        // prevent copy and assignment
        MeshHierarchyBuilder(const MeshHierarchyBuilder&);
        void operator=(const MeshHierarchyBuilder&);
};
