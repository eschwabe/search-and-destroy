/*******************************************************************************
* Game Development Project
*
* Eric Schwabe
* 2010-04-25
*
* Game state machine
*
*******************************************************************************/

#pragma once
#include "statemch.h"

class SMGame : public StateMachine
{
    public:
        SMGame(GameObject* object);
        virtual ~SMGame();

    private:

        virtual bool States( State_Machine_Event event, MSG_Object* msg, int state, int substate );

};
