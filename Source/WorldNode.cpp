/*******************************************************************************
* Game Development Project
* WorldNode.h
*
* Eric Schwabe
* 2009-11-13
*
* World Node (i.e. Level)
*
*******************************************************************************/

#include "DXUT.h"
#include "WorldNode.h"
#include "WorldFile.h"
#include "DXUT\SDKmisc.h"

// world scale
const float kScale = 1.0f;

// custom vertex structure definition
const D3DVERTEXELEMENT9 WorldNode::m_sCustomVertexDeclaration[] =
{
    { 0, offsetof(CustomVertex, vPos        ), D3DDECLTYPE_FLOAT3  , 0, D3DDECLUSAGE_POSITION, 0 },
    { 0, offsetof(CustomVertex, vNormal     ), D3DDECLTYPE_FLOAT3  , 0, D3DDECLUSAGE_NORMAL  , 0 },
    { 0, offsetof(CustomVertex, vTexCoord   ), D3DDECLTYPE_FLOAT2  , 0, D3DDECLUSAGE_TEXCOORD, 0 },
    { 0, offsetof(CustomVertex, cDiffuse    ), D3DDECLTYPE_D3DCOLOR, 0, D3DDECLUSAGE_COLOR   , 0 },
    { 0, offsetof(CustomVertex, cSpecular   ), D3DDECLTYPE_D3DCOLOR, 0, D3DDECLUSAGE_COLOR   , 1 },
    D3DDECL_END(),
};

/**
* Constuct world node.
*
* @param sGridFilename level grid file to load
* @param sFloorFilename floor texture file
* @param sWallFilename wall texture file
*/
WorldNode::WorldNode(
        const LPCWSTR sGridFilename, 
        const LPCWSTR sFloorFilename, 
        const LPCWSTR sWallFilename) :
    m_sGridFilename(sGridFilename),
    m_sFloorFilename(sFloorFilename),
    m_sWallFilename(sWallFilename),
    m_pFloorTexture(NULL),
    m_iFloorTriangleCount(0),
    m_iFloorCVCount(0),
    m_iFloorCVBufferSize(0),
    m_FloorCVBuffer(NULL),
    m_pWallTexture(NULL),
    m_iWallTriangleCount(0),
    m_iWallCVCount(0),
    m_iWallCVBufferSize(0),
    m_WallCVBuffer(NULL),
    iWorldHeight(0),
    iWorldWidth(0)
{
}

/**
* Deconstuct world node.
*/
WorldNode::~WorldNode()
{
    // cleanup vertex declaration
    SAFE_RELEASE(m_pCVDeclaration);

    // cleanup textures
    SAFE_RELEASE(m_pFloorTexture);
    SAFE_RELEASE(m_pWallTexture);

    // cleanup custom vertex buffers
    SAFE_DELETE_ARRAY(m_FloorCVBuffer);
    SAFE_DELETE_ARRAY(m_WallCVBuffer);
}

/*
* Returns a list of all quads (walls). The list will be empty if the world node
* has not been initialized yet.
*/
VecCollQuad WorldNode::GetCollisionQuadList()
{
    return m_vCollQuads;
}

/**
* Initialize world node.
*/
HRESULT WorldNode::InitializeNode(IDirect3DDevice9* pd3dDevice)
{
    WCHAR wsNewPath[ MAX_PATH ];

    // search and load floor texture
    DXUTFindDXSDKMediaFileCch(wsNewPath, sizeof(wsNewPath), m_sFloorFilename.c_str());
    if( FAILED(D3DXCreateTextureFromFile(
        pd3dDevice,
        wsNewPath,
        &m_pFloorTexture)) )
    {
        return E_FAIL;
    }

    // search and load wall texture
    DXUTFindDXSDKMediaFileCch(wsNewPath, sizeof(wsNewPath), m_sWallFilename.c_str());
    if( FAILED(D3DXCreateTextureFromFile(
        pd3dDevice,
        wsNewPath,
        &m_pWallTexture)) )
    {
        return E_FAIL;
    }

    // load grid data
    WorldFile grid;

    if( !grid.Load(m_sGridFilename.c_str()) )
    {
        return E_FAIL;
    }

    // save grid size
    iWorldHeight = grid.GetHeight();
    iWorldWidth = grid.GetWidth();

    // draw tiles for each row and column entry
    for(int col = 0; col < grid.GetWidth(); col++)
    {
        for(int row = 0; row < grid.GetHeight(); row++)
        {
            // compute cube base coordinates
            float x = col * kScale;
            float y = 0.0f;
            float z = row * kScale;

            // check row/col for cell
            switch(grid(row,col))
            {
                case WorldFile::OCCUPIED_CELL:
                {
                    // draw cube top
                    DrawTile(x, y, z, kScale, kTop, kWall);

                    // check for occupied cells next to cell, draw cube sides if not occupied

                    // left
                    if(grid(row,col-1) == WorldFile::EMPTY_CELL)
                        DrawTile(x, y, z, kScale, kLeft, kWall);

                    // right
                    if(grid(row,col+1) == WorldFile::EMPTY_CELL)
                        DrawTile(x, y, z, kScale, kRight, kWall);

                    // upper
                    if(grid(row+1,col) == WorldFile::EMPTY_CELL)
                        DrawTile(x, y, z, kScale, kUpper, kWall);

                    // lower
                    if(grid(row-1,col) == WorldFile::EMPTY_CELL)
                        DrawTile(x, y, z, kScale, kLower, kWall);

                    break;
                }
                default:
                {
                    // draw floor
                    DrawTile(x, y, z, kScale, kBottom, kFloor);
                    break;
                }
            }
        }
    }
    
    // setup world collision walls
    SetupWorldCollWalls(grid);

    // create vertex declaration
    HRESULT result = pd3dDevice->CreateVertexDeclaration(m_sCustomVertexDeclaration, &m_pCVDeclaration);

    return result;
}

/**
* Update world node.
*/
void WorldNode::UpdateNode(double /* fTime */)
{
}

/**
* Render world node.
*
* Draw the triangles in the vertex buffer. This is broken into a few
* steps. We are passing the Vertices down a "stream", so first we need
* to specify the source of that stream, which is our vertex buffer. Then
* we need to let D3D know what vertex shader to use. Full, custom vertex
* shaders are an advanced topic, but in most cases the vertex shader is
* just the FVF, so that D3D knows what type of Vertices we are dealing
* with. Finally, we call DrawPrimitive() which does the actual rendering
* of our geometry.
*/
void WorldNode::RenderNode(IDirect3DDevice9* pd3dDevice, const RenderData& rData)
{
    // setup shaders
    rData.EnableDirectionalShaders(pd3dDevice);

    // set vertex declaration
    pd3dDevice->SetVertexDeclaration(m_pCVDeclaration);
    
    // set floor texture and draw primitives
    if(m_iFloorTriangleCount)
    {
        pd3dDevice->SetTexture(0, m_pFloorTexture);
        pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, m_iFloorTriangleCount, m_FloorCVBuffer, sizeof(m_FloorCVBuffer[0]) );
    }

    // set wall texture and draw primitives
    if(m_iWallTriangleCount)
    {
        pd3dDevice->SetTexture(0, m_pWallTexture);
        pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, m_iWallTriangleCount, m_WallCVBuffer, sizeof(m_WallCVBuffer[0]) );
    }
}

/**
* Draw a cube into the verticies buffer. Draws the cube in the x-z plane at 
* the specified row and column with a set height and color.
*
* In most cases, the texture coordinates are the same as the offsets for the cube
* vertex coordinates. Minor exception for left and right side to ensure texture
* is not sideways.
*
* @param x coordinate location
* @param y coordinate location
* @param z coordinate location
* @param size cube scaling size
* @param side cube side to draw
* @param type tile type to draw
*/
void WorldNode::DrawTile(float x, float y, float z, float s, CubeSide side, TileType type)
{
    D3DXVECTOR3 n; // normal

    switch(side)
    {

    case kTop:
    {
        D3DXVECTOR3 p1( x,   y+s, z   );
        D3DXVECTOR3 p2( x,   y+s, z+s );
        D3DXVECTOR3 p3( x+s, y+s, z+s );
        D3DXVECTOR3 p4( x+s, y+s, z   );
        D3DXVec3Cross(&n, &(p2-p1), &(p3-p1));
        AddPlane(
            CustomVertex( p1, n, D3DXVECTOR2(0.0f, 1.0f) ),
            CustomVertex( p2, n, D3DXVECTOR2(0.0f, 0.0f) ),
            CustomVertex( p3, n, D3DXVECTOR2(1.0f, 0.0f) ),
            type);
        AddPlane(
            CustomVertex( p1, n, D3DXVECTOR2(0.0f, 1.0f) ),
            CustomVertex( p3, n, D3DXVECTOR2(1.0f, 0.0f) ),
            CustomVertex( p4, n, D3DXVECTOR2(1.0f, 1.0f) ),
            type);
        m_vCollQuads.push_back( CollQuad(p1, p2, p3, p4, n) ); 
        break;
    }
    case kBottom:
    {
        D3DXVECTOR3 p1( x,   y,   z   );
        D3DXVECTOR3 p2( x,   y,   z+s );
        D3DXVECTOR3 p3( x+s, y,   z+s );
        D3DXVECTOR3 p4( x+s, y,   z   );
        D3DXVec3Cross(&n, &(p2-p1), &(p3-p1));
        AddPlane(
            CustomVertex( p1, n, D3DXVECTOR2(0.0f, 1.0f) ),
            CustomVertex( p2, n, D3DXVECTOR2(0.0f, 0.0f) ),
            CustomVertex( p3, n, D3DXVECTOR2(1.0f, 0.0f) ),
            type);
        AddPlane(
            CustomVertex( p1, n, D3DXVECTOR2(0.0f, 1.0f) ),
            CustomVertex( p3, n, D3DXVECTOR2(1.0f, 0.0f) ),
            CustomVertex( p4, n, D3DXVECTOR2(1.0f, 1.0f) ),
            type);
        m_vCollQuads.push_back( CollQuad(p1, p2, p3, p4, n) ); 
        break;
    }
    case kLeft:        
    {
        D3DXVECTOR3 p1( x,   y,   z   );
        D3DXVECTOR3 p2( x,   y,   z+s );
        D3DXVECTOR3 p3( x,   y+s, z+s );
        D3DXVECTOR3 p4( x,   y+s, z   );
        D3DXVec3Cross(&n, &(p2-p1), &(p3-p1));
        AddPlane(
            CustomVertex( p1, n, D3DXVECTOR2(1.0f, 1.0f) ),
            CustomVertex( p2, n, D3DXVECTOR2(0.0f, 1.0f) ), 
            CustomVertex( p3, n, D3DXVECTOR2(0.0f, 0.0f) ),
            type);
        AddPlane(
            CustomVertex( p1, n, D3DXVECTOR2(1.0f, 1.0f) ),
            CustomVertex( p3, n, D3DXVECTOR2(0.0f, 0.0f) ),
            CustomVertex( p4, n, D3DXVECTOR2(1.0f, 0.0f) ),
            type);
        m_vCollQuads.push_back( CollQuad(p1, p2, p3, p4, n) ); 
        break;
    }
    case kRight:
    {
        D3DXVECTOR3 p1( x+s, y,    z   );
        D3DXVECTOR3 p2( x+s, y+s,  z   );
        D3DXVECTOR3 p3( x+s, y+s,  z+s );
        D3DXVECTOR3 p4( x+s, y,    z+s );
        D3DXVec3Cross(&n, &(p2-p1), &(p3-p1));
        AddPlane(
            CustomVertex( p1, n, D3DXVECTOR2(0.0f, 1.0f) ),
            CustomVertex( p2, n, D3DXVECTOR2(0.0f, 0.0f) ),
            CustomVertex( p3, n, D3DXVECTOR2(1.0f, 0.0f) ),
            type);
        AddPlane(
            CustomVertex( p1, n, D3DXVECTOR2(0.0f, 1.0f) ),
            CustomVertex( p3, n, D3DXVECTOR2(1.0f, 0.0f) ),
            CustomVertex( p4, n, D3DXVECTOR2(1.0f, 1.0f) ),
            type);
        m_vCollQuads.push_back( CollQuad(p1, p2, p3, p4, n) ); 
        break;
    }
    case kUpper:
    {
        D3DXVECTOR3 p1( x,   y,   z+s );
        D3DXVECTOR3 p2( x+s, y,   z+s );
        D3DXVECTOR3 p3( x+s, y+s, z+s );
        D3DXVECTOR3 p4( x,   y+s, z+s );
        D3DXVec3Cross(&n, &(p2-p1), &(p3-p1));
        AddPlane(
            CustomVertex( p1, n, D3DXVECTOR2(1.0f, 1.0f) ),
            CustomVertex( p2, n, D3DXVECTOR2(0.0f, 1.0f) ),
            CustomVertex( p3, n, D3DXVECTOR2(0.0f, 0.0f) ),
            type);
        AddPlane(
            CustomVertex( p1, n, D3DXVECTOR2(1.0f, 1.0f) ),
            CustomVertex( p3, n, D3DXVECTOR2(0.0f, 0.0f) ),
            CustomVertex( p4, n, D3DXVECTOR2(1.0f, 0.0f) ),
            type);
        m_vCollQuads.push_back( CollQuad(p1, p2, p3, p4, n) ); 
        break;
    }
    case kLower:
    {
        D3DXVECTOR3 p1( x,   y,   z   );
        D3DXVECTOR3 p2( x,   y+s, z   );
        D3DXVECTOR3 p3( x+s, y+s, z   );
        D3DXVECTOR3 p4( x+s, y,   z   );
        D3DXVec3Cross(&n, &(p2-p1), &(p3-p1));
        AddPlane(
            CustomVertex( p1, n, D3DXVECTOR2(0.0f, 1.0f) ),
            CustomVertex( p2, n, D3DXVECTOR2(0.0f, 0.0f) ),
            CustomVertex( p3, n, D3DXVECTOR2(1.0f, 0.0f) ),
            type);
        AddPlane(
            CustomVertex( p1, n, D3DXVECTOR2(0.0f, 1.0f) ),
            CustomVertex( p3, n, D3DXVECTOR2(1.0f, 0.0f) ),
            CustomVertex( p4, n, D3DXVECTOR2(1.0f, 1.0f) ),
            type);
        m_vCollQuads.push_back( CollQuad(p1, p2, p3, p4, n) ); 
        break;
    }
    default:
        break;
    }
}

/**
* Draw a triangle into a verticies buffer based on type. Draws a plane with the
* three verticies. Updates number of triangles for rendering.
* 
* @param v1 vertex
* @param v2 vertex
* @param v3 vertex
* @param type tile type
*/
void WorldNode::AddPlane(const CustomVertex& v1, const CustomVertex& v2, const CustomVertex& v3, const TileType& type)
{
    switch(type)
    {

    case kFloor:
        CheckBufferSize(m_iFloorCVCount, &m_iFloorCVBufferSize, &m_FloorCVBuffer);
        m_FloorCVBuffer[m_iFloorCVCount++] = v1;
        m_FloorCVBuffer[m_iFloorCVCount++] = v2;
        m_FloorCVBuffer[m_iFloorCVCount++] = v3;
        m_iFloorTriangleCount++;
        break;

    case kWall:
        CheckBufferSize(m_iWallCVCount, &m_iWallCVBufferSize, &m_WallCVBuffer);
        m_WallCVBuffer[m_iWallCVCount++] = v1;
        m_WallCVBuffer[m_iWallCVCount++] = v2;
        m_WallCVBuffer[m_iWallCVCount++] = v3;
        m_iWallTriangleCount++;
        break;

    default:
        break;

    }
}

/**
* Verifies the custom vertex buffer has enough space to add three vertices. If not,
* the buffer is grown.
*/
void WorldNode::CheckBufferSize(int iCVCount, int* iCVBufferSize, CustomVertex** buffer)
{
    if( (iCVCount+3) > (*iCVBufferSize) )
    {
        // allocate new buffer
        int tmpBufferSize = (*iCVBufferSize) + 100;
        CustomVertex* tmpBuffer = new CustomVertex[tmpBufferSize];

        // copy buffer data
        memcpy(tmpBuffer, *buffer, (*iCVBufferSize)*sizeof(CustomVertex));

        // delete old buffer
        SAFE_DELETE(*buffer);

        // set new buffer and size
        *buffer = tmpBuffer;
        *iCVBufferSize = tmpBufferSize;
    }
}

/**
* Adds collision quads for the world boundary walls. Prevents player from leaving
* the world node area.
*/
void WorldNode::SetupWorldCollWalls(const WorldFile& grid)
{
    // add boundary walls to quad list

    // world left side
    for(int row = 0; row < grid.GetHeight(); row++)
    {
        D3DXVECTOR3 p1( 0.0f, 0.0f,   (float)row );
        D3DXVECTOR3 p2( 0.0f, kScale, (float)row );
        D3DXVECTOR3 p3( 0.0f, kScale, (float)row+kScale );
        D3DXVECTOR3 p4( 0.0f, 0.0f,   (float)row+kScale );
        
        D3DXVECTOR3 n;
        D3DXVec3Cross(&n, &(p2-p1), &(p3-p1));

        m_vCollQuads.push_back( CollQuad(p1, p2, p3, p4, n) );
    }

    // world right side
    for(int row = 0; row < grid.GetHeight(); row++)
    {
        D3DXVECTOR3 p1( (float)grid.GetWidth(), 0.0f,   (float)row );
        D3DXVECTOR3 p2( (float)grid.GetWidth(), 0.0f,   (float)row+kScale );
        D3DXVECTOR3 p3( (float)grid.GetWidth(), kScale, (float)row+kScale );
        D3DXVECTOR3 p4( (float)grid.GetWidth(), kScale, (float)row );
        
        D3DXVECTOR3 n;
        D3DXVec3Cross(&n, &(p2-p1), &(p3-p1));

        m_vCollQuads.push_back( CollQuad(p1, p2, p3, p4, n) );
    }

    // world lower side
    for(int col = 0; col < grid.GetWidth(); col++)
    {
        D3DXVECTOR3 p1( (float)col,        0.0f, 0.0f );
        D3DXVECTOR3 p2( (float)col+kScale, 0.0f, 0.0f );
        D3DXVECTOR3 p3( (float)col+kScale, kScale, 0.0f );
        D3DXVECTOR3 p4( (float)col,        kScale, 0.0f );
        
        D3DXVECTOR3 n;
        D3DXVec3Cross(&n, &(p2-p1), &(p3-p1));

        m_vCollQuads.push_back( CollQuad(p1, p2, p3, p4, n) );
    }

    // world upper side
    for(int col = 0; col < grid.GetWidth(); col++)
    {
        D3DXVECTOR3 p1( (float)col,        0.0f,   (float)grid.GetHeight() );
        D3DXVECTOR3 p2( (float)col,        kScale, (float)grid.GetHeight() );
        D3DXVECTOR3 p3( (float)col+kScale, kScale, (float)grid.GetHeight() );
        D3DXVECTOR3 p4( (float)col+kScale, 0.0f,   (float)grid.GetHeight() );
        
        D3DXVECTOR3 n;
        D3DXVec3Cross(&n, &(p2-p1), &(p3-p1));

        m_vCollQuads.push_back( CollQuad(p1, p2, p3, p4, n) );
    }
}
