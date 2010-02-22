/*******************************************************************************
* Game Development Project
* RenderData.h
*
* Eric Schwabe
* 2010-02-17
*
* Render data
*
*******************************************************************************/

#pragma once

class RenderData
{
    public:

        // constructor
        RenderData();
        virtual ~RenderData();

        // initialize
        HRESULT RenderData::Initialize(IDirect3DDevice9* pd3dDevice);

        // compute a world, view, projection transform matrix
        D3DXMATRIXA16 ComputeWorldViewProjection() const
        {
            return matWorld * matView * matProjection;
        }

        // enables basic shaders to perform directional lighting
        HRESULT EnableDirectionalShaders(IDirect3DDevice9* pd3dDevice) const;
        
        // enables basic shaders to perform directional lighting
        HRESULT EnableSkinShaders(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX* matBones) const;

        // enables D3D fixed function pipeline directional lighting
        HRESULT EnableD3DLighting(IDirect3DDevice9* pd3dDevice) const;

        // transform data
        D3DXMATRIXA16 matView;
        D3DXMATRIXA16 matProjection;
        D3DXMATRIXA16 matWorld;

        // light data
        D3DXVECTOR4 vDirectionalLight;
        D3DXCOLOR vDirectionalLightColor;
        D3DXCOLOR vAmbientColor;

    private:

        LPDIRECT3DVERTEXSHADER9 m_pVertexShader;    // vertex shader
        LPDIRECT3DPIXELSHADER9 m_pPixelShader;      // pixel shader

        LPDIRECT3DVERTEXSHADER9 m_pVSSkin;          // vertex skinning shader
        LPDIRECT3DPIXELSHADER9 m_pPSSkin;           // pixel skinning shader
};
