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
#include "GameController.h"

class SMGame : public StateMachine
{
    public:

        SMGame( GameController* controller, objectID mapID );
        virtual ~SMGame();

    private:

        virtual bool States( State_Machine_Event event, MSG_Object* msg, int state, int substate );

        GameController* m_controller;       // game controller object
        objectID m_mapID;                   // minimap object id
};
