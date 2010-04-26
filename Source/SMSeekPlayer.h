/*******************************************************************************
* Game Development Project
*
* Eric Schwabe
* 2010-04-25
*
* Seek player state machine
*
*******************************************************************************/

#pragma once
#include "statemch.h"

class SMSeekPlayer : public StateMachine
{
    public:
        SMSeekPlayer( GameObject* object, objectID pid );
        virtual ~SMSeekPlayer();

    private:

        virtual bool States( State_Machine_Event event, MSG_Object* msg, int state, int substate );

        objectID m_idPlayer;    // player object id
};
