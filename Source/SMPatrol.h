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

#include "WorldPath.h"
#include "statemch.h"

class SMPatrol : public StateMachine
{
    public:

	    SMPatrol( GameObject* object, const D3DXVECTOR2& vPatrolPos, objectID pid );
        ~SMPatrol();

    private:

	    virtual bool States( State_Machine_Event event, MSG_Object* msg, int state, int substate );

        // data
        D3DXVECTOR2 m_vPatrolPos;       // patrol position
        objectID m_idPlayer;            // player object id

        PathWaypointList waypointList;  // movement waypoint list
};
