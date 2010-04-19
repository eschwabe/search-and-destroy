/*******************************************************************************
* Game Development Project
* ParticleEmitter.h
*
* Eric Schwabe
* 2010-03-07
*
* Particle Emitter
*
*******************************************************************************/

#pragma once
#include "gameobject.h"
#include <list>

class ParticleEmitter : public GameObject
{
    public:

        enum ParticleType
        {
            kFire,
            kFountain
        };

        // constructors
        ParticleEmitter(const LPCWSTR sParticleFilename);
        virtual ~ParticleEmitter();

        // particle management
        void EnableParticles(const ParticleType& type, const D3DXVECTOR3& vPos);
        void DisableParticles(const ParticleType& type);

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

        /**
        * Particle Management
        */
        struct ParticleSource
        {
            std::list<Particle> pList;
            ParticleType pType;                 // type of particle
            D3DXVECTOR3 vPos;                   // source position
            double fLastParticleCreateTime;     // last time a particle was created for source
        };


        // METHODS
        D3DXVECTOR3 ComputeParticleAccel(const ParticleType& type, const Particle& p);
        void UpdateParticleSources();
        void AddFountainParticles(const DWORD& dNumParticles, ParticleSource& source);
        void AddFireParticles(const DWORD& dNumParticles, ParticleSource& source);


        // DATA
        static const DWORD dParticleVertexCount = 6;
        std::list<ParticleSource> m_ParticleSourceList;   // list of particle sources

        LPDIRECT3DTEXTURE9 m_pParticleTexture;
        std::wstring m_sParticleFilename;


        // prevent copy and assignment
        ParticleEmitter(const ParticleEmitter&);
        void operator=(const ParticleEmitter&);
};
