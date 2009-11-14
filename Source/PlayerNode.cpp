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

#include "DXUT.h"
#include "PlayerNode.h"
#include "DXUT/SDKmisc.h"

/**
* Constuct player node
* Specify player scale, x,y,z coordinates, and x,y,z rotations (in radians)
*/
PlayerNode::PlayerNode(const std::wstring& sMeshFilename, const float fScale, 
                       const float fX, const float fY, const float fZ, 
                       const float fYaw, const float fPitch, const float fRoll) :
    m_sMeshFilename(sMeshFilename),
    m_NumBoneMatrices(0),
    m_pBoneMatrices(NULL),
    m_fPlayerScale(fScale),
    m_fPlayerYawRotation(fYaw),
    m_fPlayerPitchRotation(fPitch),
    m_fPlayerRollRotation(fRoll),
    vPlayerPos(fX, fY, fZ)
{
    // initialize movement to default state
    for(int i =0; i < sizeof(m_PlayerMovement); i++)
        m_PlayerMovement[i] = false;
}

/**
* Destroy player node
*/
PlayerNode::~PlayerNode()
{
    // deallocate bone matrices buffer (if allocated)
    if(m_pBoneMatrices)
        delete [] m_pBoneMatrices;

    // deallocate controller
    if(m_AnimationController)
		m_AnimationController->Release();

    // deallocate frame
    MeshHierarchyBuilder hierarchyBuilder(NULL);
    D3DXFrameDestroy(m_FrameRoot, &hierarchyBuilder);
}

/**
* Get the current position of the player.
*
* @return vector with player position coordinates
*/
D3DXVECTOR3 PlayerNode::GetPlayerPosition() const
{
    return vPlayerPos;
}

/**
* Get the current height of the player.
*
* @return player height
*/
float PlayerNode::GetPlayerHeight() const
{
    return 1.0f;
}

/**
* Get the current rotation of the player.
*
* @return player rotation in radians
*/
float PlayerNode::GetPlayerRotation() const
{
    return m_fPlayerYawRotation;
}

/**
* Handle user input messages. 
*
* @param hWnd Handle to the window procedure to receive the message
* @param uMsg Specifies the message type (i.e. key down or key up)
* @param wParam Specifies additional message-specific information. The contents 
*               of this parameter depend on the value of the Msg parameter.
*               In this context, the param is the associated keyboard key with the event.
* @param lParam Specifies additional message-specific information. The contents 
*               of this parameter depend on the value of the Msg parameter.
*/
LRESULT PlayerNode::HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    UNREFERENCED_PARAMETER( hWnd );
    UNREFERENCED_PARAMETER( lParam );

    switch( uMsg )
    {
        // Key down message
        case WM_KEYDOWN:
        {
            Movement move = GetPlayerMovement( (UINT)wParam );
            if( move != kUnknown )
            {
                m_PlayerMovement[move] = true;  
            }
            break;
        }

        // Key up message
        case WM_KEYUP:
        {
            Movement move = GetPlayerMovement( (UINT)wParam );
            if( move != kUnknown )
            {
                m_PlayerMovement[move] = false;  
            }
            break; 
        }

        // Unsupported message
        default:
            return FALSE;
    }

    return TRUE;
}

/**
* Map keyboard key to camera movement
*
* @return camera movement 
*/
PlayerNode::Movement PlayerNode::GetPlayerMovement(const UINT& key)
{
    switch(key)
    {
        case VK_LEFT:   return kRotateLeft;     // turn player left
        case VK_RIGHT:  return kRotateRight;    // turn player right
        case VK_UP:     return kMoveForward;    // move player forward
        case VK_DOWN:   return kMoveBackward;   // move player backward
        default:        return kUnknown;        // unsupported
    }
}

/**
* Initialize player node
*/
HRESULT PlayerNode::InitializeNode(IDirect3DDevice9* pd3dDevice)
{
    // search for file
    WCHAR wsNewPath[ MAX_PATH ];
    HRESULT result = DXUTFindDXSDKMediaFileCch(wsNewPath, sizeof(wsNewPath), m_sMeshFilename.c_str());

    // load mesh
    if( SUCCEEDED(result) )
    {
        MeshHierarchyBuilder hierarchyBuilder(pd3dDevice);

        result = D3DXLoadMeshHierarchyFromX(
            wsNewPath,                  // [in] mesh filename
            0,                          // [in] options (D3DXMESH)
            pd3dDevice,                 // [in] D3DDevice
            &hierarchyBuilder,          // [in] mesh/frame memory allocater
            NULL,                       // [in] user data 
            &m_FrameRoot,               // [out] top of frame hierarchy
            &m_AnimationController);    // [out] animation controller
    }

    // setup matrices and animation
    if( SUCCEEDED(result) )
    {
        // get and set animation set
        if(m_AnimationController)
        {
	        LPD3DXANIMATIONSET set;
	        m_AnimationController->GetAnimationSet(1, &set);	
	        m_AnimationController->SetTrackAnimationSet( 0, set );
            set->Release();	
        }

        // setup bone frame matrix pointers
        SetupBoneMatrices((EXTD3DXFRAME*)m_FrameRoot);
    }

    return result;
}

/**
* Setup mesh container bone matrices pointers
*/
void PlayerNode::SetupBoneMatrices(EXTD3DXFRAME *pFrame)
{
    assert(pFrame);

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

/**
* Update traversal for physics, AI, etc.
*/
void PlayerNode::UpdateNode(double fTime)
{
    // update animation
    if(m_AnimationController)
        m_AnimationController->AdvanceTime(fTime, NULL);

    // update player rotation (yaw) (radians)
    if( m_PlayerMovement[kRotateLeft] )
        m_fPlayerYawRotation -= 0.03f;
    if( m_PlayerMovement[kRotateRight] )
        m_fPlayerYawRotation += 0.03f;

    // create movement rotation matrix (yaw only)
    D3DXMATRIX mMoveRot;
    D3DXMatrixRotationYawPitchRoll( &mMoveRot, m_fPlayerYawRotation, 0, 0 );

    // compute player position delta
    D3DXVECTOR3 vPosDelta = D3DXVECTOR3(0,0,0);

    // update z axis movement
    if( m_PlayerMovement[kMoveForward] )
        vPosDelta.z += 0.05f;
    if( m_PlayerMovement[kMoveBackward] )
        vPosDelta.z += -0.05f;

    // rotate position delta based on yaw
    D3DXVec3TransformCoord( &vPosDelta, &vPosDelta, &mMoveRot );

    // move player position
    vPlayerPos += vPosDelta;

    // compute new player model transform matrix
    D3DXMATRIX mx;

    // translate player
    D3DXMatrixTranslation(&m_matPlayer, vPlayerPos.x, vPlayerPos.y, vPlayerPos.z);

    // rotate
    D3DXMatrixRotationYawPitchRoll(&mx, m_fPlayerYawRotation, m_fPlayerPitchRotation, m_fPlayerRollRotation);
    D3DXMatrixMultiply(&m_matPlayer, &mx, &m_matPlayer);

    // scale
    D3DXMatrixScaling(&mx, m_fPlayerScale, m_fPlayerScale, m_fPlayerScale);
    D3DXMatrixMultiply(&m_matPlayer, &mx, &m_matPlayer);
}

/**
* Render traversal for drawing objects
*/
void PlayerNode::RenderNode(IDirect3DDevice9* pd3dDevice, D3DXMATRIX rMatWorld)
{
    D3DXMatrixMultiply(&rMatWorld, &m_matPlayer, &rMatWorld);

    // update frame transform matrices
    UpdateFrameTransforms((EXTD3DXFRAME*)m_FrameRoot, rMatWorld);

    // recursively draw the root frame
    DrawFrame(pd3dDevice, (EXTD3DXFRAME*)m_FrameRoot);
}

/**
* Recursively update frame transform matrices
*/
void PlayerNode::UpdateFrameTransforms(EXTD3DXFRAME* pFrame, D3DXMATRIX rMatWorld)
{
    D3DXMatrixMultiply( &pFrame->CombinedTransformationMatrix, &pFrame->TransformationMatrix, &rMatWorld );

    // recurse for frame sibblings
    if(pFrame->pFrameSibling)
        UpdateFrameTransforms((EXTD3DXFRAME*)pFrame->pFrameSibling, rMatWorld);

    // recurse for frame children
    if(pFrame->pFrameFirstChild)
        UpdateFrameTransforms((EXTD3DXFRAME*)pFrame->pFrameFirstChild, pFrame->CombinedTransformationMatrix);
}

/**
* Recursively draw the frame
*/
void PlayerNode::DrawFrame(IDirect3DDevice9* pd3dDevice, EXTD3DXFRAME* pFrame)
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

/**
* Draw mesh container (standard mesh or skinned mesh)
*/
void PlayerNode::DrawMeshContainer(IDirect3DDevice9* pd3dDevice, EXTD3DXFRAME* pFrame, EXTD3DXMESHCONTAINER* pMeshContainer)
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

/**
* Update bone matricies buffer. Increases buffer size if mesh requires it.
*/
void PlayerNode::UpdateBoneMatricesBuffer(DWORD NumBones)
{
    if(NumBones > m_NumBoneMatrices)
    {
        if(m_pBoneMatrices)
            delete [] m_pBoneMatrices;

        m_NumBoneMatrices = NumBones;
        m_pBoneMatrices = new D3DXMATRIXA16[m_NumBoneMatrices];
    }
}
