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
#include "global.h"
#include "database.h"
#include "time.h"
#include "RenderData.h"

// add new object types here (bitfield mask - objects can be combinations of types)
#define OBJECT_Ignore_Type  (0)
#define OBJECT_World        (1<<1)
#define OBJECT_Character    (1<<2)
#define OBJECT_NPC          (1<<3)
#define OBJECT_Player       (1<<4)
#define OBJECT_Enemy        (1<<5)
#define OBJECT_Weapon       (1<<6)
#define OBJECT_Item         (1<<7)
#define OBJECT_Projectile   (1<<8)
#define OBJECT_Map          (1<<9)

#define GAME_OBJECT_MAX_NAME_SIZE 64


// forward declarations
class StateMachineManager;
class MSG_Object;


class GameObject
{
    public:

	    GameObject( objectID id, unsigned int type, char* name );
	    virtual ~GameObject( void );

        // game object info
	    inline objectID GetID( void )			{ return( m_id ); }
	    inline unsigned int GetType( void )		{ return( m_type ); }
	    inline char* GetName( void )			{ return( m_name ); }
    	
	    // state machine
	    StateMachineManager* GetStateMachineManager( void );

	    // scheduled deletion
	    inline void MarkForDeletion( void )				{ m_markedForDeletion = true; }
	    inline bool IsMarkedForDeletion( void )			{ return( m_markedForDeletion ); }

        // object management
        HRESULT InitializeObject(IDirect3DDevice9* pd3dDevice);
        void UpdateObject();
        void RenderObject(IDirect3DDevice9* pd3dDevice, const RenderData* rData);
        
        // object info
        DWORD GetHealth() const                 { return m_dHealth;         };
        void SetHealth(DWORD health)            { m_dHealth = health;       };
        float GetHeight() const                 { return m_fHeight;         };

        // object position info
        D3DXVECTOR3 GetPosition() const;
        D3DXVECTOR3 GetVelocity() const         { return m_vVelocity;       };
        D3DXVECTOR3 GetAcceleration() const     { return m_vAccel;          };
        float GetYawRotation() const            { return m_fYawRotation;    };
        float GetPitchRotation() const          { return m_fPitchRotation;  };
        float GetRollRotation() const           { return m_fRollRotation;   };
        
        // collision events
        virtual void EnvironmentCollisionEvent(const D3DXVECTOR3& vPosDelta) { m_vPos += vPosDelta; };

    protected:

        // methods implemented by derived objects
        virtual HRESULT Initialize(IDirect3DDevice9* pd3dDevice) = 0;
        virtual void Update() = 0;
        virtual void Render(IDirect3DDevice9* pd3dDevice, const RenderData* rData) = 0;

        // object info
        float m_fHeight;            // object height
        DWORD m_dHealth;            // object health

        // object position info
        D3DXVECTOR3 m_vPos;         // position
        D3DXVECTOR3 m_vVelocity;    // velocity
        D3DXVECTOR3 m_vAccel;       // acceleration
        
        float m_fYawRotation;       // rotation (y-axis)
        float m_fPitchRotation;     // rotation (z-axis)
        float m_fRollRotation;      // rotation (x-axis)

    private:

	    objectID m_id;								// unique id of object (safer than a pointer)
	    unsigned int m_type;						// type of object (can be combination)
	    bool m_markedForDeletion;					// flag to delete this object (when it is safe to do so)
	    char m_name[GAME_OBJECT_MAX_NAME_SIZE];		// string name of object
  
        IDirect3DStateBlock9* m_pStateBlock;        // state block

	    StateMachineManager* m_stateMachineManager; // state machine manager
};
