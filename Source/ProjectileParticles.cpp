/*******************************************************************************
* Game Development Project
* ParticleEmitter.cpp
*
* Eric Schwabe
* 2010-03-07
*
* Projectile particles
*
*******************************************************************************/

#include "DXUT.h"
#include "DXUT\SDKmisc.h"
#include "ProjectileParticles.h"

// custom vertex structure definition
const D3DVERTEXELEMENT9 ProjectileParticles::m_sCustomVertexDeclaration[] =
{
    { 0, offsetof(CustomVertex, vPos        ), D3DDECLTYPE_FLOAT3  , 0, D3DDECLUSAGE_POSITION, 0 },
    { 0, offsetof(CustomVertex, vTexCoord   ), D3DDECLTYPE_FLOAT2  , 0, D3DDECLUSAGE_TEXCOORD, 0 },
    { 0, offsetof(CustomVertex, cDiffuse    ), D3DDECLTYPE_D3DCOLOR, 0, D3DDECLUSAGE_COLOR   , 0 },
    D3DDECL_END(),
};

/**
* Constructor 
*/
ProjectileParticles::ProjectileParticles(const D3DXVECTOR3& vPos, const ParticleType& type) :
    GameObject(g_database.GetNewObjectID(), OBJECT_Projectile, "PROJECTILE"),
    m_sParticleFilename(L"particle-point.png"),
    m_pType(type),
    m_fLastParticleCreateTime(0.0f),
    m_bExploded(false)
{
    // set initial position
    m_vPos = vPos;
    m_vResetPos = vPos;

    // set height
    if(type == kLightBall)
        m_fHeight = 0.1f;
    else
        m_fHeight = 0.5f;
}

/**
* Deconstructor 
*/
ProjectileParticles::~ProjectileParticles()
{
    // cleanup vertex declaration
    SAFE_RELEASE(m_pCVDeclaration);

    // cleanup texture
    SAFE_RELEASE(m_pParticleTexture);
}

/**
* Initialize particle emitter 
*/
HRESULT ProjectileParticles::Initialize(IDirect3DDevice9* pd3dDevice)
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
* Update projectile particles 
*/
void ProjectileParticles::Update()                             
{
    // if exploded, do not add particles
    if(m_bExploded)
    {
        // nothing
    }    
    
    // else if projectile stopped, update particles to explode
    else if(m_dHealth <= 0)
    {
        UpdateParticlesExplode();
        m_bExploded = true;
    }

    // else if enough time passed, create new particles
    else if(g_time.GetCurTime() - m_fLastParticleCreateTime > 0.1f)
    {
        CreateParticles(3);
        m_fLastParticleCreateTime = g_time.GetCurTime();
    }

    // update existing particles
    std::list<Particle>::iterator itParticle = m_pList.begin();

    // update each particle
    while(itParticle != m_pList.end())
    {
        Particle& p = *itParticle;

        // update particle life
        p.fLife += g_time.GetElapsedTime();

        // remove particle if at end of life
        if(p.fLife > p.fTotalLife)
        {
            std::list<Particle>::iterator itDelete = itParticle;
            ++itParticle;
            m_pList.erase(itDelete);
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
        ++itParticle;
    }

    // update object position
    UpdateObjectPosition();
}

/**
* Add and update all particles to create explosion effect
*/
void ProjectileParticles::UpdateParticlesExplode()
{
    // add new particles
    CreateParticles(75);

    // update existing particles
    std::list<Particle>::iterator itParticle = m_pList.begin();

    // update each particle
    while(itParticle != m_pList.end())
    {
        Particle& p = *itParticle;

        // update particle data to create explosion effect
        p.fSize = 0.025f;
        float rangeFactor = (float)(RAND_MAX*20);
        p.vVel.x = (float)(rand()-RAND_MAX/2)/rangeFactor;
        p.vVel.y = (float)(rand()-RAND_MAX/2)/rangeFactor;
        p.vVel.z = (float)(rand()-RAND_MAX/2)/rangeFactor;

        // move to next particle
        ++itParticle;
    }
}

/**
* Create more particles
*/
void ProjectileParticles::CreateParticles(const DWORD& dNumParticles)
{
    for( DWORD i = 0; i < dNumParticles; i++)
    {
        // build particle
        Particle p;

        // particle float distance
        float rangeFactor = 100.0f;

        // customize particle
        switch(m_pType)
        {
            // small projectile ball
            case kLightBall:
                rangeFactor = 1000.0f;
                p.fSize = 0.1f;
                p.vVel = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
                p.cInitColor = D3DXCOLOR(0.1f, 0.3f, 1.0f, 1.0f);
                break;

            // large projectile ball
            case kBigLightBall:
                rangeFactor = 500.0f;
                p.fSize = 0.3f;
                p.vVel = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
                p.cInitColor = D3DXCOLOR(0.2f, 0.4f, 1.0f, 1.0f);
                break;

            // large fire ball
            default:
                p.fSize = 0.5f;
                p.vVel = D3DXVECTOR3(0.0f, 0.02f, 0.0f);
                p.cInitColor = D3DXCOLOR(1.0f, 0.3f, 0.1f, 1.0f);
                break;
        }

        // randomize particle velocity/direction
        p.vVel.x = sin((float)(rand()-RAND_MAX/2))/rangeFactor;
        p.vVel.z = sin((float)(rand()-RAND_MAX/2))/rangeFactor;
        p.vInitVel = p.vVel;

        // set color changes
        p.cFinalColor = p.cInitColor; 
        p.cCurrentColor = p.cInitColor;

        // add particle
        m_pList.push_back(p);
    }
}

/**
* Computes the force applied to the specified particle
*/
D3DXVECTOR3 ProjectileParticles::ComputeParticleAccel(const Particle& p)
{
    D3DXVECTOR3 vAccel = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

    // rising particles
    vAccel = -(p.vInitVel/60);

    return vAccel;
}

/**
* Render particle emitter 
*/
void ProjectileParticles::Render(IDirect3DDevice9* pd3dDevice, const RenderData* rData)
{
    // determine number of particles
    DWORD dTotalNumParticles = m_pList.size();

    // build vertex buffer
    CustomVertex* cvBuffer = new CustomVertex[dTotalNumParticles * dParticleVertexCount];

    // get camera view x and y vectors (compute vertices that face camera)
    D3DXVECTOR3 vXView = D3DXVECTOR3(rData->matView._11, rData->matView._21, rData->matView._31);
    D3DXVECTOR3 vYView = D3DXVECTOR3(rData->matView._12, rData->matView._22, rData->matView._32);

    // form vertices for each particle
    DWORD dBufIdx = 0;

    for( std::list<Particle>::iterator itParticle = m_pList.begin(); itParticle != m_pList.end(); ++itParticle ) 
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

    if(dBufIdx > 0)
    {
        // translate projectile
        D3DXMATRIX mxTranslate;
        D3DXMatrixTranslation(&mxTranslate, m_vPos.x, m_vPos.y, m_vPos.z);

        // set standard mesh transformation matrix
        D3DXMATRIX mxWorld = rData->matWorld * mxTranslate;
        pd3dDevice->SetTransform(D3DTS_WORLD, &mxWorld);

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
