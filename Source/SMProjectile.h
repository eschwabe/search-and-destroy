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

        SMProjectile(GameObject* object);
        virtual ~SMProjectile();

    private:

        virtual bool States( State_Machine_Event event, MSG_Object* msg, int state, int substate );

};
