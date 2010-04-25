/*******************************************************************************
* Game Development Project
*
* Eric Schwabe
* 2010-04-25
*
* Player state machine
*
*******************************************************************************/

#pragma once
#include "statemch.h"

class SMPlayer : public StateMachine
{
    public:

        SMPlayer(GameObject* object);
        virtual ~SMPlayer();

    private:

        virtual bool States( State_Machine_Event event, MSG_Object* msg, int state, int substate );

};
