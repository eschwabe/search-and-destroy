/* Copyright Steve Rabin, 2008. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright Steve Rabin, 2008"
 */

#pragma once

#include <list>
#include <vector>
#include "global.h"
#include "msg.h"
#include "singleton.h"

#define INVALID_OBJECT_ID 0

class GameObject;
class RenderData;

// game object list
typedef std::vector<GameObject*> dbCompositionList;


class Database : public Singleton <Database>
{
    public:

        // constructor
	    Database( void );
	    ~Database( void );

        // object management
        HRESULT InitializeObjects(IDirect3DDevice9* pd3dDevice);
        void UpdateObjects();
        void RenderObjects(IDirect3DDevice9* pd3dDevice, const RenderData* rData);

	    void Store( GameObject* object );
	    void Remove( objectID id );

        // find objects
	    GameObject* Find( objectID id );
	    GameObject* FindByName( char* name );
	    void ComposeList( dbCompositionList & list, unsigned int type = 0 );

        // objects ids
	    objectID GetIDByName( char* name );
	    objectID GetNewObjectID( void );
    	
        // send messages
	    void SendMsgFromSystem( objectID id, MSG_Name name, MSG_Data& data = MSG_Data() );
	    void SendMsgFromSystem( GameObject* object, MSG_Name name, MSG_Data& data = MSG_Data() );
	    void SendMsgFromSystem( MSG_Name name, MSG_Data& data = MSG_Data() );

    private:

	    typedef std::list<GameObject*> dbContainer;

	    // make this a more efficient data structure (like a hash table)
	    dbContainer m_database;

	    objectID m_nextFreeID;
};
