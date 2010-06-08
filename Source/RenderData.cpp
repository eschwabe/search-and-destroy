/*******************************************************************************
* Game Development Project
* RenderData.cpp
*
* Eric Schwabe
* 2010-02-17
*
* Render data
*
*******************************************************************************/

#include "DXUT.h"
#include "RenderData.h"
#include "DXUT\SDKmisc.h"
#include "VertexShader.vfxobj"
#include "PixelShader.pfxobj"
#include "VSSkin.vfxobj"
#include "PSSkin.pfxobj"

/**
* Constructor
*/
RenderData::RenderData() :
    m_bDrawShadows(true)
{}

/**
* Deconstructor
*/
RenderData::~RenderData()
{
    // cleanup shaders
    SAFE_RELEASE(m_pVertexShader);
    SAFE_RELEASE(m_pPixelShader);
    SAFE_RELEASE(m_pVSSkin);
    SAFE_RELEASE(m_pPSSkin);

    // cleanup textures
    SAFE_RELEASE(pShadowTexture);
}

/**
* Initialize render data
*/
HRESULT RenderData::Initialize(IDirect3DDevice9* pd3dDevice)
{ 
    // create directional light shaders
    HRESULT result = pd3dDevice->CreateVertexShader((DWORD const*)VFX_VertexShader, &m_pVertexShader);
    
    if( SUCCEEDED(result) )
        result = pd3dDevice->CreatePixelShader((DWORD const*)PFX_PixelShader, &m_pPixelShader);

    // create skinning shaders
    if( SUCCEEDED(result) )
        result = pd3dDevice->CreateVertexShader((DWORD const*)VFX_VSSkin, &m_pVSSkin);

    if( SUCCEEDED(result) )
        result = pd3dDevice->CreatePixelShader((DWORD const*)PFX_PSSkin, &m_pPSSkin);

    // search and load shadow texture
    if( SUCCEEDED(result) )
    {    
        WCHAR wsNewPath[ MAX_PATH ];
        DXUTFindDXSDKMediaFileCch(wsNewPath, sizeof(wsNewPath), L"shadow-texture.png");
        result = D3DXCreateTextureFromFile(pd3dDevice, wsNewPath, &pShadowTexture);
    }

    return result;
}

/**
* Setup directional light
*/
void RenderData::SetDirectionalLight(const D3DXVECTOR4& vDir, const D3DXCOLOR& cColor)
{
    vDirectionalLight = vDir;
    D3DXVec4Normalize(&vDirectionalLight, &vDirectionalLight);
    vDirectionalLightColor = cColor;
}

/**
* Setup ambient light
*/
void RenderData::SetAmbientLight(const D3DXCOLOR& cColor)
{
    vAmbientColor = cColor;
}

/**
* Get shadow world matrix
*/
D3DXMATRIX RenderData::ComputeShadowWorldMatrix() const
{
    // create skew transform
    D3DXMATRIX matSkew;
    D3DXMatrixIdentity(&matSkew);
    matSkew._21 = 0.5f - vDirectionalLight.x; // kxy
    matSkew._31 = 0.0f; // kxz (always 0)
    matSkew._13 = 0.0f; // kzx (always 0)
    matSkew._23 = 0.5f - vDirectionalLight.y; // kzy

    // create world matrix and flatten (y axis)
    D3DXMATRIX matShadowWorld = matWorld * matSkew;
    matShadowWorld._12 = 0.0f;
    matShadowWorld._22 = 0.0f;
    matShadowWorld._32 = 0.0f;
    matShadowWorld._42 = 0.0f;

    return matShadowWorld;
}

/**
* Enables basic shaders to perform directional lighting
*/
HRESULT RenderData::EnableDirectionalShaders(IDirect3DDevice9* pd3dDevice, const bool bShadowDraw) const
{
    D3DXMATRIXA16 matWorldViewProjTrans;
    D3DXCOLOR cDirLightColor;
    D3DXCOLOR cAmbientColor;

    // compute transform and set colors (must be transposed for vertex shader)
    if(bShadowDraw)
    {
        matWorldViewProjTrans = ComputeShadowWorldMatrix() * matView * matProjection;
        cDirLightColor = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.0f); // negates lighting calculations
        cAmbientColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);  // use only texture colors and alpha
    }
    else
    {
        matWorldViewProjTrans = ComputeWorldViewProjection();
        cDirLightColor = vDirectionalLightColor;
        cAmbientColor = vAmbientColor;
    }

    // transpose world-view-proj matrix for vertex shader
    D3DXMatrixTranspose(&matWorldViewProjTrans, &matWorldViewProjTrans);

    // transpose world matrix
    D3DXMATRIX matWorldTrans = matWorld;
    D3DXMatrixTranspose(&matWorldTrans, &matWorldTrans);

    // set shaders
    pd3dDevice->SetVertexShader(m_pVertexShader);
    pd3dDevice->SetPixelShader(m_pPixelShader);
    
    // set vertex shader constants
    pd3dDevice->SetVertexShaderConstantF(0, (const float*)(&matWorldViewProjTrans), 4);
    pd3dDevice->SetVertexShaderConstantF(4, (const float*)(&matWorldTrans), 3);

    // set pixel shader constants
    pd3dDevice->SetPixelShaderConstantF(0, (const float*)(&cDirLightColor), 1);
    pd3dDevice->SetPixelShaderConstantF(1, (const float*)(&vDirectionalLight), 1);
    pd3dDevice->SetPixelShaderConstantF(2, (const float*)(&cAmbientColor), 1);

    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);
}

/**
* Enables skinning shaders
*/
HRESULT RenderData::EnableSkinShaders(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX& matWorldObj, const D3DXMATRIX* matBones, const DWORD dNumBones) const
{
    // compute transform (must be transposed for vertex shader)
    D3DXMATRIXA16 matWorldViewProjTrans = ComputeWorldViewProjection();
    matWorldViewProjTrans = matWorldObj * matWorldViewProjTrans;
    D3DXMatrixTranspose(&matWorldViewProjTrans, &matWorldViewProjTrans);
    
    // set shaders
    pd3dDevice->SetVertexShader(m_pVSSkin);
    pd3dDevice->SetPixelShader(m_pPSSkin);
  
    // set vertex shader constants
    pd3dDevice->SetVertexShaderConstantF(0, (const float*)(&matWorldViewProjTrans), 4);

    // set vertex shader bone transforms
    for(DWORD i = 0; i < dNumBones; i++)
    {
        D3DXMATRIX matBone = matBones[i];
        D3DXMatrixTranspose(&matBone, &matBone);
        pd3dDevice->SetVertexShaderConstantF(4+4*i, (const float*)(&matBone), 4);
    }

    // set pixel shader constants
    pd3dDevice->SetPixelShaderConstantF(0, (const float*)(&vDirectionalLightColor), 1);
    pd3dDevice->SetPixelShaderConstantF(1, (const float*)(&vDirectionalLight), 1);
    pd3dDevice->SetPixelShaderConstantF(2, (const float*)(&vAmbientColor), 1);

    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);
}

/**
* Enables D3D fixed function pipeline directional lighting
*/
HRESULT RenderData::EnableD3DLighting(IDirect3DDevice9* pd3dDevice) const
{   
    // enable lighting
    pd3dDevice->SetRenderState(D3DRS_LIGHTING, true);

    // enable ambient light
    pd3dDevice->SetRenderState(D3DRS_AMBIENT, vAmbientColor);

    // build direction light
    // dwarf model requires light direction to be inverted to make light
    // consistent with rest of world (problem with shaders?)
    D3DLIGHT9 light;
    memset(&light, 0, sizeof(light));
    light.Type		    = D3DLIGHT_DIRECTIONAL;
    light.Diffuse       = vDirectionalLightColor;
    light.Ambient       = vAmbientColor;
    light.Direction.x   = -vDirectionalLight.x;
    light.Direction.y   = -vDirectionalLight.y;
    light.Direction.z   = -vDirectionalLight.z;
    light.Attenuation0  = 10000.f;
    light.Range		    = 10000.f;

    // enable directional light
    pd3dDevice->SetLight(0, &light);
    pd3dDevice->LightEnable(0, true);
    
    pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
    
    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);
}
