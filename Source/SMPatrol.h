/*******************************************************************************
* Game Development Project
* RandomMovement.h
*
* Eric Schwabe
* 2010-04-20
*
* Random movement state machine
*
*******************************************************************************/

#pragma once

#include "statemch.h"

class SMPatrol : public StateMachine
{
    public:

	    SMPatrol( GameObject* object, const D3DXVECTOR2& vPatrolPos );
        ~SMPatrol();

    private:

	    virtual bool States( State_Machine_Event event, MSG_Object* msg, int state, int substate );

        // data
        D3DXVECTOR2 m_vPatrolPos;
};
