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
#include "PlayerTinyNode.h"
#include "DXUT/SDKmisc.h"

/**
* Constuct player node
*/
PlayerTinyNode::PlayerTinyNode(
        const std::wstring& sMeshFilename,
        const D3DXVECTOR3& vInitialPos) :
    PlayerBaseNode(vInitialPos, g_database.GetNewObjectID(), OBJECT_Player, "PLAYER"),
    m_sMeshFilename(sMeshFilename),
    m_NumBoneMatrices(0),
    m_pBoneMatrices(NULL),
    m_fPlayerScale(1.0f/800.0f),    // shrink tiny model
    m_ePlayerAnimation(kWait),
    m_iPlayerAnimationTrack(0)
{
    // set rotation and height for tiny model
    m_fPitchRotation = -D3DX_PI/2.0f;
    m_fHeight = 0.5f;
}

/**
* Destroy player node
*/
PlayerTinyNode::~PlayerTinyNode()
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
LRESULT PlayerTinyNode::HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER( hWnd );
    UNREFERENCED_PARAMETER( lParam );

    switch( uMsg )
    {
        // Key down message
        case WM_KEYDOWN:
        {
            PlayerActions move = GetPlayerMovement( (UINT)wParam );
            if( move != kUnknown )
            {
                m_PlayerMovement[move] = true;  
            }
            break;
        }

        // Key up message
        case WM_KEYUP:
        {
            PlayerActions move = GetPlayerMovement( (UINT)wParam );
            if( move != kUnknown )
            {
                m_PlayerMovement[move] = false;  
            }
            break; 
        }

        // left mouse down
        case WM_LBUTTONDOWN:
        {
            g_database.SendMsgFromSystem(GetID(), MSG_StartFireProjectiles); 
            break;
        }

        // left mouse up
        case WM_LBUTTONUP:
        {
            g_database.SendMsgFromSystem(GetID(), MSG_StopFireProjectiles); 
            break;
        }

        // right mouse
        case WM_RBUTTONDOWN:
        {
            g_database.SendMsgFromSystem(GetID(), MSG_FireBigProjectile);
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
PlayerTinyNode::PlayerActions PlayerTinyNode::GetPlayerMovement(const UINT& key)
{
    switch(key)
    {
        case VK_LEFT:       return kRotateLeft;         // turn player left
        case VK_RIGHT:      return kRotateRight;        // turn player right
        case VK_UP:         return kMoveForward;        // move player forward
        case VK_DOWN:       return kMoveBackward;       // move player backward

        case 'A':           return kRotateLeft;         // turn player left
        case 'D':           return kRotateRight;        // turn player right
        case 'W':           return kMoveForward;        // move player forward
        case 'S':           return kMoveBackward;       // move player backward

        case VK_SHIFT:      return kIncreaseSpeed;      // increase player speed

        default:        return kUnknown;        // unsupported
    }
}

/**
* Initialize player node. Loads the mesh hierarchy from the file and configures
* the node for rendering.
*/
HRESULT PlayerTinyNode::Initialize(IDirect3DDevice9* pd3dDevice)
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
void PlayerTinyNode::SetupBoneMatrices(EXTD3DXFRAME *pFrame)
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
void PlayerTinyNode::Update()
{
    const float kMaxSpeed = 3.0f;

    // update player velocity
    if( m_PlayerMovement[kMoveForward] )
    {
        // set initial velocity if not moving (+tiles per second)
        if(m_fVelocity == 0.0f)
            m_fVelocity = 1.5f;

        // check for max velocity (tiles per second)
        if(m_fVelocity > kMaxSpeed)
            m_fAccel = 0.0f;
        else
            m_fAccel = 1.0f;
    }
    else if( m_PlayerMovement[kMoveBackward] )
    {
        // set initial velocity if not moving (-tiles per second)
        if(m_fVelocity == 0.0f)
            m_fVelocity = -1.5f;

        // check for max velocity (tiles per second)
        if(m_fVelocity < -kMaxSpeed)
            m_fAccel = 0.0f;
        else
            m_fAccel = -1.0f;
    }
    else
    {
        // reset velocity and acceleration
        m_fAccel = 0.0f;
        m_fVelocity = 0.0f;
    }

    // update player rotation (yaw) (radians)
    if( m_PlayerMovement[kRotateLeft] )
        m_fYawRotation -= 0.03f;
    if( m_PlayerMovement[kRotateRight] )
        m_fYawRotation += 0.03f;

    // update direction based on player rotation
    D3DXMATRIX mMoveRot;
    D3DXMatrixRotationYawPitchRoll( &mMoveRot, m_fYawRotation, 0, 0 );
    D3DXVec3TransformCoord( &m_vDirection, &m_vDefaultDirection, &mMoveRot );

    // update position
    UpdateObjectPosition();

    // update animation
    UpdateAnimation();
}

/**
* Recomputes the player's transform matrix based on the current player position, rotation, and scale.
*/
void PlayerTinyNode::ComputeTransform()
{
    D3DXMATRIX mxTranslate;
    D3DXMATRIX mxRotate;
    D3DXMATRIX mxScale;

    // translate player
    D3DXMatrixTranslation(&mxTranslate, m_vPos.x, m_vPos.y, m_vPos.z);

    // rotate
    D3DXMatrixRotationYawPitchRoll(&mxRotate, m_fYawRotation, m_fPitchRotation, m_fRollRotation);
    
    // scale
    D3DXMatrixScaling(&mxScale, m_fPlayerScale, m_fPlayerScale, m_fPlayerScale);
    
    // compute transform
    m_matPlayer = mxScale * mxRotate * mxTranslate;
}

/**
* Updates the current player's animation and sets a new animation
* depending on the player velocity.
*/
void PlayerTinyNode::UpdateAnimation()
{
    if(m_AnimationController)
    {
        // determine animation
        Animation newAnimation;

        if( abs(m_fVelocity) > 3.0)
            newAnimation = kRun;
        else if( abs(m_fVelocity) > 0.0)
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
        m_AnimationController->AdvanceTime(g_time.GetElapsedTime(), NULL);
    }
}

/**
* Render traversal for drawing objects
*/
void PlayerTinyNode::Render(IDirect3DDevice9* pd3dDevice, const RenderData* rData)
{
    D3DXMATRIX matWorld;
    
    // update player transform
    ComputeTransform();

    // DRAW PLAYER
    // compute world player model transform matrix
    matWorld = m_matPlayer * rData->matWorld;

    // update frame transform matrices
    UpdateFrameTransforms((EXTD3DXFRAME*)m_FrameRoot, matWorld);

    // recursively draw the root frame
    DrawFrame(pd3dDevice, (EXTD3DXFRAME*)m_FrameRoot, rData, false);

    // DRAW PLAYER SHADOW
    if(rData->DrawShadows())
    {
        // compute shadow world player model transform matrix
        matWorld = m_matPlayer * rData->ComputeShadowWorldMatrix();

        // update frame transform matrices
        UpdateFrameTransforms((EXTD3DXFRAME*)m_FrameRoot, matWorld);

        // recursively draw the root frame
        DrawFrame(pd3dDevice, (EXTD3DXFRAME*)m_FrameRoot, rData, true);
    }
}

/**
* Recursively update frame transform matrices
*/
void PlayerTinyNode::UpdateFrameTransforms(EXTD3DXFRAME* pFrame, D3DXMATRIX matWorld)
{
    D3DXMatrixMultiply( &pFrame->CombinedTransformationMatrix, &pFrame->TransformationMatrix, &matWorld );

    // recurse for frame sibblings
    if(pFrame->pFrameSibling)
        UpdateFrameTransforms((EXTD3DXFRAME*)pFrame->pFrameSibling, matWorld);

    // recurse for frame children
    if(pFrame->pFrameFirstChild)
        UpdateFrameTransforms((EXTD3DXFRAME*)pFrame->pFrameFirstChild, pFrame->CombinedTransformationMatrix);
}

/**
* Recursively draw the frame
*/
void PlayerTinyNode::DrawFrame(IDirect3DDevice9* pd3dDevice, EXTD3DXFRAME* pFrame, const RenderData* rData, const bool bShadowDraw)
{
	// draw all mesh containers in this frame
    EXTD3DXMESHCONTAINER* pMeshContainer = (EXTD3DXMESHCONTAINER*)pFrame->pMeshContainer;

    // draw mesh containers
    while(pMeshContainer)
    {
        DrawMeshContainer(pd3dDevice, pFrame, pMeshContainer, rData, bShadowDraw);
        pMeshContainer = (EXTD3DXMESHCONTAINER*)pMeshContainer->pNextMeshContainer;
    }

	// recurse for frame sibblings
    if (pFrame->pFrameSibling)
        DrawFrame(pd3dDevice, (EXTD3DXFRAME*)pFrame->pFrameSibling, rData, bShadowDraw);

    // recurse for frame children
	if (pFrame->pFrameFirstChild)
        DrawFrame(pd3dDevice, (EXTD3DXFRAME*)pFrame->pFrameFirstChild, rData, bShadowDraw);
}

/**
* Draw mesh container (standard mesh or skinned mesh)
*/
void PlayerTinyNode::DrawMeshContainer(
    IDirect3DDevice9* pd3dDevice, EXTD3DXFRAME* pFrame, EXTD3DXMESHCONTAINER* pMeshContainer, 
    const RenderData* rData, const bool bShadowDraw)
{
    // check bone matrices buffer size
    UpdateBoneMatricesBuffer(pMeshContainer->dwNumPaletteEntries);

    // get bone combinations
    if( pMeshContainer->pBoneCombinationBuf != NULL )
    {
        LPD3DXBONECOMBINATION pBC = ( LPD3DXBONECOMBINATION )( pMeshContainer->pBoneCombinationBuf->GetBufferPointer() );
        DWORD dwAttrib, dwPalEntry;

        // for each palette
        for( dwAttrib = 0; dwAttrib < pMeshContainer->dwNumAttributeGroups; ++dwAttrib )
        {
            // set each transform into the palette
            for( dwPalEntry = 0; dwPalEntry < pMeshContainer->dwNumPaletteEntries; ++dwPalEntry )
            {
                DWORD dwMatrixIndex = pBC[ dwAttrib ].BoneId[ dwPalEntry ];
                if( dwMatrixIndex != UINT_MAX )
                    D3DXMatrixMultiply( &m_pBoneMatrices[ dwPalEntry ],
                                        &( pMeshContainer->pBoneOffsetMatrices[ dwMatrixIndex ] ),
                                        pMeshContainer->ppBoneFrameMatrixPtrs[ dwMatrixIndex ] );
            }

            // set light attributes in shader
            m_pEffect->SetVector( "lhtDir", &rData->vDirectionalLight);
            m_pEffect->SetVector( "lightDiffuse", (const D3DXVECTOR4*)&rData->vDirectionalLightColor );

            // set effect view projection matrix
            D3DXMATRIX matViewProj;
            D3DXMatrixMultiply(&matViewProj, &rData->matView, &rData->matProjection);
            m_pEffect->SetMatrix( "g_mViewProj", &matViewProj );

            // set the matrix palette into the effect
            m_pEffect->SetMatrixArray( "amPalette", m_pBoneMatrices, pMeshContainer->dwNumPaletteEntries );

            // override texture settings for shadow drawing
            if(bShadowDraw)
            {
                // shadow texture
                m_pEffect->SetTexture( "g_txScene", rData->pShadowTexture );
                        
                // enable alpha blending
                pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
                pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);
                pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);

                // override material colors
                m_pEffect->SetVector( "MaterialAmbient", (const D3DXVECTOR4*)&(D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.0f)) );
                m_pEffect->SetVector( "MaterialDiffuse", (D3DXVECTOR4*)&(D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.0f)) );

                // set depth bias (prevent z-fighting)
                float fDepthBias = -0.0000001f;
                float fDepthBiasSlope = -1.0f;
                pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
                pd3dDevice->SetRenderState(D3DRS_DEPTHBIAS, *((DWORD*)&fDepthBias));
                pd3dDevice->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, *((DWORD*)&fDepthBiasSlope));
                
                // disable z buffer
                pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
            }
            else
            {   
                // set material colors
                m_pEffect->SetVector( "MaterialAmbient", (const D3DXVECTOR4*)&rData->vAmbientColor );
                m_pEffect->SetVector( "MaterialDiffuse", (D3DXVECTOR4*)&(pMeshContainer->pMaterials[pBC[dwAttrib].AttribId].Diffuse) );

                // we're pretty much ignoring the materials we got from the x-file; just set the texture here
                m_pEffect->SetTexture( "g_txScene", pMeshContainer->ppTextures[ pBC[ dwAttrib ].AttribId ] );
            }

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
}

/**
* Update bone matricies buffer. Increases buffer size if mesh requires it.
*/
void PlayerTinyNode::UpdateBoneMatricesBuffer(DWORD NumBones)
{
    if(NumBones > m_NumBoneMatrices)
    {
        if(m_pBoneMatrices)
            delete [] m_pBoneMatrices;

        m_NumBoneMatrices = NumBones;
        m_pBoneMatrices = new D3DXMATRIX[m_NumBoneMatrices];
    }
}
