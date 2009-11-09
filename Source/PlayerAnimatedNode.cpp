//------------------------------------------------------------------------------
// Project: Game Development (2009)
// 
// Animated Player Node
//------------------------------------------------------------------------------

#include "DXUT.h"
#include "PlayerAnimatedNode.h"

//------------------------------------------------------------------------------
// Constuct player node
//------------------------------------------------------------------------------
PlayerAnimatedNode::PlayerAnimatedNode(IDirect3DDevice9* pd3dDevice) :
    Node(pd3dDevice),
    m_HierarchyBuilder(pd3dDevice),
    m_NumBoneMatrices(0),
    m_pBoneMatrices(NULL)
{
    // TODO: errors?
    HRESULT result = D3DXLoadMeshHierarchyFromX( 
        //L"tiger.x",
        //L"bones_all.x",
        L"tiny_4anim.x",                 // [in] filename
        0,                          // [in] options (D3DXMESH)
        pd3dDevice,                 // [in] D3DDevice
        &m_HierarchyBuilder,        // [in] mesh/frame memory allocater
        NULL,                       // [in] user data 
        &m_FrameRoot,               // [out] top of frame hierarchy
        &m_AnimationController);    // [out] animation controller

    if( FAILED(result) )
    {
        MessageBox( NULL, L"Could not find texture map", L"UWGame", MB_OK );
    }

    //if(m_AnimationController)
    //{
    //    // get and set animation set
	   // LPD3DXANIMATIONSET set;
	   // m_AnimationController->GetAnimationSet(1, &set);	
	   // m_AnimationController->SetTrackAnimationSet( 0, set );
    //    set->Release();	
    //}

    // setup bone frame matrix pointers
    SetupBoneMatrices((EXTD3DXFRAME*)m_FrameRoot);
}

//------------------------------------------------------------------------------
// Destroy player node
//------------------------------------------------------------------------------
PlayerAnimatedNode::~PlayerAnimatedNode()
{
    // deallocate bone matrices buffer (if allocated)
    if(m_pBoneMatrices)
        delete [] m_pBoneMatrices;

    // deallocate controller
    if(m_AnimationController)
		m_AnimationController->Release();

    // deallocate frame
    D3DXFrameDestroy(m_FrameRoot, &m_HierarchyBuilder);
}

//------------------------------------------------------------------------------
// setup mesh container bone matrices pointers
//------------------------------------------------------------------------------
void PlayerAnimatedNode::SetupBoneMatrices(EXTD3DXFRAME *pFrame)
{
	// Cast to our extended structure first
	EXTD3DXMESHCONTAINER* pMesh = (EXTD3DXMESHCONTAINER*)pFrame->pMeshContainer;

	// If this frame has a mesh
	if(pMesh)
	{	
		// if there is skin info, then setup the bone matrices
		if(pMesh->pSkinInfo && pMesh->MeshData.pMesh)
		{
			// For each bone work out its matrix
			for(DWORD i = 0; i < pMesh->pSkinInfo->GetNumBones(); i++)
			{   
				// Find the frame containing the bone
				EXTD3DXFRAME* pTempFrame = (EXTD3DXFRAME*)D3DXFrameFind(m_FrameRoot, pMesh->pSkinInfo->GetBoneName(i));

				// set the bone part - point it at the transformation matrix
				pMesh->ppBoneFrameMatrixPtrs[i] = &pTempFrame->CombinedTransformationMatrix;
			}
		}
	}

	// Pass on to sibblings
	if(pFrame->pFrameSibling)
		SetupBoneMatrices((EXTD3DXFRAME*)pFrame->pFrameSibling);

	// Pass on to children
	if(pFrame->pFrameFirstChild)
		SetupBoneMatrices((EXTD3DXFRAME*)pFrame->pFrameFirstChild);
}


//------------------------------------------------------------------------------
// update traversal for physics, AI, etc.
//------------------------------------------------------------------------------
void PlayerAnimatedNode::Update(double fTime)
{
    if(m_AnimationController)
        m_AnimationController->AdvanceTime(fTime, NULL);
}

//------------------------------------------------------------------------------
// render traversal for drawing objects
//------------------------------------------------------------------------------
void PlayerAnimatedNode::Render(IDirect3DDevice9* pd3dDevice, D3DXMATRIX matWorld)
{
    // only for tiny!
    D3DXMATRIX mx;
    float fScale = 1.0f/100.0f;
    D3DXMatrixScaling( &mx, fScale, fScale, fScale );
    D3DXMatrixMultiply(&matWorld, &mx, &matWorld);
    D3DXMatrixRotationX( &mx, -D3DX_PI / 2.0f );
    D3DXMatrixMultiply( &matWorld, &matWorld, &mx );
    D3DXMatrixRotationY( &mx, D3DX_PI / 2.0f );
    D3DXMatrixMultiply( &matWorld, &matWorld, &mx );

    // update frame transform matrices
    UpdateFrameTransformMatrices((EXTD3DXFRAME*)m_FrameRoot, matWorld);

    // recursively draw the root frame
    DrawFrame(pd3dDevice, (EXTD3DXFRAME*)m_FrameRoot);
}

//------------------------------------------------------------------------------
// recursively update frame transform matrices
//------------------------------------------------------------------------------
void PlayerAnimatedNode::UpdateFrameTransformMatrices(EXTD3DXFRAME* pFrame, D3DXMATRIX ParentMatrix)
{
    D3DXMatrixMultiply( &pFrame->CombinedTransformationMatrix, &pFrame->TransformationMatrix, &ParentMatrix );

    // recurse for frame sibblings
    if(pFrame->pFrameSibling)
        UpdateFrameTransformMatrices((EXTD3DXFRAME*)pFrame->pFrameSibling, ParentMatrix);

    // recurse for frame children
    if(pFrame->pFrameFirstChild)
        UpdateFrameTransformMatrices((EXTD3DXFRAME*)pFrame->pFrameFirstChild, pFrame->CombinedTransformationMatrix);
}

//------------------------------------------------------------------------------
// recursively draw the frame
//------------------------------------------------------------------------------
void PlayerAnimatedNode::DrawFrame(IDirect3DDevice9* pd3dDevice, EXTD3DXFRAME* pFrame)
{
	// draw all mesh containers in this frame
    EXTD3DXMESHCONTAINER* pMeshContainer = (EXTD3DXMESHCONTAINER*)pFrame->pMeshContainer;

    // draw mesh containers
    while(pMeshContainer)
    {
        DrawMeshContainer(pd3dDevice, pFrame, pMeshContainer);
        pMeshContainer = (EXTD3DXMESHCONTAINER*)pMeshContainer->pNextMeshContainer;
    }

	// recurse for frame sibblings
    if (pFrame->pFrameSibling)
        DrawFrame(pd3dDevice, (EXTD3DXFRAME*)pFrame->pFrameSibling);

    // recurse for frame children
	if (pFrame->pFrameFirstChild)
        DrawFrame(pd3dDevice, (EXTD3DXFRAME*)pFrame->pFrameFirstChild);
}

//------------------------------------------------------------------------------
// draw mesh container (standard mesh or skinned mesh)
//------------------------------------------------------------------------------
void PlayerAnimatedNode::DrawMeshContainer(IDirect3DDevice9* pd3dDevice, EXTD3DXFRAME* pFrame, EXTD3DXMESHCONTAINER* pMeshContainer)
{
    // check for skinned mesh rendering
    if(pMeshContainer->pSkinInfo)
    {
        // check bone matrices buffer size
        UpdateBoneMatricesBuffer(pMeshContainer->pSkinInfo->GetNumBones());

        // set up bone transforms
        for(DWORD iBone = 0; iBone < pMeshContainer->pSkinInfo->GetNumBones(); ++iBone)
        {
            D3DXMatrixMultiply(
                &m_pBoneMatrices[iBone],
                &pMeshContainer->pBoneOffsetMatrices[iBone],
                pMeshContainer->ppBoneFrameMatrixPtrs[iBone]);
        }

        // set world transform
        D3DXMATRIX Identity;
        D3DXMatrixIdentity(&Identity);
        pd3dDevice->SetTransform(D3DTS_WORLD, &Identity);

        // lock mesh buffer and get verticies pointers
        PBYTE pbVerticesSrc;
        PBYTE pbVerticesDest;

        pMeshContainer->pSkinMesh->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&pbVerticesSrc);
        pMeshContainer->MeshData.pMesh->LockVertexBuffer(0, (LPVOID*)&pbVerticesDest);

        // update skinned mesh using bone matrices
        pMeshContainer->pSkinInfo->UpdateSkinnedMesh(m_pBoneMatrices, NULL, pbVerticesSrc, pbVerticesDest);

        // unlock mesh buffer
        pMeshContainer->pSkinMesh->UnlockVertexBuffer();
        pMeshContainer->MeshData.pMesh->UnlockVertexBuffer();
    }
    else
    {
        // set standard mesh transformation matrix
        pd3dDevice->SetTransform(D3DTS_WORLD, &pFrame->CombinedTransformationMatrix);
    }

    // traverse container materials
    for(DWORD i = 0; i < pMeshContainer->NumMaterials; i++ )
    {
        // set the material and texture for this subset
        pd3dDevice->SetMaterial(&pMeshContainer->pMaterials[i]);
        pd3dDevice->SetTexture(0, pMeshContainer->ppTextures[i]);

        // draw mesh
        pMeshContainer->MeshData.pMesh->DrawSubset(i);
    }
}

//------------------------------------------------------------------------------
// update bone matricies buffer
// increases buffer size if mesh requires it
//------------------------------------------------------------------------------
void PlayerAnimatedNode::UpdateBoneMatricesBuffer(DWORD NumBones)
{
    if(NumBones > m_NumBoneMatrices)
    {
        if(m_pBoneMatrices)
            delete [] m_pBoneMatrices;

        m_NumBoneMatrices = NumBones;
        m_pBoneMatrices = new D3DXMATRIXA16[m_NumBoneMatrices];
    }
}
