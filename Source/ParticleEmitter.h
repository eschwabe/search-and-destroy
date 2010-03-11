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
#include "Node.h"
#include "PlayerNode.h"
#include <list>

class ParticleEmitter : public Node
{
    public:

        enum ForceType
        {
            kGravity,
            kVelocityReduction
        };

        // constructors
        ParticleEmitter();
        virtual ~ParticleEmitter();

        // player tracking
        void AddPlayerTracking(const PlayerNode* player);

        // particle management
        void AddFountainParticles(const DWORD& dNumParticles, const D3DXVECTOR3& vPos);
        void AddSparkParticles(const DWORD& dNumParticles, const D3DXVECTOR3& vPos, const D3DXVECTOR3& vDir, const D3DXCOLOR& cColor);

    protected:

        HRESULT InitializeNode(IDirect3DDevice9* pd3dDevice);                   // initialize world node
	    void UpdateNode(double fTime);                                          // update traversal for physics, AI, etc.
	    void RenderNode(IDirect3DDevice9* pd3dDevice, const RenderData& rData); // render traversal for drawing objects

    private:

	    /**
        * Custom vertex type. Specifies a custom vertex that can be written
        * to the verticies buffer for rendering.
        */
	    struct CustomVertex
	    {
            D3DXVECTOR3 vPos;       // untransformed, 3D position for the vertex
            D3DCOLOR cDiffuse;      // diffuse color

            // default constructor
            CustomVertex() :
                vPos(0.0f, 0.0f, 0.0f), 
                cDiffuse(D3DXCOLOR(0.0f, 0.0f, 0.0f, 1))
            {}

            // initialization constructor
            CustomVertex(const D3DXVECTOR3& ivPos, const D3DXCOLOR& iColor) :
                vPos(ivPos), 
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
            ForceType forceType;        // type of force applied to particle
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
                vVel(D3DXVECTOR3( sin((float)rand())/10.0f, 0.2f*((float)rand())/RAND_MAX+0.2f, cos((float)rand())/10.0f)),
                vInitVel(vVel),
                forceType(kGravity),
                fSize(0.05f),
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

        // DATA
        static const DWORD dParticleVertexCount = 6;
        std::list<Particle> m_ParticleList;             // list of particles
        std::vector<const PlayerNode*> m_PlayerList;    // list of players


        // prevent copy and assignment
        ParticleEmitter(const ParticleEmitter&);
        void operator=(const ParticleEmitter&);
};
