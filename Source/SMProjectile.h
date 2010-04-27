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

        SMProjectile( GameObject* object, const float fVel, const float fAccel, const D3DXVECTOR3 vDir, const bool bSeek );
        virtual ~SMProjectile();

    private:

        virtual bool States( State_Machine_Event event, MSG_Object* msg, int state, int substate );

        float m_fVel;
        float m_fAccel;
        D3DXVECTOR3 m_vDir;
        bool m_bSeek;

        objectID m_pID;
};
