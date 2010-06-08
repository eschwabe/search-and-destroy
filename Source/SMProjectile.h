/*******************************************************************************
* Game Development Project
*
* Eric Schwabe
* 2010-04-25
*
* Projectile state machine
*
*******************************************************************************/

#pragma once
#include "statemch.h"

class SMProjectile : public StateMachine
{
    public:

        SMProjectile( GameObject* object, const float fVel, const float fAccel, const D3DXVECTOR3 vDir );
        virtual ~SMProjectile();

    private:

        virtual bool States( State_Machine_Event event, MSG_Object* msg, int state, int substate );

        float m_fVel;
        float m_fAccel;
        float m_fDist;
        D3DXVECTOR3 m_vInitialPos;
        D3DXVECTOR3 m_vDir;

        objectID m_pID;
};
