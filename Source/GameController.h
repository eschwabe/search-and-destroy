/*******************************************************************************
* Game Development Project
* GameController.h
*
* Eric Schwabe
* 2010-05-05
*
* Game Controller
*
*******************************************************************************/

#pragma once
#include "gameobject.h"
#include "DebugCamera.h"
#include "PlayerCamera.h"
#include "RotationCamera.h"
#include "DXUT\SDKsound.h"

class GameController : public GameObject
{
    public:

        enum CameraType
        {
            CAMERA_DEBUG,
            CAMERA_PLAYER,
            CAMERA_WORLDVIEW
        };

        // constructors
        GameController(objectID pid);
        virtual ~GameController();

        // game control
        void SetTitleText(const std::wstring str)   { m_titleText = str;        }
        void ShowTitleScreen()                      { m_titleScreen = true;     }
        void HideTitleScreen()                      { m_titleScreen = false;    }
        void SetCameraType(const CameraType type);

        void ShowDebugInfo()                        { m_debugInfo = true;       }
        void HideDebugInfo()                        { m_debugInfo = false;      }
        bool DebugEnabled()                         { return m_debugInfo;       }

        // camera methods
        void ConfigureCameras(const D3DSURFACE_DESC* pBackBufferSurfaceDesc);
        const D3DXMATRIX* GetCameraViewMatrix();
        const D3DXMATRIX* GetCameraProjMatrix();

        // sound methods
        void PlayLaserSound();

        // handle user input
        LRESULT HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    protected:

        // init, update, render
        HRESULT Initialize(IDirect3DDevice9* pd3dDevice);
        void Update();
        void Render(IDirect3DDevice9* pd3dDevice, const RenderData* rData);
       
    private:

        objectID        m_pid;              // player object id
        GameObject*     m_PlayerObject;     // player object

        //////////////////
        // TITLE SCREEN //
        //////////////////

        // custom FVF, which describes the custom vertex structure
        static const DWORD D3DFVF_CUSTOMVERTEX = (D3DFVF_XYZRHW | D3DFVF_DIFFUSE );

        /**
        * Custom vertex type. Specifies a custom vertex that can be written
        * to the verticies buffer for rendering.
        */
	    struct CustomVertex
	    {
            D3DXVECTOR3 vPos;   // transformed, 2D position for the vertex (z unused?)
            float rhw;          // set to 1 (prevent pipeline from transforming vertices)
            D3DCOLOR vColor;    // vertex color

            // default constructor
            CustomVertex() :
                vPos(0.0f, 0.0f, 0.0f), rhw(1.0f), vColor( D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.0f) )
            {}

            // initialization constructor
            CustomVertex(D3DXVECTOR3 ivPos, D3DXCOLOR iColor) :
                vPos(ivPos), rhw(1.0), vColor(iColor)
            {}
	    };

        // methods
        void RenderGameTitleText();
        void RenderGameHelpText();

        // data
        CustomVertex    m_cvScreenVertices[4];  // title screen vertices
        std::wstring    m_titleText;            // title text
        bool            m_titleScreen;          // show title screen
        bool            m_debugInfo;            // show debug info

        ////////////
        // SOUNDS //
        ////////////
        CSoundManager*  m_pSoundManager;    // sound manager
        CSound*         m_pSoundLaser;      // laser sound
        bool            m_bEnableSound;     // sound toggle

        /////////////
        // CAMERAS //
        /////////////

        // methods
        CBaseCamera* GetCurrentCamera();

        // data
        CDebugCamera    m_DebugCamera;      // debug camera
        CPlayerCamera   m_PlayerCamera;     // player camera
        CRotationCamera m_WorldViewCamera;  // world view camera
        CameraType      m_CurrentCamera;    // current camera

        ///////////
        // DEBUG //
        ///////////
        void RenderDebugText();

        ID3DXFont*      m_pDebugFont;       // font for drawing text
        ID3DXFont*      m_pGameFontLarge;   // font for drawing text
        ID3DXFont*      m_pGameFontSmall;   // font for drawing text
        ID3DXSprite*    m_pTextSprite;      // sprite for batching draw text calls

};
