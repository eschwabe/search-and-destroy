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
    m_vPlayerPos(fX, fY, fZ),
    m_vPlayerVelocity(0.0f, 0.0f, 0.0f),
    m_vPlayerAccel(0.0f, 0.0f, 0.0f),
    m_ePlayerAnimation(kWait),
    m_iPlayerAnimationTrack(0),
    m_playerSkinInfo(false),
    m_NPCMode(false)
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
    // deallocate bone matrices buffer
    SAFE_DELETE_ARRAY(m_pBoneMatrices);

    // deallocate controller
	SAFE_RELEASE(m_AnimationController);

    // deallocate effects
    SAFE_RELEASE(m_pEffect);

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
    return m_vPlayerPos;
}

/**
* Get the current height of the player. The current height implementation is fixed
* to the size of the tiny model. The height should be calculated based on the model.
*
* @return player height
*/
float PlayerNode::GetPlayerHeight() const
{
    return m_fPlayerScale * 500.0f;
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
* Notifies the player that it has collided with an object. The event provides
* the position delta required to resolve the collision. Modifies the player 
* position by the specified delta.
*/
void PlayerNode::PlayerCollisionEvent(const D3DXVECTOR3& vPosDelta)
{
    // set collision flag
    m_PlayerCollision = true;

    // move player position
    m_vPlayerPos += vPosDelta;
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
* Map keyboard key to player movement
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

        case 'A':       return kRotateLeft;     // turn player left
        case 'D':       return kRotateRight;    // turn player right
        case 'W':       return kMoveForward;    // move player forward
        case 'S':       return kMoveBackward;   // move player backward

        case VK_SHIFT:  return kIncreaseSpeed;  // increase player speed

        default:        return kUnknown;        // unsupported
    }
}

/**
* Initialize player node. Loads the mesh hierarchy from the file and configures
* the node for rendering.
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

    // load effects
    if( SUCCEEDED(result) )
    {
        // Increase the palette size if the shader allows it. We are sort
        // of cheating here since we know tiny has 35 bones. The alternative
        // is use the maximum number that vs_2_0 allows.
        D3DXMACRO mac[2] =
        {
            { "MATRIX_PALETTE_SIZE_DEFAULT", "35" },
            { NULL,                          NULL }
        };

        WCHAR szEffectPath[MAX_PATH];
        DXUTFindDXSDKMediaFileCch( szEffectPath, MAX_PATH, L"MultiAnimation.fx" );
        result = D3DXCreateEffectFromFile( pd3dDevice, szEffectPath, mac, NULL, D3DXFX_NOT_CLONEABLE, NULL, &m_pEffect, NULL );
        m_pEffect->SetTechnique( "Skinning20" );
    }

    // setup matrices and animation
    if( SUCCEEDED(result) )
    {
        if(m_AnimationController)
        {
            // set new animation track
            LPD3DXANIMATIONSET set;
            m_AnimationController->GetAnimationSet( m_ePlayerAnimation, &set );	      
            m_AnimationController->SetTrackAnimationSet( m_iPlayerAnimationTrack, set );
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
		if(pMesh->pSkinInfo)
		{
            // model contains skin info
            m_playerSkinInfo = true;

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
* Automatically changes the player movements. Should be called in NPC mode only.
*/
void PlayerNode::AutoPlayerMove(double fTime)
{
    static double dNextUpdateTime = 0.0;
    static double dCurrentTime = 0.0;

    // update time
    dCurrentTime += fTime;

    // check if enough time has passed
    if(dCurrentTime >= dNextUpdateTime)
    {
        // reset actions
        for(int i =0; i < sizeof(m_PlayerMovement); i++)
            m_PlayerMovement[i] = false;

        // if collision, rotate player
        if(m_PlayerCollision)
        {
            // reset collision flag
            m_PlayerCollision = false;

            // choose rotate action
            switch(rand() % 2)
            {
            case 0:
                m_PlayerMovement[kRotateLeft] = true;
                break;
            case 1:
                m_PlayerMovement[kRotateRight] = true;
                break;
            
            default:
                break;
            }
        }
        else
        {
            // choose new move action
            switch(rand() % 3)
            {
            case 0:
                m_PlayerMovement[kMoveForward] = true;
                break;
            case 1:
                m_PlayerMovement[kMoveForward] = true;
                break;
            case 2:
                m_PlayerMovement[kIncreaseSpeed] = true;
                m_PlayerMovement[kMoveForward] = true;
                break;
            default:
                break;
            }
        }

        // set next update time
        dNextUpdateTime += 1.0;
    }
}

/**
* Update traversal for physics, AI, etc.
*/
void PlayerNode::UpdateNode(double fTime)
{
    const float kMaxSpeed = 3.0f;

    // check for NPC mode
    if(m_NPCMode)
        AutoPlayerMove(fTime);

    // reset collision flag
            m_PlayerCollision = false;

    // update player acceleration
    if( m_PlayerMovement[kIncreaseSpeed] )
    {
        // set acceleration
        m_vPlayerAccel.z = 1.5f;
    }

    // update player velocity
    if( m_PlayerMovement[kMoveForward] )
    {
        // set initial velocity if not moving (+tiles per second)
        if(m_vPlayerVelocity.z == 0.0f)
            m_vPlayerVelocity.z = 1.5f;

        // check for max velocity (tiles per second)
        if(m_vPlayerVelocity.z < kMaxSpeed)
            m_vPlayerVelocity += m_vPlayerAccel * (float)fTime;
    }
    else if( m_PlayerMovement[kMoveBackward] )
    {
        // set initial velocity if not moving (-tiles per second)
        if(m_vPlayerVelocity.z == 0.0f)
            m_vPlayerVelocity.z = -1.5f;

        // check for max velocity (tiles per second)
        if(m_vPlayerVelocity.z > -kMaxSpeed)
            m_vPlayerVelocity -= m_vPlayerAccel * (float)fTime;
    }
    else
    {
        // reset velocity and acceleration
        m_vPlayerAccel = D3DXVECTOR3(0,0,0);
        m_vPlayerVelocity = D3DXVECTOR3(0,0,0);
    }

    // compute player position delta
    D3DXVECTOR3 vPosDelta = D3DXVECTOR3(0,0,0);
    vPosDelta += m_vPlayerVelocity * (float)fTime;

    // update animation
    UpdateAnimation(fTime);

    // update player rotation (yaw) (radians)
    if( m_PlayerMovement[kRotateLeft] )
        m_fPlayerYawRotation -= 0.03f;
    if( m_PlayerMovement[kRotateRight] )
        m_fPlayerYawRotation += 0.03f;

    // create movement rotation matrix (yaw only)
    D3DXMATRIX mMoveRot;
    D3DXMatrixRotationYawPitchRoll( &mMoveRot, m_fPlayerYawRotation, 0, 0 );

    // rotate position delta based on yaw
    D3DXVec3TransformCoord( &vPosDelta, &vPosDelta, &mMoveRot );

    // move player position
    m_vPlayerPos += vPosDelta;
}

/**
* Recomputes the player's transform matrix based on the current player position, rotation, and scale.
*/
void PlayerNode::ComputeTransform()
{
    D3DXMATRIX mx;

    // translate player
    D3DXMatrixTranslation(&m_matPlayer, m_vPlayerPos.x, m_vPlayerPos.y, m_vPlayerPos.z);

    // rotate
    D3DXMatrixRotationYawPitchRoll(&mx, m_fPlayerYawRotation, m_fPlayerPitchRotation, m_fPlayerRollRotation);
    D3DXMatrixMultiply(&m_matPlayer, &mx, &m_matPlayer);

    // scale
    D3DXMatrixScaling(&mx, m_fPlayerScale, m_fPlayerScale, m_fPlayerScale);
    D3DXMatrixMultiply(&m_matPlayer, &mx, &m_matPlayer);
}

/**
* Updates the current player's animation and sets a new animation
* depending on the player velocity.
*/
void PlayerNode::UpdateAnimation(double fTime)
{
    if(m_AnimationController)
    {
        // determine animation
        Animation newAnimation;

        if( abs(m_vPlayerVelocity.z) > 3.0)
            newAnimation = kRun;
        else if( abs(m_vPlayerVelocity.z) > 0.0)
            newAnimation = kWalk;
        else
            newAnimation = kWait;
        
        // check if new animation
        if(m_ePlayerAnimation != newAnimation)
        {
            // get animation set and track
            LPD3DXANIMATIONSET set;
            m_AnimationController->GetAnimationSet( newAnimation, &set );	      
            int iNewTrack = ( m_iPlayerAnimationTrack == 0 ? 1 : 0 );

            // set new animation track
            m_AnimationController->SetTrackAnimationSet( iNewTrack, set );
            set->Release();

            // clear events
            m_AnimationController->UnkeyAllTrackEvents( m_iPlayerAnimationTrack );
            m_AnimationController->UnkeyAllTrackEvents( iNewTrack );

            // set track events
            m_AnimationController->KeyTrackEnable( m_iPlayerAnimationTrack, FALSE, m_AnimationController->GetTime() + 0.25f );
            m_AnimationController->KeyTrackSpeed( m_iPlayerAnimationTrack, 0.0f, m_AnimationController->GetTime(), 0.25f, D3DXTRANSITION_LINEAR );
            m_AnimationController->KeyTrackWeight( m_iPlayerAnimationTrack, 0.0f, m_AnimationController->GetTime(), 0.25f, D3DXTRANSITION_LINEAR );
            m_AnimationController->SetTrackEnable( iNewTrack, TRUE );
            m_AnimationController->KeyTrackSpeed( iNewTrack, 1.0f, m_AnimationController->GetTime(), 0.25f, D3DXTRANSITION_LINEAR );
            m_AnimationController->KeyTrackWeight( iNewTrack, 1.0f, m_AnimationController->GetTime(), 0.25f, D3DXTRANSITION_LINEAR );

            // set new player track and animation
            m_iPlayerAnimationTrack = iNewTrack;
            m_ePlayerAnimation = newAnimation;
        }

        // update animation
        m_AnimationController->AdvanceTime(fTime, NULL);
    }
}

/**
* Render traversal for drawing objects
*/
void PlayerNode::RenderNode(IDirect3DDevice9* pd3dDevice, D3DXMATRIX rMatWorld)
{
    D3DXMatrixMultiply(&rMatWorld, &m_matPlayer, &rMatWorld);

    // compute new player model transform matrix
    ComputeTransform();

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
        UpdateBoneMatricesBuffer(pMeshContainer->dwNumPaletteEntries);

        // get bone combinations
        LPD3DXBONECOMBINATION pBC = ( LPD3DXBONECOMBINATION )( pMeshContainer->pBoneCombinationBuf->GetBufferPointer() );
        DWORD dwAttrib, dwPalEntry;

        // for each palette
        for( dwAttrib = 0; dwAttrib < pMeshContainer->dwNumAttributeGroups; ++ dwAttrib )
        {
            // set each transform into the palette
            for( dwPalEntry = 0; dwPalEntry < pMeshContainer->dwNumPaletteEntries; ++ dwPalEntry )
            {
                DWORD dwMatrixIndex = pBC[ dwAttrib ].BoneId[ dwPalEntry ];
                if( dwMatrixIndex != UINT_MAX )
                    D3DXMatrixMultiply( &m_pBoneMatrices[ dwPalEntry ],
                                        &( pMeshContainer->pBoneOffsetMatrices[ dwMatrixIndex ] ),
                                        pMeshContainer->ppBoneFrameMatrixPtrs[ dwMatrixIndex ] );
            }

            // set effect view projection matrix
            m_pEffect->SetMatrix( "g_mViewProj", &m_matViewProj );

            // set the matrix palette into the effect
            m_pEffect->SetMatrixArray( "amPalette", m_pBoneMatrices, pMeshContainer->dwNumPaletteEntries );

            // set effect material diffuse
            m_pEffect->SetVector( "MaterialDiffuse", (D3DXVECTOR4*)&(pMeshContainer->pMaterials[pBC[dwAttrib].AttribId].Diffuse) );

            // we're pretty much ignoring the materials we got from the x-file; just set the texture here
            m_pEffect->SetTexture( "g_txScene", pMeshContainer->ppTextures[ pBC[ dwAttrib ].AttribId ] );

            // set the current number of bones; this tells the effect which shader to use
            m_pEffect->SetInt( "CurNumBones", pMeshContainer->dwNumInfl - 1 );

            // run through each pass and draw
            UINT uiPasses, uiPass;

            m_pEffect->Begin( &uiPasses, 0/*D3DXFX_DONOTSAVESTATE*/ );

            for( uiPass = 0; uiPass < uiPasses; ++ uiPass )
            {
                m_pEffect->BeginPass( uiPass );
                pMeshContainer->pSkinMesh->DrawSubset( dwAttrib );
                m_pEffect->EndPass();
            }

            m_pEffect->End();
        }
    }

    // normal mesh rendering
    else if(!m_playerSkinInfo)
    {
        // set standard mesh transformation matrix
        pd3dDevice->SetTransform(D3DTS_WORLD, &pFrame->CombinedTransformationMatrix);

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
        m_pBoneMatrices = new D3DXMATRIX[m_NumBoneMatrices];
    }
}
