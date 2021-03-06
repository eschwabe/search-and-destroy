/*******************************************************************************
* Game Development Project
* MeshHierarchyBuilder.cpp
*
* Eric Schwabe
* 2009-11-13
*
* Creates and destorys a mesh hierarchy
*
*******************************************************************************/

#include "DXUT.h"

#include <string>
#include "MeshHierarchyBuilder.h"
#include "DXUT/SDKmisc.h"

/**
* Constuct player node.
*/
MeshHierarchyBuilder::MeshHierarchyBuilder(IDirect3DDevice9* pd3dDevice)
{
    m_pd3dDevice = pd3dDevice;
}

/**
* Destroy player node.
*/
MeshHierarchyBuilder::~MeshHierarchyBuilder()
{
}

/**
* Callback to create a D3DXFRAME extended object and initialize it
* called by D3DX during the loading of a mesh hierarchy. The app can
* customize its behavior. At a minimum, the app should allocate a
* D3DXFRAME or a child of it and fill in the Name member.
*/
HRESULT MeshHierarchyBuilder::CreateFrame(const LPCSTR Name, LPD3DXFRAME* ppNewFrame )
{
    // allocate frame
    EXTD3DXFRAME* pFrame = new EXTD3DXFRAME;

    // zero frame memory
    ZeroMemory(pFrame, sizeof(EXTD3DXFRAME));

    // set frame name
    if(Name && strlen(Name) > 0)
        pFrame->Name = StringCopy((CHAR*)Name);
    else
        pFrame->Name = StringCopy("<NoName>");

    // return new frame
    *ppNewFrame = pFrame;
        
    return S_OK;
}

/**
* Callback to create a D3DXMESHCONTAINER extended object and initialize it
* called by D3DX during the loading of a mesh hierarchy. At a minumum,
* the app should allocate a D3DXMESHCONTAINER or a child of it and fill
* in the members based on the parameters here. The app can further
* customize the allocation behavior here.
*/
HRESULT MeshHierarchyBuilder::CreateMeshContainer(
    LPCSTR Name, 
    const D3DXMESHDATA* pMeshData, 
    const D3DXMATERIAL* pMaterials, 
    const D3DXEFFECTINSTANCE* pEffectInstances,
    DWORD NumMaterials, 
    const DWORD* pAdjacency, 
    LPD3DXSKININFO pSkinInfo, 
    LPD3DXMESHCONTAINER* ppNewMeshContainer)
{
    assert(m_pd3dDevice);

    HRESULT result = S_OK;

    // create and initialize a mesh container structure to fill
	EXTD3DXMESHCONTAINER* pMeshContainer = new EXTD3DXMESHCONTAINER;
	ZeroMemory(pMeshContainer, sizeof(EXTD3DXMESHCONTAINER));

	// set mesh name
	if(Name)
		pMeshContainer->Name = StringCopy((CHAR*)(Name));
	else
		pMeshContainer->Name = StringCopy("<NoName>");
	
	// adjacency data - holds information about triangle adjacency, required by the ID3DMESH object
	DWORD dwFaces = pMeshData->pMesh->GetNumFaces();
	pMeshContainer->pAdjacency = new DWORD[dwFaces*3];
	memcpy(pMeshContainer->pAdjacency, pAdjacency, sizeof(DWORD)*dwFaces*3);

	// copy mesh data
    pMeshContainer->MeshData.Type = pMeshData->Type;
	pMeshContainer->MeshData.pMesh = pMeshData->pMesh;
	pMeshContainer->MeshData.pMesh->AddRef();

	// create material and texture arrays
	pMeshContainer->NumMaterials = max(NumMaterials, 1);
    pMeshContainer->pEffects = NULL;
	pMeshContainer->pMaterials = new D3DMATERIAL9[pMeshContainer->NumMaterials];
	pMeshContainer->ppTextures = new LPDIRECT3DTEXTURE9[pMeshContainer->NumMaterials];

    // clear texture memory
	ZeroMemory(pMeshContainer->ppTextures, sizeof(LPDIRECT3DTEXTURE9)*pMeshContainer->NumMaterials);

    // default material (if not provided by mesh)
	ZeroMemory(&pMeshContainer->pMaterials[0], sizeof(D3DMATERIAL9));
    pMeshContainer->pMaterials[0].Diffuse.r = 0.5f;
    pMeshContainer->pMaterials[0].Diffuse.g = 0.5f;
    pMeshContainer->pMaterials[0].Diffuse.b = 0.5f;
    pMeshContainer->pMaterials[0].Specular = pMeshContainer->pMaterials[0].Diffuse;
	pMeshContainer->ppTextures[0] = 0;

    // load all the textures and copy the materials over
	for(DWORD i = 0; i < NumMaterials; ++i)
	{
        // set material
		pMeshContainer->ppTextures[i] = 0;	
		pMeshContainer->pMaterials[i] = pMaterials[i].MatD3D;

		if(pMaterials[i].pTextureFilename)
		{
			// convert filename and find file
            WCHAR wsNewPath[ MAX_PATH ];
            WCHAR wsTextureFilename[ MAX_PATH ];

            if( MultiByteToWideChar(CP_ACP, 0, pMaterials[i].pTextureFilename, -1, wsTextureFilename, sizeof(wsTextureFilename)) == 0 )
                result = E_FAIL;
            
            if( SUCCEEDED(result) )
                result = DXUTFindDXSDKMediaFileCch(wsNewPath, sizeof(wsNewPath), wsTextureFilename);

            // create texture
            if( SUCCEEDED(result) )
		        result = D3DXCreateTextureFromFile(
                    m_pd3dDevice, 
                    wsNewPath,
			        &pMeshContainer->ppTextures[i]);
            
            if( FAILED(result) )
                break;
		}
	}

    // copy skin data associated with the mesh
	if( SUCCEEDED(result) && pSkinInfo)
	{
		// save off the skininfo
	    pMeshContainer->pSkinInfo = pSkinInfo;
	    pSkinInfo->AddRef();

        // duplicate mesh data for setting vertex buffer
        D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE];
	    if(FAILED(pMeshContainer->MeshData.pMesh->GetDeclaration(Declaration)))
			return E_FAIL;

        // Need an array of offset matrices to move the vertices from the figure space to the bone's space
	    UINT NumBones = pSkinInfo->GetNumBones();
	    pMeshContainer->pBoneOffsetMatrices = new D3DXMATRIX[NumBones];

		// Create the arrays for the bones and the frame matrices
		pMeshContainer->ppBoneFrameMatrixPtrs = new D3DXMATRIX*[NumBones];

	    // get each of the bone offset matrices so that we don't need to get them later
	    for (DWORD i = 0; i < NumBones; i++)
	        pMeshContainer->pBoneOffsetMatrices[i] = *(pMeshContainer->pSkinInfo->GetBoneOffsetMatrix(i));

        // convert to indexed blended mesh
        pMeshContainer->dwNumPaletteEntries = NumBones;

        result = pMeshContainer->pSkinInfo->ConvertToIndexedBlendedMesh( 
            pMeshContainer->MeshData.pMesh,
            D3DXMESH_MANAGED | D3DXMESHOPT_VERTEXCACHE,
            pMeshContainer->dwNumPaletteEntries,
            pMeshContainer->pAdjacency,
            NULL,
            NULL,
            NULL,
            &pMeshContainer->dwNumInfl,
            &pMeshContainer->dwNumAttributeGroups,
            &pMeshContainer->pBoneCombinationBuf,
            &pMeshContainer->pSkinMesh );

        // ensure the proper vertex format for the mesh
        if( SUCCEEDED(result) )
        {
            DWORD dwOldFVF = pMeshContainer->pSkinMesh->GetFVF();
            DWORD dwNewFVF = ( dwOldFVF & D3DFVF_POSITION_MASK ) | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_LASTBETA_UBYTE4;
            if( dwNewFVF != dwOldFVF )
            {
                LPD3DXMESH pMesh;
                result = pMeshContainer->pSkinMesh->CloneMeshFVF( 
                    pMeshContainer->pSkinMesh->GetOptions(),
                    dwNewFVF,
                    m_pd3dDevice,
                    &pMesh );
                
                if( SUCCEEDED(result) )
                {
                    pMeshContainer->pSkinMesh->Release();
                    pMeshContainer->pSkinMesh = pMesh;

                    // if the loaded mesh didn't contain normals, compute them here
                    if( ! ( dwOldFVF & D3DFVF_NORMAL ) )
                    {
                        result = D3DXComputeNormals( pMeshContainer->pSkinMesh, NULL );
                    }
                }
            }

            // Interpret the UBYTE4 as a D3DCOLOR.
            // The GeForce3 doesn't support the UBYTE4 decl type.  So, we convert any
            // blend indices to a D3DCOLOR semantic, and later in the shader convert
            // it back using the D3DCOLORtoUBYTE4() intrinsic.  Note that we don't
            // convert any data, just the declaration.
            D3DVERTEXELEMENT9 pDecl[ MAX_FVF_DECL_SIZE ];
            D3DVERTEXELEMENT9 * pDeclCur;
            result = pMeshContainer->pSkinMesh->GetDeclaration( pDecl );

            if( SUCCEEDED(result) )
            {
                pDeclCur = pDecl;
                while( pDeclCur->Stream != 0xff )
                {
                    if( ( pDeclCur->Usage == D3DDECLUSAGE_BLENDINDICES ) && ( pDeclCur->UsageIndex == 0 ) )
                        pDeclCur->Type = D3DDECLTYPE_D3DCOLOR;
                    pDeclCur++;
                }

                result = pMeshContainer->pSkinMesh->UpdateSemantics( pDecl );
            }
        }  
    }

	// set the output mesh container pointer to the new one
    if( SUCCEEDED(result) )
    {
	    *ppNewMeshContainer = pMeshContainer;    
    }
    else
    {
        *ppNewMeshContainer = NULL;
        DestroyMeshContainer(pMeshContainer);
    }

	return result;
}

/**
* Callback to release a D3DXFRAME extended object
* called by D3DX during the release of a mesh hierarchy. Here we should
* free all resources allocated in CreateFrame().
*/
HRESULT MeshHierarchyBuilder::DestroyFrame(LPD3DXFRAME pFrameToFree )
{
    EXTD3DXFRAME* pFrame = (EXTD3DXFRAME*)pFrameToFree;

    SAFE_DELETE_ARRAY(pFrame->Name);
    SAFE_DELETE(pFrame);

    return S_OK;
}

/**
* Callback to release a D3DXMESHCONTAINER extended object
* called by D3DX during the release of a mesh hierarchy. Here we should
* free all resources allocated in CreateMeshContainer().
*/
HRESULT MeshHierarchyBuilder::DestroyMeshContainer(LPD3DXMESHCONTAINER pMeshContainerToFree )
{
    EXTD3DXMESHCONTAINER* pMeshContainer = (EXTD3DXMESHCONTAINER*)pMeshContainerToFree;

    // free mesh container contents
    SAFE_DELETE_ARRAY(pMeshContainer->Name);
    SAFE_DELETE_ARRAY(pMeshContainer->pAdjacency);
    SAFE_DELETE_ARRAY(pMeshContainer->pMaterials);

    // release textures
    for(DWORD i = 0; i < pMeshContainer->NumMaterials; ++i)
	{
		SAFE_RELEASE(pMeshContainer->ppTextures[i]);
	}
    SAFE_DELETE_ARRAY(pMeshContainer->ppTextures);

    // release meshdata
    SAFE_RELEASE(pMeshContainer->MeshData.pMesh);
    SAFE_RELEASE(pMeshContainer->pSkinMesh);

    // release skin info
	SAFE_RELEASE(pMeshContainer->pSkinInfo);

    // release bone combination buffer
    SAFE_RELEASE(pMeshContainer->pBoneCombinationBuf);

    // release matrices
    SAFE_DELETE_ARRAY(pMeshContainer->pBoneOffsetMatrices);
    SAFE_DELETE_ARRAY(pMeshContainer->ppBoneFrameMatrixPtrs);

    // free mesh container
    SAFE_DELETE(pMeshContainer);

    return S_OK;
}

/**
* Duplicate input string. Allocates a new string that must be freed by the caller.
*/
CHAR* MeshHierarchyBuilder::StringCopy(const CHAR* pString)
{
    DWORD dwLen = (DWORD)strlen(pString) + 1;
    CHAR* pNewString = new CHAR[dwLen];
    strcpy_s(pNewString, dwLen, pString);
    return pNewString;
}
