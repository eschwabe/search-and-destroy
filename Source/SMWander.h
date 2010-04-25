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

};
