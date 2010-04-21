/* Copyright Steve Rabin, 2008. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright Steve Rabin, 2008"
 */

#include "DXUT.h"
#include "gameobject.h"
#include "msgroute.h"
#include "statemch.h"

/**
* Constructor
*/
GameObject::GameObject( objectID id, unsigned int type, char* name ) : 
    m_markedForDeletion(false),
    m_vPos(0.0f, 0.0f, 0.0f),
    m_vVelocity(0.0f, 0.0f, 0.0f),
    m_vAccel(0.0f, 0.0f, 0.0f),
    m_fYawRotation(0.0f),
    m_fPitchRotation(0.0f),
    m_fRollRotation(0.0f),
    m_fHeight(0.0f),
    m_dHealth(0),
    m_stateMachineManager(NULL)
{
	m_id = id;
	m_type = type;
	
	if( strlen(name) < GAME_OBJECT_MAX_NAME_SIZE ) {
		strcpy( m_name, name );
	}
	else {
		strcpy( m_name, "invalid_name" );
		ASSERTMSG(0, "GameObject::GameObject - name is too long" );
	}
}

/**
* Deconstructor
*/
GameObject::~GameObject( void )
{
    SAFE_RELEASE(m_pStateBlock);
	SAFE_DELETE(m_stateMachineManager);
}

/**
* Get State Machine
*/
StateMachineManager* GameObject::GetStateMachineManager( void )
{ 
    ASSERTMSG(m_stateMachineManager, "GameObject::GetStateMachineManager - m_stateMachineManager not set"); 
    return( m_stateMachineManager ); 
}

/**
* Initialize Object
*/
HRESULT GameObject::InitializeObject(IDirect3DDevice9* pd3dDevice)
{
    assert(pd3dDevice);
    
    // create state machine manager
	m_stateMachineManager = new StateMachineManager( *this );

    // setup state block
    HRESULT result = pd3dDevice->CreateStateBlock(D3DSBT_ALL, &m_pStateBlock);

    if( SUCCEEDED(result) )
    {
        // implemented by derived object
        result = Initialize(pd3dDevice);
    }

    return result;
}

/**
* Update Object
*/
void GameObject::UpdateObject()
{
	if(m_stateMachineManager)
	{
		m_stateMachineManager->Update();
	}

    // implemented by derived object
    Update();
}

/**
* Render Object
*/
void GameObject::RenderObject(IDirect3DDevice9* pd3dDevice, const RenderData* rData)
{
    assert(pd3dDevice);
    assert(rData);

    // capture state
    m_pStateBlock->Capture();

    // implemented by derived object
    Render(pd3dDevice, rData);

    // restore state
    m_pStateBlock->Apply();
}

/**
* Get the current position of the object. 
* Position is the center of the object.
*/
D3DXVECTOR3 GameObject::GetPosition() const
{
    D3DXVECTOR3 pos = m_vPos;
    pos.y += m_fHeight/2.0f;
    return pos;
}
