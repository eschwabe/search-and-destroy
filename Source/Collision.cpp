/**************************** COLLISION.CPP *****************************/

/*
    collision.cpp - collision data definitions and collision detection routines. 

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

#include "DXUT.h"
#include "collision.h"

const float epsilon = 0.000005f;

CollOutput  gCollOutput;

static CollSphere aCollSphere[cMobyMax];
static int iMobyLast = 0;

/*
  CollSphere::VsMobys - Tests for collisions against all mobys. 
  Returns true if a collision was found or false if not. 
 */
bool CollSphere::VsMobys()
{
  bool  ret = false;

  gCollOutput.Reset();
  for (int i = 0; i < iMobyLast; ++i)
  {
    // Don't do collision test against self. 
    if (this != &aCollSphere[i] && VsSphere(&aCollSphere[i]))
      ret = true;
  }
  return ret;
} // CollSphere::VsMobys


bool CollSphere::VsQuad(const CollQuad& quad) const
/************************************************************************/
/* Collision test between sphere and a quadrangle.                      */
/* Returns true if they collide and false if not.                       */
/************************************************************************/
{
  // First find distance from center of sphere to plane of quad
  D3DXVECTOR3    v(center - quad.point[0]);
  float t = D3DXVec3Dot(&v, &quad.normal);

  // Added epsilon because otherwise it would sometimes return a very small 
  // push that wouldn't eliminate the collision. 
  // BUGBUG: is it better to put the epsilon in the collision test as 
  // implemented, or put the add the epsilon to the push value? 
  if ((t > 0.0f) && (t + epsilon < radius))
  {     // The plane is within radius of the sphere
    D3DXVECTOR3 p(center - t*quad.normal); // Get point in plane closest to center of sphere

    if (PointInQuad(p, quad))
    {   // If that point is inside the quadrangle - you're done
      t = radius - t;
      if (t > gCollOutput.length)
      {
        gCollOutput.length = t;
        gCollOutput.normal = quad.normal;
        gCollOutput.point = p;
        gCollOutput.push = t * quad.normal;
      }
      return true;
    }
    else
    {   // If that point is outside the triangle, then we have to check each line of the
        // tri for the closest point to the center of the sphere.
      float     radSq;
      float     min;
      D3DXVECTOR3        closest;

      radSq = min = radius * radius;
      for (int i = 0; i < 4; ++i)
      {
        // Make a line for each edge of the quadrangle
        int j = i + 1;
        if (j == 4)
          j = 0;
        CollLine  line;
        line.Set(&quad.point[i], &quad.point[j]);
        // Get the point on that line closest to the sphere
        t = line.ClosestPoint(p, center);
        // Cap to within the bounds of the line
        if (t > line.length)
          p = quad.point[j];
        else if (t < 0.0f)
          p = quad.point[i];
        // How far from sphere is that point?
        v = center - p;
        t = D3DXVec3Dot(&v, &v);
        if (t < min)   // Keep track of closest point
        {
          closest = p;
          min = t;
        }
      }
      if (min < radSq)     // Was any point within radius of the sphere?
      {
        t = radius - sqrtf(min);
        if (t > gCollOutput.length)   // Update the collision data
        {
          gCollOutput.length = t;
          gCollOutput.normal = quad.normal;
          gCollOutput.point = closest;
          v = center - closest;
          D3DXVec3Normalize(&v, &v);
          gCollOutput.push = t * v;
        }
        return( TRUE );
      }
    }

    return( FALSE );
  }

  return( FALSE );
} // CollSphere::VsQuad


bool CollSphere::VsSphere(CollSphere *sphere)
/************************************************************************/
/* Collision test between sphere and another sphere.                    */
/* Returns TRUE if the 2 spheres collide and FALSE if not.              */
/************************************************************************/
{
  float d2, r2;
  D3DXVECTOR3    diff;

  diff = center - sphere->center;
  d2 = diff.x*diff.x + diff.y*diff.y + diff.z*diff.z;   // distance squared
  
  r2 = radius + sphere->radius;   // Sum of radii
  r2 = r2 * r2;                   // Sum of radii squared

  if( d2 < r2 )   // Collision has occurred
  {
    // Optimize this later - probably just save the 2 spheres and only calculate the other
    // info when asked for it.
    float length;
    length = sqrtf(r2) - sqrtf(d2);     // by how much?
    if( length > gCollOutput.length )   // Update the collision data
    {
      gCollOutput.type = COLL_SPHERE_VS_SPHERE;
      gCollOutput.length = length;
      D3DXVec3Normalize(&gCollOutput.normal, &diff);
      gCollOutput.push = gCollOutput.length * gCollOutput.normal;
      gCollOutput.point = sphere->center + sphere->radius*gCollOutput.normal;
    }
    return( TRUE );
  }

  return( FALSE );
}

void CollLine::Set(const D3DXVECTOR3* p1, const D3DXVECTOR3* p2)
/************************************************************************/
/* Set up a collision line.                                             */
/* p1 and p2 are pointers to the 2 end points.                          */
/************************************************************************/
{
  start = *p1;
  end = *p2;
  dir = end - start;
  length = D3DXVec3Length(&dir);
  dir.x /= length;    // Normalize -- since length is known this should be 
  dir.y /= length;    // faster than calling dir.Normalize()
  dir.z /= length;
}


bool CollLine::VsQuad(const CollQuad& quad) const
/************************************************************************/
/* Collision test between line and a quadrangle.                        */
/* Returns true if they collide and false if not.                       */
/************************************************************************/
{
  float denom = D3DXVec3Dot(&dir, &quad.normal);
  if (denom == 0.0f)   // The line and quadrangle are parallel
    return false;      // I'm considering parallel to be a miss

  // get the intersection of the line and the plane
  D3DXVECTOR3    diff(start - quad.point[0]);
  float t = -D3DXVec3Dot(&diff, &quad.normal);
  t = t / denom;

  if ((t > 0.0f) && (t < length))
  {   // The intesection lies between start and end on the line
    diff = t * dir;
    D3DXVECTOR3    point(start + diff);
    float len = length - t;

    if (PointInQuad(point, quad))
    {   // There is an intersection
      if (len > gCollOutput.length)   // Update the collision data
      {
        // Optimize this later - probably just save the line and quad and only calculate the other
        // info when asked for it.
        gCollOutput.type = COLL_LINE_VS_QUAD;
        gCollOutput.length = len;
        gCollOutput.point = point;
        gCollOutput.normal = quad.normal;
        gCollOutput.push = gCollOutput.point - end;
      }
      return true;
    }
  }
  return false;       // The plane is too far away from the line
} // CollLine::VsQuad


float CollLine::ClosestPoint(D3DXVECTOR3& p, const D3DXVECTOR3& c) const
/************************************************************************/
/* Sets p to the point on the line closest to point c.                  */
/* Returns how far along the line that point is with 0.0 being the      */
/* start point and length being the end point.                          */
/************************************************************************/
{
  D3DXVECTOR3      v(c - start);
  float   t = D3DXVec3Dot(&v, &dir);
  p = (t * dir) + start;

  return( t );
}


bool PointInQuad(const D3DXVECTOR3& p3, const CollQuad& quad)
/************************************************************************/
/* Check whether the point p3 is inside the quadrangle quad.            */
/* Returns true if the p3 is in quad or false if not.                   */
/* Does not check whether p3 lies within the plane of quad, only whether*/
/* the projection of p3 onto that plane is inside the quadrangle quad.  */
/************************************************************************/
{
  for (int i = 0; i < 4; ++i)
  {
    int j = i + 1;
    if (j == 4)
      j = 0;
    D3DXVECTOR3  diff(p3 - quad.point[i]);
    D3DXVECTOR3  diffTri(quad.point[j] - quad.point[i]);
    D3DXVec3Cross(&diff, &diffTri, &diff);
    float dot = D3DXVec3Dot(&diff, &quad.normal);
    if (dot < 0.0f)           // The point is outside the quadrangle 
      return false;
  }
  return true;
} // PointInQuad

/*
  AppendMobyCollSphere - Copy data to Moby CollSphere array, return 
  pointer to array entry where we've stored the data. 
 */ 
CollSphere* AppendMobyCollSphere(CollSphere& sphere)
{
  assert(iMobyLast < cMobyMax);         // trying to add too many CollSpheres
  aCollSphere[iMobyLast++] = sphere;
  return &aCollSphere[iMobyLast - 1];
} // AppendMobyCollSphere


/************************************************************************/
/* Game Object Collision                                                */
/************************************************************************/

/**
* Constructor
*/
GameObjectCollision::GameObjectCollision(const VecCollQuad& quads) :
    m_vQuadList(quads)
{}
 
/**
* Run collision checks between object and environment.
*/
void GameObjectCollision::RunWorldCollision(GameObject* obj)
{
    assert(obj);

    // get object position
    D3DXVECTOR3 vObjPos = obj->GetPosition();

    // generate sphere from player position and height
    CollSphere sphere;
    sphere.Set(&vObjPos, 0.25f);

    // run sphere vs quad checks on entire list
    for(size_t i = 0; i < m_vQuadList.size(); i++)
    {
        // check for collision
        if(sphere.VsQuad(m_vQuadList[i]))
        {
            // if collision, send player collision event
            obj->SetPosition( obj->GetPosition() + gCollOutput.push );
            gCollOutput.Reset();
        }
    }  
}

/**
* Run collision checks between two objects. Returns true if a collision occured.
*/
bool GameObjectCollision::RunObjectCollision(GameObject* obj1, GameObject* obj2)
{
    assert(obj1);
    assert(obj2);

    // generate sphere from position and height
    CollSphere obj1Sphere;
    D3DXVECTOR3 vObj1Pos = obj1->GetPosition();
    obj1Sphere.Set(&vObj1Pos, 0.25f);

    // generate sphere from player position and height
    CollSphere obj2Sphere;
    D3DXVECTOR3 vObj2Pos = obj2->GetPosition();
    obj2Sphere.Set(&vObj2Pos, 0.25f);

    bool coll = obj1Sphere.VsSphere(&obj2Sphere);

    // notify players of collision result
    if(coll)
    {
        obj1->SetPosition( obj1->GetPosition() + gCollOutput.push );
        gCollOutput.Reset();
    }

    return coll;
}

/**
* Run line collision check between line and environment. Returns
* true if a collision occured.
*/
bool GameObjectCollision::RunLineCollision(const D3DXVECTOR3& p1, const D3DXVECTOR3& p2, CollOutput* output)
{
    assert(output);

    bool coll = false;

    // run sphere vs quad checks on entire list
    for(size_t i = 0; i < m_vQuadList.size(); i++)
    {
        // generate collision line
        CollLine line;
        line.Set(&p1, &p2);

        // check for collision
        if(line.VsQuad(m_vQuadList[i]))
        {
            // if collision, modify line end point
            *output = gCollOutput;
            gCollOutput.Reset();
            coll = true;
        }
    }

    return coll;
}
