/*******************************************************************************
* Game Development Project
* WorldData.cpp
*
* Eric Schwabe
* 2010-05-16
*
* World Data
*
*******************************************************************************/

#include "DXUT.h"
#include "WorldData.h"
#include "database.h"

/**
* Constructor
*/
WorldData::WorldData(const WorldFile& worldFile) :
    GameObject(g_database.GetNewObjectID(), OBJECT_Debug, "PATH_DEBUG"),
    m_bPathInProgress(false),
    m_worldFile(worldFile),
    m_rubberband(true),
    m_heuristicCalc(true),
    m_smooth(true),
    m_heuristicWeight(1.01f),
    m_debuglines(true),
    m_terrainType(kTerrainAnalysisNone)
{
    // allocate terrain analysis grids
    m_fTerrainOpenness = AllocateTerrainGrid();
    m_fTerrainOccupancy = AllocateTerrainGrid();
    m_fTerrainLineOfFire = AllocateTerrainGrid();

    // initialize quad memory
    m_vQuads.reserve(m_worldFile.GetHeight()*m_worldFile.GetWidth());
}

/**
* Deconstructor
*/
WorldData::~WorldData()
{
    // delete terrain grids
    DeleteTerrainGrid(m_fTerrainOpenness);
    DeleteTerrainGrid(m_fTerrainOccupancy);
    DeleteTerrainGrid(m_fTerrainLineOfFire);
}

/**
* Allocates a new 2D grid for terrain analysis.
*/
float** WorldData::AllocateTerrainGrid()
{
    float** fGrid = NULL;

    // allocate terrain analysis grid rows
    fGrid = new float*[m_worldFile.GetHeight()];

    // allocate each column
    for(int row = 0; row < m_worldFile.GetHeight(); ++row)
        fGrid[row] = new float[m_worldFile.GetWidth()];

    return fGrid;
}

/**
* Deletes a 2D terrain analysis grid.
*/
void WorldData::DeleteTerrainGrid(float** fGrid)
{
    // delete each column
    for(int row = 0; row < m_worldFile.GetHeight(); ++row)
        delete [] fGrid[row];

    // delete grid rows
    delete [] fGrid;
}

/**
* Toggle to next terrain analysis type
*/
void WorldData::ToggleTerrainAnalysisType()
{
    switch(m_terrainType)
    {
    case kTerrainAnalysisNone:
        m_terrainType = kTerrainAnalysisOccupancy;
        break;
    case kTerrainAnalysisOccupancy:
        m_terrainType = kTerrainAnalysisOpenness;
        break;
    case kTerrainAnalysisOpenness:
        m_terrainType = kTerrainAnalysisLineOfFire;
        break;
    case kTerrainAnalysisLineOfFire:
        m_terrainType = kTerrainAnalysisAll;
        break;
    case kTerrainAnalysisAll:
        m_terrainType = kTerrainAnalysisNone;
        break;
    }
}

/**
* Generates names for the various types of terrain analysis
*/
std::wstring WorldData::GetTerrainAnalysisName() const
{
    switch(m_terrainType)
    {
    case kTerrainAnalysisOccupancy:
        return std::wstring(L"Occupancy");
    case kTerrainAnalysisOpenness:
        return std::wstring(L"Openness");
    case kTerrainAnalysisLineOfFire:
        return std::wstring(L"LineOfFire");
    case kTerrainAnalysisAll:
        return std::wstring(L"All");
    case kTerrainAnalysisNone:
    default:
        return std::wstring(L"None");
    }
}

/**
* Initialize object
*/
HRESULT WorldData::Initialize(IDirect3DDevice9* pd3dDevice)
{
    // initialize terrain openness
    for(int col = 0; col < m_worldFile.GetWidth(); ++col)
    {
        for(int row = 0; row < m_worldFile.GetHeight(); ++row)
        {
            m_fTerrainOpenness[row][col] = 0.0f;
        }
    }

    // analyze terrain openness
    for(int col = 0; col < m_worldFile.GetWidth(); ++col)
    {
        for(int row = 0; row < m_worldFile.GetHeight(); ++row)
        {
            // occupied cell
            if( m_worldFile(row,col) == WorldFile::OCCUPIED_CELL )
            {
                m_fTerrainOpenness[row][col] = 1.0f;
            }

            // next to occupied cell
            else if( m_worldFile(row-1,col) == WorldFile::OCCUPIED_CELL ||
                     m_worldFile(row+1,col) == WorldFile::OCCUPIED_CELL ||
                     m_worldFile(row,col-1) == WorldFile::OCCUPIED_CELL ||
                     m_worldFile(row,col+1) == WorldFile::OCCUPIED_CELL )
            {
                m_fTerrainOpenness[row][col] = 0.67f;
            }

            // corner to occupied cell
            else if( m_worldFile(row-1,col-1) == WorldFile::OCCUPIED_CELL ||
                     m_worldFile(row+1,col+1) == WorldFile::OCCUPIED_CELL ||
                     m_worldFile(row+1,col-1) == WorldFile::OCCUPIED_CELL ||
                     m_worldFile(row-1,col+1) == WorldFile::OCCUPIED_CELL )
            {
                m_fTerrainOpenness[row][col] = 0.33f;
            }

            // no occupied cells nearby
            else
            {
                m_fTerrainOpenness[row][col] = 0.15f;
            }
        }
    }

    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);
}

/**
* Update object
*/
void WorldData::Update()
{
    // compute paths
    ComputePaths();

    // reset lines
    m_vLines.clear();

    // update path debug lines
    if(m_debuglines)
    {
        for(std::map<objectID, PathWaypointList>::iterator list = m_completeWaypointLists.begin(); list != m_completeWaypointLists.end(); ++list)
        {
            // get waypoint list and object
            PathWaypointList& waypointList = (*list).second;
            GameObject* obj = g_database.Find( (*list).first );
              
            // if list not empty
            if( !waypointList.empty() )
	        {
                // initialize previous point to object position
		        D3DXVECTOR3 vPrevPoint = obj->GetPosition();
    		    
                // add lines for remaining points
		        for( PathWaypointList::iterator point = waypointList.begin(); point != waypointList.end(); ++point )
		        {
                    D3DXVECTOR3 vPoint = CreateLinePosition( *point );
                    D3DXVECTOR3 vPointMarker = vPoint;
                    vPointMarker.y -= 0.25f;

                    // add lines for path and marker
			        AddLine( vPointMarker, vPoint, DEBUG_COLOR_CYAN, false );
			        AddLine( vPrevPoint, vPoint, DEBUG_COLOR_GREEN, true );
    			    
                    // update previous point
                    vPrevPoint = vPoint;
		        }
	        }
        }
    }
    
    // analyze world occupancy
    AnalyzeTerrainOccupancy();

    // analyze player line of fire
    AnalyzeTerrainLineOfFire();

    // create terrain analysis debug quads
    GenerateTerrainQuads();
}

/**
* Update terrain occupancy based on objects in the world
*/
void WorldData::AnalyzeTerrainOccupancy()
{
    // reset terrain occupancy
    for(int col = 0; col < m_worldFile.GetWidth(); ++col)
    {
        for(int row = 0; row < m_worldFile.GetHeight(); ++row)
        {
            m_fTerrainOccupancy[row][col] = 0.0f;
        }
    }

    // generate list of players/NPCs
    dbCompositionList playerList;
    g_database.ComposeList(playerList, OBJECT_NPC | OBJECT_Player);

    // update occupancy based on player/NPC positions
    for(dbCompositionList::iterator it = playerList.begin(); it != playerList.end(); ++it)
    {
        int row = (int)(*it)->GetGridPosition().y;
        int col = (int)(*it)->GetGridPosition().x;

        // occupied at location of player
        m_fTerrainOccupancy[row][col] = min(1.0f, m_fTerrainOccupancy[row][col]+1.0f);

        // partially occupied near player (one cell from player)
        UpdateTerrainGridCells(m_fTerrainOccupancy, row, col, 1, 0.67f);

        // partially occupied near player (two cells from player)
        UpdateTerrainGridCells(m_fTerrainOccupancy, row, col, 2, 0.33f);
    }
}

/**
* Updates terrain  a distance of range from the specified row and column
*/
void WorldData::UpdateTerrainGridCells(float** grid, const int row, const int col, const int range, const float value)
{
    // partially occupied near player (one cell from player)
    for(int i = col-range; i <= col+range; ++i)
    {
        for(int j = row-range; j <= row+range; ++j)
        {
            // check if along border of selected grid
            if(i == col-range || i == col+range || j == row-range || j == row+range)
            {
                // check if valid cell
                if( (j >= 0 && j < m_worldFile.GetHeight()) && (i >= 0 && i < m_worldFile.GetWidth()) )
                {
                    grid[j][i] = min(1.0f, grid[j][i]+value);
                }
            }
        }
    }
}

/**
* Update terrain line of fire based on player objects and direction
*/
void WorldData::AnalyzeTerrainLineOfFire()
{
    // reset terrain occupancy
    for(int col = 0; col < m_worldFile.GetWidth(); ++col)
    {
        for(int row = 0; row < m_worldFile.GetHeight(); ++row)
        {
            m_fTerrainLineOfFire[row][col] = 0.0f;
        }
    }

    // generate list of players/NPCs
    dbCompositionList playerList;
    g_database.ComposeList(playerList, OBJECT_Player);

    // update occupancy based on player/NPC positions
    for(dbCompositionList::iterator it = playerList.begin(); it != playerList.end(); ++it)
    {
        // get player position and direction
        D3DXVECTOR3 vDir = (*it)->GetDirection();
        D3DXVECTOR3 vPos = (*it)->GetPosition();
        D3DXVec3Normalize(&vDir, &vDir);

        // compute initial grid position
        int row = (int)vPos.z;
        int col = (int)vPos.x;

        // until empty cell found
        while( m_worldFile(row, col) == WorldFile::EMPTY_CELL )
        {
            // set cell in direct line of fire
            m_fTerrainLineOfFire[row][col] = 1.0f;

            // update cells around direct line of fire
            UpdateTerrainGridCells(m_fTerrainLineOfFire, row, col, 1, 0.2f);

            // move to next grid position
            vPos += vDir;
            row = (int)vPos.z;
            col = (int)vPos.x;
        }
    }
}

/**
* Determine the color for the specified grid
*/
D3DXCOLOR WorldData::GetTerrainColor(const int row, const int col)
{
    // alpha
    float fGridAlpha = 0.5f;

    // default color
    D3DXCOLOR color = D3DXCOLOR(0.0f, 0.0f, 0.0f, fGridAlpha);

    switch(m_terrainType)
    {
    case kTerrainAnalysisNone:
        break;
    case kTerrainAnalysisOccupancy:
        color.r = m_fTerrainOccupancy[row][col];
        break;
    case kTerrainAnalysisOpenness:
        color.g = m_fTerrainOpenness[row][col];
        break;
    case kTerrainAnalysisLineOfFire:
        color.b = m_fTerrainLineOfFire[row][col];
        break;
    case kTerrainAnalysisAll:
        color.r = m_fTerrainOccupancy[row][col];
        color.g = m_fTerrainOpenness[row][col];
        color.b = m_fTerrainLineOfFire[row][col];
        break;
    }

    return color;
}

/**
* Create the quads for displaying terrain analysis debug
*/
void WorldData::GenerateTerrainQuads()
{
    // update terrain analysis grid
    float fGridHeight = 0.05f;

    // clear quads
    m_vQuads.clear();

    for(int col = 0; col < m_worldFile.GetWidth(); ++col)
    {
        for(int row = 0; row < m_worldFile.GetHeight(); ++row)
        {
            // determine color
            D3DXCOLOR color = GetTerrainColor(row, col);

            // initialize vertex
            CustomVertex vertex;
            vertex.cColor = color;

            // lower triangle
            vertex.vPos = D3DXVECTOR3((float)col, fGridHeight, (float)row);
            m_vQuads.push_back(vertex);

            vertex.vPos = D3DXVECTOR3((float)col, fGridHeight, row+kWorldScale);
            m_vQuads.push_back(vertex);

            vertex.vPos = D3DXVECTOR3(col+kWorldScale, fGridHeight, row+kWorldScale);
            m_vQuads.push_back(vertex);

            // upper triangle
            vertex.vPos = D3DXVECTOR3((float)col, fGridHeight, (float)row);
            m_vQuads.push_back(vertex);

            vertex.vPos = D3DXVECTOR3(col+kWorldScale, fGridHeight, row+kWorldScale);
            m_vQuads.push_back(vertex);

            vertex.vPos = D3DXVECTOR3(col+kWorldScale, fGridHeight, (float)row);
            m_vQuads.push_back(vertex);
        }
    }
}

/**
* Create line position from grid position
*/
D3DXVECTOR3 WorldData::CreateLinePosition(const D3DXVECTOR2& pos)
{
    D3DXVECTOR3 vLinePos;
    vLinePos.x = pos.x;
    vLinePos.z = pos.y;
    vLinePos.y = 0.5f;

    return vLinePos;
}

/**
* Create debug drawing color
*/
D3DXCOLOR WorldData::GetColor( DebugDrawingColor color )
{
	D3DXCOLOR xColor( 0.0f, 0.0f, 0.0f, 1.0f );

	switch( color )
	{
		case DEBUG_COLOR_RED:		xColor.r = 1.0f; break;
		case DEBUG_COLOR_GREEN:		xColor.g = 1.0f; break;
		case DEBUG_COLOR_BLUE:		xColor.b = 1.0f; break;
		case DEBUG_COLOR_YELLOW:	xColor.r = 1.0f; xColor.g = 1.0f; break;
		case DEBUG_COLOR_PURPLE:	xColor.r = 1.0f; xColor.b = 1.0f; break;
		case DEBUG_COLOR_CYAN:		xColor.g = 1.0f; xColor.b = 1.0f; break;
		case DEBUG_COLOR_WHITE:		xColor.r = 1.0f; xColor.g = 1.0f; xColor.b = 1.0f; break;
		case DEBUG_COLOR_BLACK:		break;
		case DEBUG_COLOR_NULL:		ASSERTMSG( 0, "Color can't be DEBUG_COLOR_NULL" ); break;
	}

	return( xColor );
}

/**
* Add line to vertex buffer. Optional arrow head.
*/
void WorldData::AddLine( D3DXVECTOR3 vP1, D3DXVECTOR3 vP2, DebugDrawingColor dColor, bool bArrowHead )
{
    CustomVertex vertex;

    // get color
    D3DXCOLOR xColor = GetColor( dColor );
    vertex.cColor = xColor;
    
    // add line verticies
    vertex.vPos = vP1;
    m_vLines.push_back(vertex);

    vertex.vPos = vP2;
    m_vLines.push_back(vertex);

    // add arrow head vertices
	if(bArrowHead)
	{
		D3DXVECTOR3 vArrowHead[3];

		D3DXVECTOR3 vBackDir = vP1 - vP2;
		D3DXVec3Normalize(&vBackDir, &vBackDir);

		D3DXVECTOR3 vUpDir = D3DXVECTOR3(0.0f, 1.0f, 0.0);

		D3DXVECTOR3 vCrossDir;
		D3DXVec3Cross(&vCrossDir, &vBackDir, &vUpDir);

        // add right arrow head line
		vertex.vPos = vP2;
        m_vLines.push_back(vertex);
		vertex.vPos = vP2 + (vCrossDir + (2.0f*vBackDir)) * 0.075f;
        m_vLines.push_back(vertex);

        // add left arrow head line
		vertex.vPos = vP2;
        m_vLines.push_back(vertex);
		vertex.vPos = vP2 + (-vCrossDir + (2.0f*vBackDir)) * 0.075f;  
        m_vLines.push_back(vertex);
	}
}

/**
* Render object (debug lines)
*/
void WorldData::Render(IDirect3DDevice9* pd3dDevice, const RenderData* rData)
{
    // set the texture (or unset)
    pd3dDevice->SetTexture(0, NULL);

    // set the world space transform
    pd3dDevice->SetTransform(D3DTS_WORLD, &rData->matWorld);

    // turn off D3D lighting, since we are providing our own vertex colors
    pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

    // set vertex type
    pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );    
    
    // check if any lines
    if(m_vLines.size() && m_debuglines)
    {
        // draw
        pd3dDevice->DrawPrimitiveUP( D3DPT_LINELIST, m_vLines.size()/2, &(m_vLines[0]), sizeof(m_vLines[0]) );
    }

    // check for any terrain analysis quads
    if(m_vQuads.size())
    {
        // enable alpha blending
        pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);

        // draw
        pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, m_vQuads.size()/3, &(m_vQuads[0]), sizeof(m_vQuads[0]) );
    }
}

/**
* Finds a random location in the map that an object can move to (i.e. non-wall location)
*/
D3DXVECTOR2 WorldData::GetRandomMapLocation()
{
    int iRow = rand() % m_worldFile.GetHeight();    // z
    int iCol = rand() % m_worldFile.GetWidth();     // x

    // find an empty cell
    while( m_worldFile(iRow, iCol) != WorldFile::EMPTY_CELL )
    {
        // move to next row
        ++iRow;

        // move to next column if at end of row
        if( iRow >= m_worldFile.GetHeight() )
        {
            iRow = 0;
            ++iCol;

            // reset to first column if at end of map
            if( iCol >= m_worldFile.GetWidth() )
            {
                iCol = 0;
            }
        }
    }

    NodeKey key;
    key.iRow = iRow;
    key.iCol = iCol;

    return GetCoordinates(key);
}


/////////////////////////////
// PATHFINDING COMPUTATION //
/////////////////////////////

/**
* Add path requests for computation. Object notified when request
* calculation complete.
*
* Initiates A* path computation from the current position to the requested position.
* Path waypoints are appended to the waypoint list. If in a path is currently being
* calculated, all current results are reset.
*
* 1. Push Start Node onto the Open List
* 2. Pop best node off Open List (call it B)
*     a. If B is the Goal Node, then path found
*     b. If Open List is empty, then no path possible
*     c. Find all neighboring nodes – for each node (call it C):
*       I. Compute its cost (f, g, h)
*       II. If C is not on the Open List or Closed List with a lower f(x) cost, then take it off both lists and add it to the Open List
*     d. Place B on the Closed List (we’re done with it)
*     e. If taken too much time this frame, abort search for now and resume next frame
*     f. Go to step 2
*/
void WorldData::AddPathRequest(
    const D3DXVECTOR2& vPos, 
    const D3DXVECTOR2& vDestPos,
    objectID id)
{
    PathRequest req;

    // clear any existing waypoints
    ClearWaypointList(id);

    // create destination key
    GetRowColumn(vDestPos, &req.nkDestPos);

    // create current position key
    GetRowColumn(vPos, &req.nkPos);

    // set id
    req.id = id;

    // add request
    m_requestList.push_back(req);
}

/**
* Find waypoint list for specified object id. Returns null if not found.
*/
PathWaypointList* WorldData::GetWaypointList(objectID id)
{
    return &(m_completeWaypointLists[id]);
}

/**
* Clears an object waypoint list.
*/
void WorldData::ClearWaypointList(objectID id)
{
    m_completeWaypointLists.erase(id);
}

/**
* Runs a single pass of the A* computation. Updates state when computation complete.
* Compute paths. Only works on path calculations for a specific interval.
*/
void WorldData::ComputePaths()
{
    // check if a path computation in progress
    if(m_bPathInProgress)
    {
        std::list<PathRequest>::iterator req = m_requestList.begin();
        
        // run computation
        if( RunComputationLoop(*req) )
        {
            // request complete, reset computation
            m_bPathInProgress = false;

            // send completion message
            g_database.SendMsgFromSystem(m_requestList.begin()->id, MSG_PathComputed);

            // store completed waypoints
            m_completeWaypointLists[req->id] = req->waypointList;

            // remove request
            m_requestList.pop_front();
        }
    }

    // start new request if one in queue
    else if(!m_bPathInProgress && !m_requestList.empty() )
    {
        // make search in progress
        m_bPathInProgress = true;    
    
        // clear list
        m_mNodeList.clear();

        // create starting node
        CreateNode(m_requestList.begin()->nkPos, m_requestList.begin()->nkDestPos, m_mNodeList.end());
    }
}

/**
* Runs A* computation for a specific duration on the request. 
* If request completed, returns true; otherwise false.
*/
bool WorldData::RunComputationLoop(const PathRequest& req)
{
    bool bComplete = false;

    // loop until out of computation time (5ms)
    float fStartTime = g_time.GetCurTime();
    while( (g_time.GetCurTime() - fStartTime) < 0.005 )
    {
        // get waypoint list
        PathWaypointList* waypointList = &(m_requestList.begin()->waypointList);

        // pop lowest cost
        NodeList::iterator nLowest = FindLowestCostNode();

        // if no nodes open, done
        if( nLowest == m_mNodeList.end() )
        {
            // no path, push current position
            waypointList->push_back( GetCoordinates(req.nkPos) );
            bComplete = true;
            break;
        }

        // if lowest cost is destination, done
        else if( nLowest->loc.iRow == req.nkDestPos.iRow && nLowest->loc.iCol == req.nkDestPos.iCol )
        {
            // push all waypoints
            AddWaypoints(waypointList, nLowest);

            // run catmull-rom if requested
            if(m_smooth)
            {
                SmoothWaypoints(waypointList);
            }
            
            bComplete = true;
            break;
        }

        // otherwise, add all neighboring nodes
        else
        {
            // add neighbor nodes
            AddNeighborNodes(nLowest, req.nkDestPos);

            // move node to closed list
            nLowest->bClosed = true;
            
            // set terrain color (for closed node)
	        //g_terrain.SetColor( nLowest->loc.iRow, nLowest->loc.iCol, DEBUG_COLOR_YELLOW );
        }
    }

    return bComplete;
}

/**
* Push all waypoints to the movement list
*/
void WorldData::AddWaypoints(PathWaypointList* waypointList, NodeList::iterator position)
{
    NodeList::iterator prev = m_mNodeList.end();
    NodeList::iterator next = m_mNodeList.end();

    // until starting parent node reached
    while(position != m_mNodeList.end())
    {
        // set next node
        next = position->nParent;

        // check for node rubberbanding if requested
        if( m_rubberband && CheckNodeRubberband(position, prev, next) )
        {
            // remove node
            prev->nParent = next;
            position = prev;
        }

        // node required, push point to front
        else
        {
            D3DXVECTOR2 point = GetCoordinates( position->loc );
            waypointList->push_front( point );
        }

        // set previous and move to next position
        prev = position;
        position = position->nParent;
    }  
}

/**
* Smooth the specified waypoint list using catmull-rom.
*/
void WorldData::SmoothWaypoints(PathWaypointList* waypointList)
{
    float fTotalDist = 0.0f;
    float fAvgDist = 0.0f;
    int iPointCount = 0;

    // find average point distance
    for(PathWaypointList::iterator cur = waypointList->begin(); cur != waypointList->end(); ++cur)
    {
        // find next point
        PathWaypointList::iterator next = cur; 
        ++next;

        // compute distance vector between current and next point
        if( next != waypointList->end() )
        {
            D3DXVECTOR2 vDist = (*next) - (*cur);

            // add distance to total
            fTotalDist += D3DXVec2Length(&vDist);
            ++iPointCount;
        }
    }

    // compute average
    fAvgDist = fTotalDist / iPointCount;

    // make points evenly spaced (for catmull-rom)
    PathWaypointList::iterator cur = waypointList->begin();

    while( cur != waypointList->end() )
    {
        // find next point
        PathWaypointList::iterator next = cur; 
        ++next;

        // check for valid next point
        if( next != waypointList->end() )
        {
            // compute distance vector between current and next point
            D3DXVECTOR2 vDist = (*next) - (*cur);

            // if distance greater than average, insert a new point in the middle
            if( D3DXVec2Length(&vDist) > fAvgDist )
            {
                D3DXVECTOR2 vNewPoint = (*cur) + vDist/2.0f;
                waypointList->insert(next, vNewPoint);
                continue;
            }
        }

        // move to next point
        ++cur;
    }

    // add catmull rom points
    PathWaypointList::iterator p1 = waypointList->begin(); 
    
    while(p1 != waypointList->end())
    {
        // determine next point
        PathWaypointList::iterator next = p1; ++next; 
        
        // compute points from p1
        PathWaypointList::iterator p0 = p1; 
        PathWaypointList::iterator p2 = p1; 
        PathWaypointList::iterator p3 = p1;

        // compute previous point (p0)
        if(p0 != waypointList->begin())
        {
            --p0;
        }

        // compute next points (decrement if last point)
        ++p2;
        if(p2 == waypointList->end()) 
        { 
            break;    
        }
        else
        {
            p3 = p2; 
            ++p3;
            if(p3 == waypointList->end())
            { 
                --p3; 
            }
        }

        // compute and add calmull-rom points
        D3DXVECTOR2 result;
        waypointList->insert(p2, *D3DXVec2CatmullRom(&result, &(*p0), &(*p1), &(*p2), &(*p3), 0.25f));
        waypointList->insert(p2, *D3DXVec2CatmullRom(&result, &(*p0), &(*p1), &(*p2), &(*p3), 0.50f));
        waypointList->insert(p2, *D3DXVec2CatmullRom(&result, &(*p0), &(*p1), &(*p2), &(*p3), 0.75f));

        // move past added points
        p1 = next;
    }
}

/**
* Check if node is unnecessary and should be removed from path.
* Return true if remove, else false
*/
bool WorldData::CheckNodeRubberband(NodeList::iterator position, NodeList::iterator prev, NodeList::iterator next)
{
    int iMinRow = position->loc.iRow;
    int iMaxRow = position->loc.iRow;
    int iMinCol = position->loc.iCol;
    int iMaxCol = position->loc.iCol;

    // check for valid prev and next nodes
    if(prev == m_mNodeList.end() || next == m_mNodeList.end())
    {
        // node position required for path
        return false;
    }

    // form node set bounds
    UpdateRubberbandBounds(prev, iMinRow, iMaxRow, iMinCol, iMaxCol);
    UpdateRubberbandBounds(next, iMinRow, iMaxRow, iMinCol, iMaxCol);

    // check each point for wall
    for(int row = iMinRow; row <= iMaxRow; row++)
    {
        for(int col = iMinCol; col <= iMaxCol; col++)
        {
            // wall found, node required for path
            if( m_worldFile(row, col) == WorldFile::OCCUPIED_CELL )
                return false;
        }
    }

    return true;
}

/**
* Update row and column bounds for checking rubberbanding.
*/
void WorldData::UpdateRubberbandBounds(NodeList::iterator node, int& iMinRow, int& iMaxRow, int& iMinCol, int& iMaxCol)
{
    // minimum row
    if(iMinRow > node->loc.iRow)
        iMinRow = node->loc.iRow;

    // maximum row
    if(iMaxRow < node->loc.iRow)
        iMaxRow = node->loc.iRow;

    // minimum column
    if(iMinCol > node->loc.iCol)
        iMinCol = node->loc.iCol;

    // maximum column
    if(iMaxCol < node->loc.iCol)
        iMaxCol = node->loc.iCol;
}

/**
* Add all neighboring nodes
*/
void WorldData::AddNeighborNodes(NodeList::iterator position, const NodeKey& nkDest)
{
    NodeKey nkPos = position->loc;
    NodeKey nkNew;

    bool bUpWall;
    bool bDownWall;
    bool bLeftWall;
    bool bRightWall;

    ////////////////////
    // ADJACENT NODES //
    ////////////////////

    // up
    nkNew.iRow = nkPos.iRow + 1;
    nkNew.iCol = nkPos.iCol;

    if( !(bUpWall = (m_worldFile(nkNew.iRow, nkNew.iCol) == WorldFile::OCCUPIED_CELL)) )
        CreateNode(nkNew, nkDest, position);

    // right
    nkNew.iRow = nkPos.iRow;
    nkNew.iCol = nkPos.iCol + 1;

    if( !(bRightWall = (m_worldFile(nkNew.iRow, nkNew.iCol) == WorldFile::OCCUPIED_CELL)) )
        CreateNode(nkNew, nkDest, position);

    // down
    nkNew.iRow = nkPos.iRow - 1;
    nkNew.iCol = nkPos.iCol;

    if( !(bDownWall = (m_worldFile(nkNew.iRow, nkNew.iCol) == WorldFile::OCCUPIED_CELL)) )
        CreateNode(nkNew, nkDest, position);

    // left
    nkNew.iRow = nkPos.iRow;
    nkNew.iCol = nkPos.iCol - 1;

    if( !(bLeftWall = (m_worldFile(nkNew.iRow, nkNew.iCol) == WorldFile::OCCUPIED_CELL)) )
        CreateNode(nkNew, nkDest, position);

    ////////////////////
    // DIAGONAL NODES //
    ////////////////////

    // upper right
    nkNew.iRow = nkPos.iRow + 1;
    nkNew.iCol = nkPos.iCol + 1;

    if( !(m_worldFile(nkNew.iRow, nkNew.iCol) == WorldFile::OCCUPIED_CELL) && !bUpWall && !bRightWall )
        CreateNode(nkNew, nkDest, position);

    // lower right
    nkNew.iRow = nkPos.iRow - 1;
    nkNew.iCol = nkPos.iCol + 1;

    if( !(m_worldFile(nkNew.iRow, nkNew.iCol) == WorldFile::OCCUPIED_CELL) && !bDownWall && !bRightWall )
        CreateNode(nkNew, nkDest, position);

    // lower left
    nkNew.iRow = nkPos.iRow - 1;
    nkNew.iCol = nkPos.iCol - 1;

    if( !(m_worldFile(nkNew.iRow, nkNew.iCol) == WorldFile::OCCUPIED_CELL) && !bDownWall && !bLeftWall )
        CreateNode(nkNew, nkDest, position);

    // upper left
    nkNew.iRow = nkPos.iRow + 1;
    nkNew.iCol = nkPos.iCol - 1;

    if( !(m_worldFile(nkNew.iRow, nkNew.iCol) == WorldFile::OCCUPIED_CELL) && !bUpWall && !bLeftWall )
        CreateNode(nkNew, nkDest, position);
}

/**
* Creates a new node
*/
void WorldData::CreateNode(const NodeKey& nkPos, const NodeKey& nkDest, NodeList::iterator parent)
{
    NodeData data;

    // check if within bounds
    if( nkPos.iCol >= m_worldFile.GetWidth() ||
        nkPos.iCol < 0 ||
        nkPos.iRow >= m_worldFile.GetHeight() ||
        nkPos.iRow < 0)
    {
        return;
    }

    // set location data
    data.loc = nkPos;

    // set parent node
    data.nParent = parent;

    // compute current distance cost
    if(parent != m_mNodeList.end())
    {
        data.fDistanceCost = ComputeDistanceCost(nkPos, parent);
    }
    else
    {
        data.fDistanceCost = 0.0f;
    }

    // compute heustic cost (future distance)
    data.fHeuristicCost = m_heuristicWeight * ComputeHeuristicCost(nkPos, nkDest);

    // compute total cost
    data.fTotalCost = data.fDistanceCost + data.fHeuristicCost;

    // initialize
    data.bClosed = false;

    // check if it exists
    NodeList::iterator node;

    for(node = m_mNodeList.begin(); node != m_mNodeList.end(); ++node)
    {
        if( (node->loc.iRow == nkPos.iRow) && (node->loc.iCol == nkPos.iCol) )
        {
            break;
        }
    }

    // node does not exist
    if( node == m_mNodeList.end() )
    {
        m_mNodeList.push_back(data);
	    //g_terrain.SetColor( nkPos.iRow, nkPos.iCol, DEBUG_COLOR_BLUE );
    }

    // node exists with higher cost
    else if( node->fTotalCost > data.fTotalCost )
    {
        (*node) = data;
	    //g_terrain.SetColor( nkPos.iRow, nkPos.iCol, DEBUG_COLOR_BLUE );
    }

    // node exists with lower cost
    else
    {
        // ignore
    }
}

/**
* Compute distance cost
*/
float WorldData::ComputeDistanceCost(const NodeKey& nkPos, NodeList::iterator parent)
{
    // get node and parent position
    D3DXVECTOR2 vNodePos = GetCoordinates(nkPos);
    D3DXVECTOR2 vParentPos = GetCoordinates(parent->loc);
    D3DXVECTOR2 vDist;

    // compute distance between node and parent (add to parent distance)
    vDist = vNodePos - vParentPos;
    return parent->fDistanceCost + D3DXVec2Length( &vDist );
}

/**
* Compute heuristic cost
*/
float WorldData::ComputeHeuristicCost(const NodeKey& nkPos, const NodeKey& nkDest)
{
    // get node and destination position
    D3DXVECTOR2 vNodePos = GetCoordinates(nkPos);
    D3DXVECTOR2 vDestPos = GetCoordinates(nkDest);
    
    float cost = 0.0f;

    // cardinal/intercardinal
    if(m_heuristicCalc)
    {
        // compute axis differences
        float xDiff = abs(vNodePos.x - vDestPos.x);
        float yDiff = abs(vNodePos.y - vDestPos.y);

        // compute cost
        cost = min(xDiff,yDiff) * sqrt(2.0f) + max(xDiff,yDiff) - min(xDiff,yDiff);
    }

    // eucladian
    else
    {
        // calculate distance vector
        D3DXVECTOR2 vDist;
        vDist = vNodePos - vDestPos;

        // compute cost
        cost = D3DXVec2Length( &vDist );
    }

    return cost;
}

/**
* Find lowest cost node
*/
WorldData::NodeList::iterator WorldData::FindLowestCostNode()
{
    NodeList::iterator nLowest = m_mNodeList.end();

    for(NodeList::iterator it = m_mNodeList.begin(); it != m_mNodeList.end(); ++it)
    {     
        // if not closed
        if(!it->bClosed)
        {
            // if nothing found yet or lowest cost, set node
            if( nLowest == m_mNodeList.end() || it->fTotalCost < nLowest->fTotalCost )
            {
                nLowest = it;
            }
        }
    }

    return nLowest;
}

/**
* Get coordinates
*/
D3DXVECTOR2 WorldData::GetCoordinates( const NodeKey& key )
{
	D3DXVECTOR2 pos;
	pos.x = ((float)key.iCol) + 0.5f;
	pos.y = ((float)key.iRow) + 0.5f;
	
	return( pos );
}

/**
* Get row and column from position
*/
void WorldData::GetRowColumn( const D3DXVECTOR2& pos, NodeKey* key )
{
	key->iCol = (int)(pos.x);
	key->iRow = (int)(pos.y);
}
