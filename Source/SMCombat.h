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

class SMCombat : public StateMachine
{
    public:
        SMCombat( GameObject* object, objectID pid, bool damaged );
        virtual ~SMCombat();

    private:

        virtual bool States( State_Machine_Event event, MSG_Object* msg, int state, int substate );

        objectID m_idPlayer;    // player object id
        bool m_bDamaged;        // player damaged (initialize only)
};
