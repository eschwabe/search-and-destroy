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
#include "VertexShader.vfxobj"
#include "PixelShader.pfxobj"

/**
* Constructor
*/
RenderData::RenderData()
{}

/**
* Deconstructor
*/
RenderData::~RenderData()
{
    // cleanup shaders
    SAFE_RELEASE(m_pVertexShader);
    SAFE_RELEASE(m_pPixelShader);
}

/**
* Initialize render data
*/
HRESULT RenderData::Initialize(IDirect3DDevice9* pd3dDevice)
{ 
    // create shaders
    HRESULT result = pd3dDevice->CreateVertexShader((DWORD const*)VFX_VertexShader, &m_pVertexShader);
    
    if( SUCCEEDED(result) )
        result = pd3dDevice->CreatePixelShader((DWORD const*)PFX_PixelShader, &m_pPixelShader);

    return result;
}

/**
* Enables basic shaders to perform directional lighting
*/
HRESULT RenderData::EnableShaders(IDirect3DDevice9* pd3dDevice) const
{
    // compute trans(must be transposed for vertex shader)
    D3DXMATRIXA16 matWorldViewProjTrans = ComputeWorldViewProjection();
    D3DXMatrixTranspose(&matWorldViewProjTrans, &matWorldViewProjTrans);
    
    D3DXMATRIX matWorldTrans = matWorld;
    D3DXMatrixTranspose(&matWorldTrans, &matWorldTrans);

    // set shaders
    pd3dDevice->SetVertexShader(m_pVertexShader);
    pd3dDevice->SetPixelShader(m_pPixelShader);
    
    // set vertex shader constants
    pd3dDevice->SetVertexShaderConstantF(0, (const float*)(&matWorldViewProjTrans), 4);
    pd3dDevice->SetVertexShaderConstantF(4, (const float*)(&matWorldTrans), 3);
    pd3dDevice->SetVertexShaderConstantF(7, (const float*)(&vDirectionalLight), 1);

    // set pixel shader constants
    pd3dDevice->SetPixelShaderConstantF(0, (const float*)(&vDirectionalLightColor), 1);
    pd3dDevice->SetPixelShaderConstantF(1, (const float*)(&vAmbientColor), 1);

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
    
    pd3dDevice->SetRenderState(D3DRS_COLORVERTEX, false);
    pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE, false);
    pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );
    

    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);
}
