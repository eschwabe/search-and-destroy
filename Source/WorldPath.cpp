/*******************************************************************************
* Game Development Project
* WorldPath.cpp
*
* Eric Schwabe
* 2010-05-16
*
* World Pathing
*
*******************************************************************************/

#include "DXUT.h"
#include "WorldPath.h"

/**
* Constructor
*/
WorldPath::WorldPath(const WorldFile& worldFile) :
    GameObject(g_database.GetNewObjectID(), OBJECT_Debug, "PATH_DEBUG"),
    m_bPathInProgress(false),
    m_worldFile(worldFile),
    m_rubberband(true),
    m_heuristicCalc(true),
    m_smooth(true),
    m_heuristicWeight(1.01f),
    m_debuglines(true)
{
}

/**
* Deconstructor
*/
WorldPath::~WorldPath()
{
}

/**
* Initialize object
*/
HRESULT WorldPath::Initialize(IDirect3DDevice9* pd3dDevice)
{
    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);
}

/**
* Update object
*/
void WorldPath::Update()
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
}

/**
* Create line position from grid position
*/
D3DXVECTOR3 WorldPath::CreateLinePosition(const D3DXVECTOR2& pos)
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
D3DXCOLOR WorldPath::GetColor( DebugDrawingColor color )
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
void WorldPath::AddLine( D3DXVECTOR3 vP1, D3DXVECTOR3 vP2, DebugDrawingColor dColor, bool bArrowHead )
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
void WorldPath::Render(IDirect3DDevice9* pd3dDevice, const RenderData* rData)
{
    // check if any lines
    if(m_vLines.size() && m_debuglines)
    {
        // set the texture (or unset)
        pd3dDevice->SetTexture(0, NULL);

        // set the world space transform
        pd3dDevice->SetTransform(D3DTS_WORLD, &rData->matWorld);

        // turn off D3D lighting, since we are providing our own vertex colors
        pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

        // set vertex type
        pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );

        // draw
        pd3dDevice->DrawPrimitiveUP( D3DPT_LINELIST, m_vLines.size()/2, &(m_vLines[0]), sizeof(m_vLines[0]) );
    }
}

/**
* Finds a random location in the map that an object can move to (i.e. non-wall location)
*/
D3DXVECTOR2 WorldPath::GetRandomMapLocation()
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
void WorldPath::AddPathRequest(
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
PathWaypointList* WorldPath::GetWaypointList(objectID id)
{
    return &(m_completeWaypointLists[id]);
}

/**
* Clears an object waypoint list.
*/
void WorldPath::ClearWaypointList(objectID id)
{
    m_completeWaypointLists.erase(id);
}

/**
* Runs a single pass of the A* computation. Updates state when computation complete.
* Compute paths. Only works on path calculations for a specific interval.
*/
void WorldPath::ComputePaths()
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
bool WorldPath::RunComputationLoop(const PathRequest& req)
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
void WorldPath::AddWaypoints(PathWaypointList* waypointList, NodeList::iterator position)
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
void WorldPath::SmoothWaypoints(PathWaypointList* waypointList)
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
bool WorldPath::CheckNodeRubberband(NodeList::iterator position, NodeList::iterator prev, NodeList::iterator next)
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
void WorldPath::UpdateRubberbandBounds(NodeList::iterator node, int& iMinRow, int& iMaxRow, int& iMinCol, int& iMaxCol)
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
void WorldPath::AddNeighborNodes(NodeList::iterator position, const NodeKey& nkDest)
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
void WorldPath::CreateNode(const NodeKey& nkPos, const NodeKey& nkDest, NodeList::iterator parent)
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
float WorldPath::ComputeDistanceCost(const NodeKey& nkPos, NodeList::iterator parent)
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
float WorldPath::ComputeHeuristicCost(const NodeKey& nkPos, const NodeKey& nkDest)
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
WorldPath::NodeList::iterator WorldPath::FindLowestCostNode()
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
D3DXVECTOR2 WorldPath::GetCoordinates( const NodeKey& key )
{
	D3DXVECTOR2 pos;
	pos.x = ((float)key.iCol) + 0.5f;
	pos.y = ((float)key.iRow) + 0.5f;
	
	return( pos );
}

/**
* Get row and column from position
*/
void WorldPath::GetRowColumn( const D3DXVECTOR2& pos, NodeKey* key )
{
	key->iCol = (int)(pos.x);
	key->iRow = (int)(pos.y);
}
