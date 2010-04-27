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
#include <sstream>

int i = 5;

/**
* Constructor
*/
GameObject::GameObject( objectID id, unsigned int type, char* name ) : 
    m_markedForDeletion(false),
    m_vPos(0.0f, 0.0f, 0.0f),
    m_vDefaultDirection(0.0f, 0.0f, 1.0f),
    m_vDirection(0.0f, 0.0f, 1.0f),
    m_fVelocity(0.0f),
    m_fAccel(0.0f),
    m_fYawRotation(0.0f),
    m_fPitchRotation(0.0f),
    m_fRollRotation(0.0f),
    m_fHeight(0.0f),
    m_dHealth(0),
    m_bStopMovement(false),
    m_stateMachineManager(NULL)
{
	m_id = id;
	m_type = type;

    std::string sObjectName;
    std::stringstream sStreamOut;
    sStreamOut << name << "[" << id << "]";
    sObjectName = sStreamOut.str();

    if( sObjectName.length() < GAME_OBJECT_MAX_NAME_SIZE ) {
        strcpy( m_name, sObjectName.c_str() );
	}
	else {
		strcpy( m_name, "invalid_name" );
		ASSERTMSG(0, "GameObject::GameObject - name is too long" );
	}

    // create state machine manager
	m_stateMachineManager = new StateMachineManager( *this );
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
    // update state machines
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
* Sets the object parameters to hold at the current position.
*/
void GameObject::ResetMovement()
{
    m_fVelocity = 0.0f;
    m_fAccel = 0.0f;
}

/**
* Gets the grid position for the object. Ignores y direction.
*/
D3DXVECTOR2 GameObject::GetGridPosition() const     
{ 
    D3DXVECTOR2 pos; 
    pos.x = m_vPos.x; 
    pos.y = m_vPos.z; 
    return pos; 
}

/**
* Gets the grid direction for the object. Ignores y direction.
*/
D3DXVECTOR2 GameObject::GetGridDirection() const    
{ 
    D3DXVECTOR2 pos; 
    pos.x = m_vDirection.x; 
    pos.y = m_vDirection.z; 
    return pos; 
}

/**
* Sets the object position. Ignores y direction.
*/
void GameObject::SetGridPosition(const D3DXVECTOR2& pos)    
{ 
    m_vPos.x = pos.x;
    m_vPos.z = pos.y;
};

/**
* Sets the object direction. Ignores y direction.
*/
void GameObject::SetGridDirection(const D3DXVECTOR2& dir)   
{ 
    m_vDirection.x = dir.x;
    m_vDirection.z = dir.y;
};

/**
* Update object position, velocity and acceleration
*/
void GameObject::UpdateObjectPosition()
{
    // check if movement allowed
    if( !m_bStopMovement )
    {
        // update velocity and determine position delta
        m_fVelocity += m_fAccel * g_time.GetElapsedTime();
        D3DXVECTOR3 vPosDelta = m_vDirection * m_fVelocity * g_time.GetElapsedTime();
        
        // update player position
        m_vPos += vPosDelta;
    }
}