/*******************************************************************************
* Game Development Project
* SMRandomPath.h
*
* Eric Schwabe
* 2010-04-18
*
* Random path state machine
*
*******************************************************************************/

#pragma once
#include "statemch.h"

class SMRandomPath : public StateMachine
{
    public:

        SMRandomPath(GameObject* object, objectID pid);
        virtual ~SMRandomPath();

    private:

        virtual bool States( State_Machine_Event event, MSG_Object* msg, int state, int substate );

        objectID m_idPlayer;                // player object id
};