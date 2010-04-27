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

class SMWander : public StateMachine
{
    public:

        SMWander(GameObject* object);
        virtual ~SMWander();

    private:

        virtual bool States( State_Machine_Event event, MSG_Object* msg, int state, int substate );

        // helper functions
        void UpdateFeelers();
        D3DXVECTOR3 RotateVector(const D3DXVECTOR3& vVec, const float& fYaw);


        const float kFrontFeelerLength;     // length of feeler sensor
        D3DXVECTOR3 m_vFrontFeelerPos;      // front feeler position
};
