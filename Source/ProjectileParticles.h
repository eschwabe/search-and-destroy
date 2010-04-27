/*******************************************************************************
* Game Development Project
* ParticleEmitter.h
*
* Eric Schwabe
* 2010-03-07
*
* Projectile particles
*
*******************************************************************************/

#pragma once
#include "gameobject.h"
#include <list>

class ProjectileParticles : public GameObject
{
    public:

        enum ParticleType
        {
            kFire,
            kLightBall,
            kBigLightBall
        };

        // constructors
        ProjectileParticles(const D3DXVECTOR3& vPos, const ParticleType& type);
        virtual ~ProjectileParticles();

    protected:

        // object methods
        virtual HRESULT Initialize(IDirect3DDevice9* pd3dDevice);
	    virtual void Update();
	    virtual void Render(IDirect3DDevice9*, const RenderData* rData);

    private:

	    /**
        * Custom vertex type. Specifies a custom vertex that can be written
        * to the verticies buffer for rendering.
        */
	    struct CustomVertex
	    {
            D3DXVECTOR3 vPos;       // untransformed, 3D position for the vertex
            D3DXVECTOR2 vTexCoord;  // texture coordinates
            D3DCOLOR cDiffuse;      // diffuse color

            // default constructor
            CustomVertex() :
                vPos(0.0f, 0.0f, 0.0f),
                vTexCoord(0.0f, 0.0f),
                cDiffuse(D3DXCOLOR(0.0f, 0.0f, 0.0f, 1))
            {}

            // initialization constructor
            CustomVertex(const D3DXVECTOR3& ivPos, const D3DXVECTOR3& ivTex, const D3DXCOLOR& iColor) :
                vPos(ivPos),
                vTexCoord(ivTex),
                cDiffuse(iColor)
            {}
	    };

        static const D3DVERTEXELEMENT9 m_sCustomVertexDeclaration[];    // custom vertex structure definition
        LPDIRECT3DVERTEXDECLARATION9 m_pCVDeclaration;                  // custom vertex declaration

        /**
        * Particle data
        */
        struct Particle
        {
            D3DXVECTOR3 vPos;           // position
            D3DXVECTOR3 vVel;           // current velocity
            D3DXVECTOR3 vInitVel;       // initial velocity
            float fSize;                // size
            float fMass;                // mass
            double fLife;               // current life (seconds)
            double fTotalLife;          // total life (seconds)
            D3DXCOLOR cInitColor;       // initial color
            D3DXCOLOR cFinalColor;      // final color
            D3DXCOLOR cCurrentColor;    // current color

            // default constructor
            Particle() :
                vPos(D3DXVECTOR3(0.0f, 0.0f, 0.0f)),
                vVel(D3DXVECTOR3(0.0f, 0.0f, 0.0f)),
                vInitVel(vVel),
                fSize(1.0f),
                fMass(1.0f),
                fTotalLife(1.0f),
                fLife(0.0f),
                cInitColor(D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f)),
                cFinalColor(D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.0f)),
                cCurrentColor(cInitColor)
            {}
        };

        // METHODS
        D3DXVECTOR3 ComputeParticleAccel(const Particle& p);
        void CreateParticles(const DWORD& dNumParticles);


        // DATA
        static const DWORD dParticleVertexCount = 6;
        
        std::list<Particle> m_pList;            // particle list
        ParticleType m_pType;                   // particle type
        double m_fLastParticleCreateTime;       // last time a particle was created for source

        LPDIRECT3DTEXTURE9 m_pParticleTexture;  // particle texture
        std::wstring m_sParticleFilename;       // particle texture filename


        // prevent copy and assignment
        ProjectileParticles(const ProjectileParticles&);
        void operator=(const ProjectileParticles&);
};
