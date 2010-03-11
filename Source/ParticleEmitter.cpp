/*******************************************************************************
* Game Development Project
* ParticleEmitter.cpp
*
* Eric Schwabe
* 2010-03-07
*
* Particle Emitter
*
*******************************************************************************/

#include "DXUT.h"
#include "ParticleEmitter.h"

// custom vertex structure definition
const D3DVERTEXELEMENT9 ParticleEmitter::m_sCustomVertexDeclaration[] =
{
    { 0, offsetof(CustomVertex, vPos        ), D3DDECLTYPE_FLOAT3  , 0, D3DDECLUSAGE_POSITION, 0 },
    { 0, offsetof(CustomVertex, cDiffuse    ), D3DDECLTYPE_D3DCOLOR, 0, D3DDECLUSAGE_COLOR   , 0 },
    D3DDECL_END(),
};

/**
* Constructor 
*/
ParticleEmitter::ParticleEmitter()
{
}

/**
* Deconstructor 
*/
ParticleEmitter::~ParticleEmitter()
{
    // cleanup vertex declaration
    SAFE_RELEASE(m_pCVDeclaration);
}

/**
* Add new player tracking
*/
void ParticleEmitter::AddPlayerTracking(const PlayerNode* player)
{
    m_PlayerList.push_back(player);
}

/**
* Initialize particle emitter 
*/
HRESULT ParticleEmitter::InitializeNode(IDirect3DDevice9* pd3dDevice)
{
    // create vertex declaration
    HRESULT result = pd3dDevice->CreateVertexDeclaration(m_sCustomVertexDeclaration, &m_pCVDeclaration);

    return result;
}

/**
* Add fountain particles at specified position
*/
void ParticleEmitter::AddFountainParticles(const DWORD& dNumParticles, const D3DXVECTOR3& vPos)
{
    // add new particles
    for( DWORD i = 0; i < dNumParticles; i++)
    {
        Particle p;
        p.vPos = vPos;
        
        // randomize colors
        m_ParticleList.push_back(p);
    }
}

/**
* Add particles
*/
void ParticleEmitter::AddSparkParticles(const DWORD& dNumParticles, const D3DXVECTOR3& vPos, const D3DXVECTOR3& vDir, const D3DXCOLOR& cColor)
{
    for( DWORD i = 0; i < dNumParticles; i++)
    {
        // build particle
        Particle p;
        p.vPos = vPos;
        p.fSize = 0.0025f;
        p.fLife = 0.5f;
        p.forceType = kVelocityReduction;
        
        // randomize colors (slightly)
        p.cInitColor = cColor;
        p.cFinalColor = cColor; 
        p.cFinalColor.a = 0.0f;
        p.cCurrentColor = p.cInitColor;

        // randomize particle velocity/direction
        p.vVel = vDir;
        float rangeFactor = 150.0f;
        if(p.vVel.y == 0.0f)
            p.vVel.y = sin((float)(rand()-RAND_MAX/2))/rangeFactor;
        if(p.vVel.x == 0.0f)
            p.vVel.x = sin((float)(rand()-RAND_MAX/2))/rangeFactor;
        if(p.vVel.z == 0.0f)
            p.vVel.z = sin((float)(rand()-RAND_MAX/2))/rangeFactor;
        
        p.vVel = p.vVel;
        p.vInitVel = p.vVel;

        // add particle
        m_ParticleList.push_back(p);
    }
}

/**
* Update particle emitter 
*/
void ParticleEmitter::UpdateNode(double fTime)                             
{
    // check players for updates
    for(DWORD i = 0; i < m_PlayerList.size(); i++)
    {
        float fPlayerVelocity = abs(D3DXVec3Length(&m_PlayerList[i]->GetPlayerVelocity()));

        // display white sparks if walking
        if( fPlayerVelocity > 1.0f && fPlayerVelocity < 3.0f)
            AddSparkParticles(20, m_PlayerList[i]->GetPlayerPosition(), D3DXVECTOR3(0.0f, 0.005f, 0.0f), D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
    
        // display red sparks if running
        else if( fPlayerVelocity > 3.0f )
            AddSparkParticles(30, m_PlayerList[i]->GetPlayerPosition(), D3DXVECTOR3(0.0f, 0.005f, 0.0f), D3DXCOLOR(1.0f, 0.1f, 0.1f, 1.0f));
    }

    // update existing particles
    std::list<Particle>::iterator it = m_ParticleList.begin();

    while(it != m_ParticleList.end())
    {
        Particle& p = *it;

        // update particle life
        p.fLife += fTime;

        // remove particle if at end of life
        if(p.fLife > p.fTotalLife)
        {
            std::list<Particle>::iterator itDelete = it;
            ++it;
            m_ParticleList.erase(itDelete);
            continue;
        }

        // compute acceleration  
        D3DXVECTOR3 vAccel = ComputeParticleAccel(p);
        
        // update particle velocity
        p.vVel = p.vVel + vAccel;

        // update particle position
        p.vPos = p.vPos + p.vVel;

        // update colors
        float fColorWeight = (float)(p.fLife / p.fTotalLife);
        p.cCurrentColor = p.cInitColor + (p.cFinalColor - p.cInitColor) * fColorWeight;

        // move to next particle
        ++it;
    }
}

/**
* Computes the force applied to the specified particle
*/
D3DXVECTOR3 ParticleEmitter::ComputeParticleAccel(const Particle& p)
{
    D3DXVECTOR3 vAccel = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

    switch(p.forceType)
    {
        // gravity simulator
        case kGravity:
        {
            D3DXVECTOR3 vForce = D3DXVECTOR3(0.0f, -0.01f, 0.0f);
            vAccel = vForce/p.fMass;
            break;
        }
        // deacceleration over particle lifetime
        case kVelocityReduction:
        {        
            //float fVelWeight = (float)( p.fLife / p.fTotalLife);
            vAccel = -(p.vInitVel/60);
            break;
        }
    }

    return vAccel;
}

/**
* Render particle emitter 
*/
void ParticleEmitter::RenderNode(IDirect3DDevice9* pd3dDevice, const RenderData& rData)
{
    // build vertex buffer
    CustomVertex* cvBuffer = new CustomVertex[m_ParticleList.size() * dParticleVertexCount];

    // get view x and y vectors (compute vertices that face camera)
    D3DXVECTOR3 vXView = D3DXVECTOR3(rData.matView._11, rData.matView._21, rData.matView._31);
    D3DXVECTOR3 vYView = D3DXVECTOR3(rData.matView._12, rData.matView._22, rData.matView._32);

    // form vertices for each particle
    DWORD dBufIdx = 0;
    for( std::list<Particle>::iterator it = m_ParticleList.begin(); it != m_ParticleList.end(); ++it ) 
    {
        Particle& p = *it;

        for(int i = 0; i < dParticleVertexCount; i++)
        {
            cvBuffer[dBufIdx].vPos = p.vPos;
            cvBuffer[dBufIdx].cDiffuse = p.cCurrentColor;

            switch(i)
            {
            // quad triangle 1
            case 0:
                cvBuffer[dBufIdx].vPos = cvBuffer[dBufIdx].vPos - p.fSize*vXView + p.fSize*vYView;
                break;
            case 1:
                cvBuffer[dBufIdx].vPos = cvBuffer[dBufIdx].vPos + p.fSize*vXView + p.fSize*vYView;
                break;
            case 2:
                cvBuffer[dBufIdx].vPos = cvBuffer[dBufIdx].vPos - p.fSize*vXView - p.fSize*vYView;
                break;

            // quad triangle 2
            case 3:
                cvBuffer[dBufIdx].vPos = cvBuffer[dBufIdx].vPos + p.fSize*vXView - p.fSize*vYView;
                break;
            case 4:
                cvBuffer[dBufIdx].vPos = cvBuffer[dBufIdx].vPos - p.fSize*vXView - p.fSize*vYView;
                break;
            case 5:
                cvBuffer[dBufIdx].vPos = cvBuffer[dBufIdx].vPos + p.fSize*vXView + p.fSize*vYView;
                break;
            }

            dBufIdx++;
        }
    }

    // set standard mesh transformation matrix
    pd3dDevice->SetTransform(D3DTS_WORLD, &rData.matWorld);

    // disable lighting
    pd3dDevice->SetRenderState(D3DRS_LIGHTING, false);
            
    // set vertex declaration
    pd3dDevice->SetVertexDeclaration(m_pCVDeclaration);

    // enable alpha blending
    pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
    pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTCOLOR);
    pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);

    // draw
    pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, dBufIdx/3, cvBuffer, sizeof(cvBuffer[0]) );

    // cleanup temporary vertex buffer
    SAFE_DELETE(cvBuffer);
}
