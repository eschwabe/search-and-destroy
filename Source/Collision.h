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
        char  VsSphere(CollSphere *sphere);
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
            
        CollQuad(const D3DXVECTOR3& p1, const D3DXVECTOR3& p2, const D3DXVECTOR3& p3, const D3DXVECTOR3& p4)
        {
            Set(p1, p2, p3, p4);
        }

        // set quad points and normal
        // normal is computed using first three points (N = UxV, U=p2-p1 V=p3-p1)
        void Set(const D3DXVECTOR3& p1, const D3DXVECTOR3& p2, const D3DXVECTOR3& p3, const D3DXVECTOR3& p4)
        {
            point[0] = p1;  point[1] = p2;  point[2] = p3;  point[3] = p4;

            D3DXVECTOR3 u = p2-p1;
            D3DXVECTOR3 v = p3-p1;
            D3DXVec3Cross(&normal, &u, &v);
        };
};

/**
* Collision Quad List
*/
typedef std::vector<CollQuad> VecCollQuad;

/**
* Checks for collisions between a list of quads (environment) and the player.
* When collisions are detected, the player is notified to move.
*/
class CollPlayer
{
    public:

        // constructor
        CollPlayer();

        // run collision checks between player and environment
        void RunCollisionCheck(PlayerNode* player, const VecCollQuad& quads);
    
    private:

        // prevent copy and assignment
        CollPlayer(const CollPlayer&);
        void operator=(const CollPlayer&);
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
