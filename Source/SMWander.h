/*******************************************************************************
* Game Development Project
* SMWander.cpp
*
* Eric Schwabe
* 2010-04-24
*
* Wander state machine
*
*******************************************************************************/

#pragma once
#include "statemch.h"
#include "collision.h"

class SMWander : public StateMachine
{
    public:

        SMWander(GameObject* object, GameObjectCollision* coll);
        virtual ~SMWander();

    private:

        virtual bool States( State_Machine_Event event, MSG_Object* msg, int state, int substate );

        // helper functions
        void UpdateFeelers();
        D3DXVECTOR3 RotateVector(const D3DXVECTOR3& vVec, const float& fYaw);


        const float kFrontFeelerLength;     // length of feeler sensor
        GameObjectCollision* m_objColl;     // object collision
        D3DXVECTOR3 m_vFrontFeelerPos;      // 
};
