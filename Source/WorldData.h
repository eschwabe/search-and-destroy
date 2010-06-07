/*******************************************************************************
* Game Development Project
* WorldData.h
*
* Eric Schwabe
* 2010-05-16
*
* World Data
*
*******************************************************************************/

#pragma once
#include <list>
#include <map>
#include "gameobject.h"
#include "WorldFile.h"


/* path waypoint list */
typedef std::list<D3DXVECTOR2> PathWaypointList;
        
const float kWorldScale = 1.0f;;

/* debug drawing color */
enum DebugDrawingColor
{
	DEBUG_COLOR_NULL,
	DEBUG_COLOR_WHITE,
	DEBUG_COLOR_BLACK,
	DEBUG_COLOR_RED,
	DEBUG_COLOR_GREEN,
	DEBUG_COLOR_BLUE,
	DEBUG_COLOR_YELLOW,
	DEBUG_COLOR_PURPLE,
	DEBUG_COLOR_CYAN,
	DEBUG_COLOR_NUM
};

/* world path computations */
class WorldData : public GameObject, public Singleton<WorldData>
{
    public:

        /**
        * Terrain analysis type
        */
        enum TerrainAnalysisType
        {
            kTerrainAnalysisNone,
            kTerrainAnalysisOpenness,
            kTerrainAnalysisOccupancy,
            kTerrainAnalysisLineOfFire,
            kTerrainAnalysisAll
        };

        // constructor
        WorldData(const WorldFile& worldFile);
        virtual ~WorldData();

        // path requests
        void AddPathRequest(const D3DXVECTOR2& vCurPos, const D3DXVECTOR2& vDestPos, objectID id);
        D3DXVECTOR2 GetRandomMapLocation();

        // waypoint lsits
        PathWaypointList* GetWaypointList(objectID id);
        void ClearWaypointList(objectID id);

        // debugging
        void SetTerrainAnalysisType(TerrainAnalysisType type) { m_terrainType = type; }
        void ToggleTerrainAnalysisType();
        TerrainAnalysisType GetTerrainAnalysisType() const { return m_terrainType; }
        std::wstring GetTerrainAnalysisName() const;

        void ShowPathDebug() { m_debuglines = true; }
        void HidePathDebug() { m_debuglines = false; }
        void TogglePathDebug() { m_debuglines = !m_debuglines; }

    protected:

        // game object methods
        virtual HRESULT Initialize(IDirect3DDevice9* pd3dDevice);
        virtual void Update();
        virtual void Render(IDirect3DDevice9* pd3dDevice, const RenderData* rData);

    private:

        /////////////////
        // DEBUG LINES //
        /////////////////

        D3DXVECTOR3 CreateLinePosition(const D3DXVECTOR2& pos);
        D3DXCOLOR GetColor( DebugDrawingColor color );
        void AddLine( D3DXVECTOR3 vP1, D3DXVECTOR3 vP2, DebugDrawingColor dColor, bool bArrowHead );

        // custom FVF, which describes the custom vertex structure
        static const DWORD D3DFVF_CUSTOMVERTEX = (D3DFVF_XYZ|D3DFVF_DIFFUSE);

        /**
        * Custom vertex type. Specifies a custom vertex that can be written
        * to the verticies buffer for rendering.
        */
	    struct CustomVertex
	    {
            D3DXVECTOR3 vPos;   // untransformed, 3D position for the vertex
            D3DCOLOR cColor;    // vertex color
        };

        std::vector<CustomVertex> m_vLines;     // debug line vertices

        //////////////////////
        // TERRAIN ANALYSIS //
        //////////////////////

        float** m_fTerrainOpenness;
        float** m_fTerrainOccupancy;
        float** m_fTerrainLineOfFire;

        TerrainAnalysisType m_terrainType;      // terrain analysis type
        std::vector<CustomVertex> m_vQuads;     // debug line vertices

        float** AllocateTerrainGrid();
        void DeleteTerrainGrid(float** fGrid);
        void GenerateTerrainQuads();
        void AnalyzeTerrainOccupancy();
        void AnalyzeTerrainLineOfFire();
        void UpdateTerrainGridCells(float** grid, const int row, const int col, const int range, const float value);
        D3DXCOLOR GetTerrainColor(const int row, const int col);

        /////////////////////
        // PATHING OPTIONS //
        /////////////////////

        // world info
        const WorldFile& m_worldFile;

        // path options
        bool m_debuglines;          // show path debug lines
        bool m_rubberband;          // enable path rubberbanding
        bool m_heuristicCalc;       // true for cardinal/intercardinal; false for eucladian
        bool m_smooth;              // enable catmull-rom path smoothing
        float m_heuristicWeight;    // heuristic weight (1.01f is preferred)

        //////////////////
        // A* node data //
        //////////////////
        struct NodeKey;
        struct NodeData;

        // node list
        typedef std::list<NodeData> NodeList;

        // node key
        struct NodeKey
        {
            int iRow;
            int iCol;
        };

        // node data
        struct NodeData
        {
            NodeKey loc;

            NodeList::iterator nParent;

            float fDistanceCost;
            float fHeuristicCost;
            float fTotalCost;

            bool bClosed;
        };

        bool m_bPathInProgress;
        NodeList m_mNodeList;

        ///////////////////////
        // path request list //
        ///////////////////////
        struct PathRequest
        {
            objectID id;
            NodeKey nkPos;
            NodeKey nkDestPos;
            PathWaypointList waypointList;
        };

        std::list<PathRequest> m_requestList;

        std::map<objectID, PathWaypointList> m_completeWaypointLists;

        ////////////////
        // A* methods //
        ////////////////
        void ComputePaths();
        bool RunComputationLoop(const PathRequest& req);
        void AddWaypoints(PathWaypointList* waypointList, NodeList::iterator position);
        void SmoothWaypoints(PathWaypointList* waypointList);
        bool CheckNodeRubberband(NodeList::iterator position, NodeList::iterator prev, NodeList::iterator next);
        void UpdateRubberbandBounds(NodeList::iterator node, int& iMinRow, int& iMaxRow, int& iMinCol, int& iMaxCol);
        void AddNeighborNodes(NodeList::iterator position, const NodeKey& nkDest);
        void CreateNode(const NodeKey& nkPos, const NodeKey& nkDest, NodeList::iterator parent);
        float ComputeDistanceCost(const NodeKey& nkPos, NodeList::iterator parent);
        float ComputeHeuristicCost(const NodeKey& nkPos, const NodeKey& nkDest);
        NodeList::iterator FindLowestCostNode();
        void GetRowColumn( const D3DXVECTOR2& pos, NodeKey* key );
        D3DXVECTOR2 GetCoordinates( const NodeKey& key );
};
