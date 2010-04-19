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
#include "DXUT\SDKmisc.h"

// custom vertex structure definition
const D3DVERTEXELEMENT9 ParticleEmitter::m_sCustomVertexDeclaration[] =
{
    { 0, offsetof(CustomVertex, vPos        ), D3DDECLTYPE_FLOAT3  , 0, D3DDECLUSAGE_POSITION, 0 },
    { 0, offsetof(CustomVertex, vTexCoord   ), D3DDECLTYPE_FLOAT2  , 0, D3DDECLUSAGE_TEXCOORD, 0 },
    { 0, offsetof(CustomVertex, cDiffuse    ), D3DDECLTYPE_D3DCOLOR, 0, D3DDECLUSAGE_COLOR   , 0 },
    D3DDECL_END(),
};

/**
* Constructor 
*/
ParticleEmitter::ParticleEmitter(const LPCWSTR sParticleFilename) :
    GameObject(g_database.GetNewObjectID(), OBJECT_Projectile, "PROJECTILE"),
    m_sParticleFilename(sParticleFilename)
{
}

/**
* Deconstructor 
*/
ParticleEmitter::~ParticleEmitter()
{
    // cleanup vertex declaration
    SAFE_RELEASE(m_pCVDeclaration);

    // cleanup texture
    SAFE_RELEASE(m_pParticleTexture);
}

/**
* Initialize particle emitter 
*/
HRESULT ParticleEmitter::Initialize(IDirect3DDevice9* pd3dDevice)
{
    // create vertex declaration
    HRESULT result = pd3dDevice->CreateVertexDeclaration(m_sCustomVertexDeclaration, &m_pCVDeclaration);

    // search and load particle texture
    if( SUCCEEDED(result) )
    {
        WCHAR wsNewPath[ MAX_PATH ];
        DXUTFindDXSDKMediaFileCch(wsNewPath, sizeof(wsNewPath), m_sParticleFilename.c_str());
        result = D3DXCreateTextureFromFile(pd3dDevice, wsNewPath, &m_pParticleTexture);
    }

    return result;
}

/**
* Add particle source
*/
void ParticleEmitter::EnableParticles(const ParticleType& type, const D3DXVECTOR3& vPos)
{
    ParticleSource source;
    source.vPos = vPos;
    source.pType = type;
    source.fLastParticleCreateTime = 0.0f;
    m_ParticleSourceList.push_back(source);
}

/**
* Disable particle sources for specified type
*/
void ParticleEmitter::DisableParticles(const ParticleType& type)
{
    // check each particle source
    std::list<ParticleSource>::iterator it = m_ParticleSourceList.begin();

    while(it != m_ParticleSourceList.end())
    {
        ParticleSource& source = (*it);

        // remove particle if at end of life
        if(source.pType == type)
        {
            std::list<ParticleSource>::iterator itDelete = it;
            ++it;
            m_ParticleSourceList.erase(itDelete);
            continue;
        }
        ++it;
    }
}

/**
* Update particle sources
*/
void ParticleEmitter::UpdateParticleSources()
{
    // update existing particles in all sources
    for(std::list<ParticleSource>::iterator itSource = m_ParticleSourceList.begin();
        itSource != m_ParticleSourceList.end();
        ++itSource)
    {
        ParticleSource& source = (*itSource);

        if( source.pType == kFountain )
        {
            // fountain particle source
            AddFountainParticles(10, source);
        }
        else if( source.pType == kFire )
        {
            // fire particle source
            if(g_time.GetElapsedTime() - source.fLastParticleCreateTime > 0.1f)
            {
                AddFireParticles(1, source);
                source.fLastParticleCreateTime = g_time.GetAbsoluteTime();
            }
        }
        else
        {
            // ignore
        }
    }
}

/**
* Add fountain particles at specified position
*/
void ParticleEmitter::AddFountainParticles(const DWORD& dNumParticles, ParticleSource& source)
{
    // add new particles
    for( DWORD i = 0; i < dNumParticles; i++)
    {
        Particle p;
        p.vPos = source.vPos;
        p.vVel = D3DXVECTOR3( sin((float)rand())/10.0f, 0.2f*((float)rand())/RAND_MAX+0.2f, cos((float)rand())/10.0f);
        p.fSize = 0.1f;
        source.pList.push_back(p);  
    }
}

/**
* Add particles
*/
void ParticleEmitter::AddFireParticles(const DWORD& dNumParticles, ParticleSource& source)
{
    for( DWORD i = 0; i < dNumParticles; i++)
    {
        // build particle
        Particle p;
        p.vPos = source.vPos;
        p.fSize = 0.5f;
        
        // set fire color (orange)
        //p.cInitColor = D3DXCOLOR(1.0f, 0.5f, 0.2f, 1.0f);
        p.cInitColor = D3DXCOLOR(1.0f, 0.3f, 0.1f, 1.0f);
        p.cFinalColor = p.cInitColor; 
        p.cCurrentColor = p.cInitColor;

        // randomize particle velocity/direction
        p.vVel = D3DXVECTOR3(0.0f, 0.02f, 0.0f);
        
        float rangeFactor = 100.0f;
        p.vVel.x = sin((float)(rand()-RAND_MAX/2))/rangeFactor;
        p.vVel.z = sin((float)(rand()-RAND_MAX/2))/rangeFactor;
        
        p.vInitVel = p.vVel;

        // add particle
        source.pList.push_back(p);
    }
}

/**
* Update particle emitter 
*/
void ParticleEmitter::Update()                             
{
    // add particles for sources
    UpdateParticleSources();

    // update existing particles in all sources
    for(std::list<ParticleSource>::iterator itSource = m_ParticleSourceList.begin();
        itSource != m_ParticleSourceList.end();
        ++itSource)
    {
        ParticleSource& source = (*itSource);
        std::list<Particle>::iterator itParticle = source.pList.begin();

        // update each particle
        while(itParticle != source.pList.end())
        {
            Particle& p = *itParticle;

            // update particle life
            p.fLife += g_time.GetElapsedTime();

            // remove particle if at end of life
            if(p.fLife > p.fTotalLife)
            {
                std::list<Particle>::iterator itDelete = itParticle;
                ++itParticle;
                source.pList.erase(itDelete);
                continue;
            }

            // compute acceleration  
            D3DXVECTOR3 vAccel = ComputeParticleAccel(source.pType, p);
            
            // update particle velocity
            p.vVel = p.vVel + vAccel;

            // update particle position
            p.vPos = p.vPos + p.vVel;

            // update colors
            float fColorWeight = (float)(p.fLife / p.fTotalLife);
            p.cCurrentColor = p.cInitColor + (p.cFinalColor - p.cInitColor) * fColorWeight;

            // move to next particle
            ++itParticle;
        }
    }
}

/**
* Computes the force applied to the specified particle
*/
D3DXVECTOR3 ParticleEmitter::ComputeParticleAccel(const ParticleType& type, const Particle& p)
{
    D3DXVECTOR3 vAccel = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

    switch(type)
    {
        case kFountain:
        {
            // gravity simulator
            D3DXVECTOR3 vForce = D3DXVECTOR3(0.0f, -0.01f, 0.0f);
            vAccel = vForce/p.fMass;
            break;
        }
        case kFire:
        {        
            // rising particles
            vAccel = -(p.vInitVel/60);
            break;
        }
    }

    return vAccel;
}

/**
* Render particle emitter 
*/
void ParticleEmitter::Render(IDirect3DDevice9* pd3dDevice, const RenderData* rData)
{
    // determine number of particles
    DWORD dTotalNumParticles = 0;
    for(std::list<ParticleSource>::iterator itSource = m_ParticleSourceList.begin();
        itSource != m_ParticleSourceList.end();
        ++itSource)
    {
        dTotalNumParticles += (*itSource).pList.size();
    }

    // build vertex buffer
    CustomVertex* cvBuffer = new CustomVertex[dTotalNumParticles * dParticleVertexCount];

    // get camera view x and y vectors (compute vertices that face camera)
    D3DXVECTOR3 vXView = D3DXVECTOR3(rData->matView._11, rData->matView._21, rData->matView._31);
    D3DXVECTOR3 vYView = D3DXVECTOR3(rData->matView._12, rData->matView._22, rData->matView._32);

    // form vertices for each particle in each source
    DWORD dBufIdx = 0;
    for(std::list<ParticleSource>::iterator itSource = m_ParticleSourceList.begin();
        itSource != m_ParticleSourceList.end();
        ++itSource)
    {
        for( std::list<Particle>::iterator itParticle = (*itSource).pList.begin(); 
            itParticle != (*itSource).pList.end(); 
            ++itParticle ) 
        {
            Particle& p = (*itParticle);

            for(int i = 0; i < dParticleVertexCount; i++)
            {
                cvBuffer[dBufIdx].vPos = p.vPos;
                cvBuffer[dBufIdx].cDiffuse = p.cCurrentColor;

                switch(i)
                {
                // quad triangle 1
                case 0:
                    cvBuffer[dBufIdx].vPos = cvBuffer[dBufIdx].vPos - p.fSize*vXView - p.fSize*vYView;
                    cvBuffer[dBufIdx].vTexCoord = D3DXVECTOR2(1.0f, 0.0f);
                    break;
                case 1:
                    cvBuffer[dBufIdx].vPos = cvBuffer[dBufIdx].vPos - p.fSize*vXView + p.fSize*vYView;
                    cvBuffer[dBufIdx].vTexCoord = D3DXVECTOR2(0.0f, 0.0f);
                    break;
                case 2:
                    cvBuffer[dBufIdx].vPos = cvBuffer[dBufIdx].vPos + p.fSize*vXView + p.fSize*vYView;
                    cvBuffer[dBufIdx].vTexCoord = D3DXVECTOR2(0.0f, 1.0f);
                    break;

                // quad triangle 2
                case 3:
                    cvBuffer[dBufIdx].vPos = cvBuffer[dBufIdx].vPos - p.fSize*vXView - p.fSize*vYView;
                    cvBuffer[dBufIdx].vTexCoord = D3DXVECTOR2(1.0f, 0.0f);
                    break;
                case 4:
                    cvBuffer[dBufIdx].vPos = cvBuffer[dBufIdx].vPos + p.fSize*vXView + p.fSize*vYView;
                    cvBuffer[dBufIdx].vTexCoord = D3DXVECTOR2(0.0f, 1.0f);
                    break;
                case 5:
                    cvBuffer[dBufIdx].vPos = cvBuffer[dBufIdx].vPos + p.fSize*vXView - p.fSize*vYView;
                    cvBuffer[dBufIdx].vTexCoord = D3DXVECTOR2(1.0f, 1.0f);
                    break;
                }

                dBufIdx++;
            }
        }
    }

    if(dBufIdx > 0)
    {
        // set standard mesh transformation matrix
        pd3dDevice->SetTransform(D3DTS_WORLD, &rData->matWorld);

        // disable lighting
        pd3dDevice->SetRenderState(D3DRS_LIGHTING, false);
                
        // set particle texture
        pd3dDevice->SetTexture(0, m_pParticleTexture);

        // set vertex declaration
        pd3dDevice->SetVertexDeclaration(m_pCVDeclaration);

        // enable alpha blending
        pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
        
        // disable z buffer
        pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

        // draw

        pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, dBufIdx/3, cvBuffer, sizeof(cvBuffer[0]) );
    }

    // cleanup temporary vertex buffer
    SAFE_DELETE_ARRAY(cvBuffer);
}
