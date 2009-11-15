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

// custom FVF, which describes the custom vertex structure
const DWORD D3DFVF_CUSTOMVERTEX = (D3DFVF_XYZ | D3DFVF_TEX1);

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
    m_pFloorVertexBuffer(NULL),
    m_iFloorCVCount(0),
    m_iFloorCVBufferSize(0),
    m_FloorCVBuffer(NULL),
    m_pWallTexture(NULL),
    m_iWallTriangleCount(0),
    m_pWallVertexBuffer(NULL),
    m_iWallCVCount(0),
    m_iWallCVBufferSize(0),
    m_WallCVBuffer(NULL)
{
}

/**
* Deconstuct world node.
*/
WorldNode::~WorldNode()
{
    // cleanup D3D vertex buffers
    SAFE_RELEASE(m_pFloorVertexBuffer);
    SAFE_RELEASE(m_pWallVertexBuffer);

    // cleanup textures
    SAFE_RELEASE(m_pFloorTexture);
    SAFE_RELEASE(m_pWallTexture);

    // cleanup custom vertex buffers
    SAFE_DELETE_ARRAY(m_FloorCVBuffer);
    SAFE_DELETE_ARRAY(m_WallCVBuffer);
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

    // setup rendering buffers
    HRESULT result = S_OK;
    
    if(m_iFloorCVCount)
        result = CreateRenderBuffer(pd3dDevice, m_FloorCVBuffer, m_iFloorCVCount, &m_pFloorVertexBuffer);
    if(m_iWallCVCount)
        result = CreateRenderBuffer(pd3dDevice, m_WallCVBuffer, m_iWallCVCount, &m_pWallVertexBuffer);

    return result;
}

/**
* Setup D3D vertex buffers for rendering. Uses verticies in floor and wall buffers for
* creating the vertex buffer.
*/
HRESULT WorldNode::CreateRenderBuffer(
    IDirect3DDevice9* pd3dDevice, 
    CustomVertex* pCustomVertices,
    int iVertexCount,
    LPDIRECT3DVERTEXBUFFER9* ppVertexBuffer)
{
    // create the vertex buffer.
    HRESULT result = pd3dDevice->CreateVertexBuffer( 
        iVertexCount*sizeof(CustomVertex),
        0, 
        D3DFVF_CUSTOMVERTEX,
        D3DPOOL_DEFAULT, 
        ppVertexBuffer, 
        NULL);

    if(SUCCEEDED(result))
    {
        // fill the vertex buffer.
        VOID* pVertices;
        result = (*ppVertexBuffer)->Lock(0, iVertexCount*sizeof(CustomVertex), (void**)&pVertices, 0);

        if(SUCCEEDED(result))
        {
            memcpy(pVertices, pCustomVertices, iVertexCount*sizeof(CustomVertex));
            (*ppVertexBuffer)->Unlock();
        }
    }

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
void WorldNode::RenderNode(IDirect3DDevice9* pd3dDevice, D3DXMATRIX rMatWorld)
{
	// Set the world space transform
	pd3dDevice->SetTransform(D3DTS_WORLD, &rMatWorld);

	// Turn off culling
	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    // Turn off D3D lighting, since we are providing our own vertex colors
    pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

    // Set vertex type
    pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );

    // set floor texture and draw primitives
    if(m_iFloorTriangleCount)
    {
        pd3dDevice->SetTexture(0, m_pFloorTexture);
        pd3dDevice->SetStreamSource( 0, m_pFloorVertexBuffer, 0, sizeof(CustomVertex) );
        pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, m_iFloorTriangleCount );

    }

    // set wall texture and draw primitives
    if(m_iWallTriangleCount)
    {
        pd3dDevice->SetTexture(0, m_pWallTexture);
        pd3dDevice->SetStreamSource( 0, m_pWallVertexBuffer, 0, sizeof(CustomVertex) );
        pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, m_iWallTriangleCount );
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
    // (0,0,1) (1,0,1)
    // --------------
    // |      U     |
    // |            |
    // | L        R |
    // |            |
    // |      D     |
    // --------------
    // (0,0,0) (1,0,0)

    // NOTE: In most cases

    switch(side)
    {

    case kTop:
        AddPlane(
            CustomVertex( x,      y+s,  z,    0.0f, 0.0f ), // u=x, v=z
            CustomVertex( x,      y+s,  z+s,  0.0f, 1.0f ),
            CustomVertex( x+s,    y+s,  z+s,  1.0f, 1.0f ),
            type);
        AddPlane(
            CustomVertex( x,      y+s,  z,    0.0f, 0.0f ),
            CustomVertex( x+s,    y+s,  z,    1.0f, 0.0f ),
            CustomVertex( x+s,    y+s,  z+s,  1.0f, 1.0f ),
            type);
        break;

    case kBottom:    
        AddPlane(
            CustomVertex( x,      y,    z,    0.0f, 0.0f ), // u=x, v=z
            CustomVertex( x,      y,    z+s,  0.0f, 1.0f ),
            CustomVertex( x+s,    y,    z+s,  1.0f, 1.0f ),
            type);
        AddPlane(
            CustomVertex( x,      y,    z,    0.0f, 0.0f ),
            CustomVertex( x+s,    y,    z,    1.0f, 0.0f ),
            CustomVertex( x+s,    y,    z+s,  1.0f, 1.0f ),
            type);
        break;

    case kLeft:        
        AddPlane(
            CustomVertex( x,      y,    z,    0.0f, 0.0f ), // u=y, v=z (except mid vertex)
            CustomVertex( x,      y,    z+s,  1.0f, 0.0f ), 
            CustomVertex( x,      y+s,  z+s,  1.0f, 1.0f ),
            type);
        AddPlane(
            CustomVertex( x,      y,    z,    0.0f, 0.0f ),
            CustomVertex( x,      y+s,  z,    0.0f, 1.0f ),
            CustomVertex( x,      y+s,  z+s,  1.0f, 1.0f ),
            type);
        break;

    case kRight:
        AddPlane(
            CustomVertex( x+s,    y,    z,    0.0f, 0.0f ), // u=y, v=z (except mid vertex)
            CustomVertex( x+s,    y,    z+s,  1.0f, 0.0f ),
            CustomVertex( x+s,    y+s,  z+s,  1.0f, 1.0f ),
            type);
        AddPlane(
            CustomVertex( x+s,    y,    z,    0.0f, 0.0f ),
            CustomVertex( x+s,    y+s,  z,    0.0f, 1.0f ),
            CustomVertex( x+s,    y+s,  z+s,  1.0f, 1.0f ),
            type);
        break;

    case kUpper:
        AddPlane(
            CustomVertex( x,      y,    z+s,  0.0f, 0.0f ), // u=x, v=y
            CustomVertex( x,      y+s,  z+s,  0.0f, 1.0f ),
            CustomVertex( x+s,    y,    z+s,  1.0f, 0.0f ),
            type);
        AddPlane(
            CustomVertex( x+s,    y,    z+s,  1.0f, 0.0f ),
            CustomVertex( x+s,    y+s,  z+s,  1.0f, 1.0f ),
            CustomVertex( x,      y+s,  z+s,  0.0f, 1.0f ),
            type);
        break;

    case kLower:
        AddPlane(
            CustomVertex( x,      y,    z,    0.0f, 0.0f ), // u=x, v=y
            CustomVertex( x,      y+s,  z,    0.0f, 1.0f ),
            CustomVertex( x+s,    y,    z,    1.0f, 0.0f ),
            type);
        AddPlane(
            CustomVertex( x+s,    y,    z,    1.0f, 0.0f ), // u=x, v=y
            CustomVertex( x+s,    y+s,  z,    1.0f, 1.0f ),
            CustomVertex( x,      y+s,  z,    0.0f, 1.0f ),
            type);
        break;

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
