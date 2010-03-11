/**************************** COLLISION.H *******************************/

/*
    Copyright (c) 2007 Dan Chang. All rights reserved. 

    This software is provided "as is" without express or implied 
    warranties. You may freely copy and compile this source into 
    applications you distribute provided that the copyright text 
    below is included in the resulting source code, for example: 
    "Portions Copyright (c) 2007 Dan Chang" 
*/

/*
    Modifications by Eric Schwabe
*/

#pragma once

#include <vector>
#include "PlayerNode.h"
#include "ParticleEmitter.h"
#include "WorldDecalNode.h"

/************************************************************************/
/* CONSTANTS                                                            */
/************************************************************************/

enum COLL_ENUM
{
  COLL_NONE,                // 0
  COLL_SPHERE_VS_SPHERE,
  COLL_SPHERE_VS_TRI,
  COLL_LINE_VS_QUAD, 
  COLL_LINE_VS_SPHERE,
  COLL_LINE_VS_TRI,

  COLL_TOTAL
};

const int cMobyMax  = 64;               // never more than cMobyMax active

/************************************************************************/
/* CLASSES & STRUCTURES                                                 */
/************************************************************************/


/**
* Colision Output
*/
class CollOutput
{
    public:

        char        type;
        float       length;
        D3DXVECTOR3 normal;
        D3DXVECTOR3 push;
        D3DXVECTOR3 point;

        void Reset()
        {
            type = COLL_NONE;
            length = -1.0f;
        };
};

/**
* Collision Sphere
*/
struct _CollSphere
{
    D3DVECTOR center;
    float radius;
};

class CollSphere
{
    public:

        D3DXVECTOR3 center;
        float radius;

        void Set(const D3DVECTOR *p, const float r)    
        { 
            center = *p; 
            radius = r;
        };
        void Set(const float cx, const float cy, const float cz, const float r)
        {
            center.x = cx;
            center.y = cy;
            center.z = cz;
            radius = r;
        };

        bool  VsEnv();
        bool  VsMobys();

        bool  VsQuad(const class CollQuad& quad) const;
        bool  VsSphere(CollSphere *sphere);
        bool  VsTri(const class CollTri& tri) const;
};

/**
* Collision Line
*/
class CollLine
{
    public:

        D3DXVECTOR3 start;
        D3DXVECTOR3 end;
        D3DXVECTOR3 dir;
        float length;

        void  Set(const D3DXVECTOR3* s, const D3DXVECTOR3* e);
        void  Set(D3DXVECTOR3 *p, D3DXVECTOR3 *v, float l);

        bool  VsEnv();

        bool  VsQuad(const class CollQuad& quad) const;
        char  VsSphere(CollSphere *sphere);
        bool  VsTri(const class CollTri& tri) const;
        float ClosestPoint(D3DXVECTOR3& p, const D3DXVECTOR3& c) const;
};

/**
* Collsiion Quad
*/
struct _CollQuad
{
    D3DXVECTOR3 point[4];
    D3DXVECTOR3 normal;
};

class CollQuad
{
    public:
        D3DXVECTOR3 point[4];
        D3DXVECTOR3 normal;

        // constructor
        CollQuad() {}
            
        CollQuad(const D3DXVECTOR3& p1, const D3DXVECTOR3& p2, const D3DXVECTOR3& p3, const D3DXVECTOR3& p4,const D3DXVECTOR3& n)
        {
            Set(p1, p2, p3, p4, n);
        }

        // set quad points and normal
        void Set(const D3DXVECTOR3& p1, const D3DXVECTOR3& p2, const D3DXVECTOR3& p3, const D3DXVECTOR3& p4, const D3DXVECTOR3& n)
        {
            point[0] = p1;  point[1] = p2;  point[2] = p3;  point[3] = p4; normal = n;
        };
};

/**
* Collision Quad List
*/
typedef std::vector<CollQuad> VecCollQuad;

/**
* Checks for collisions between a list of quads (environment) and the player.
* When collisions are detected, the player is notified to move. Also, can check
* for collisions between two players and move them.
*/
class CollPlayer
{
    public:

        // constructor
        CollPlayer();

        // run collision checks between player and environment
        void RunWorldCollision(const VecCollQuad& quads, PlayerNode* player, ParticleEmitter* pEmitter, WorldDecalNode* pDecal);

        // run collision checks between two players
        bool RunPlayerCollision(PlayerNode* player1, PlayerNode* player2);
    
    private:

        // prevent copy and assignment
        CollPlayer(const CollPlayer&);
        void operator=(const CollPlayer&);
};

/**
* Checks for line collisions against a list of quads. Modifies the
* end of the line to the nearest collision point.
*/
class CollLineOfSight
{
    public:

        // constructor
        CollLineOfSight() {}

        // run line of sight collision check
        bool RunLineOfSightCollision(const VecCollQuad& quads, const D3DXVECTOR3& p1, D3DXVECTOR3& p2);

    private:

        // prevent copy and assignment
        CollLineOfSight(const CollLineOfSight&);
        void operator=(const CollLineOfSight&);
};

/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/

bool PointInQuad(const D3DXVECTOR3& D3DXVECTOR3, const CollQuad& quad);

CollSphere* AppendMobyCollSphere(CollSphere& sphere);

/************************************************************************/
/* EXTERNS                                                              */
/************************************************************************/

extern CollOutput gCollOutput; // collision output data
