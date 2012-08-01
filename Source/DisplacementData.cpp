#include "StdAfx.h"
#include "DisplacementData.h"
#include "MemoryPool.h"

#define CURRENT_VERSION 100 // v1.00
#define MAX_LOADED_TILES 4
#define TILE_BYTES (m_directory.tileSize * m_directory.tileSize * sizeof(GridPoint))

DisplacementData::DisplacementData(void):
m_activeTiles(0)
{
    ZeroMemory(m_pTiles, FILE_TILE_DIMENSION * FILE_TILE_DIMENSION * sizeof(GridPoint*));
    ZeroMemory(m_lastUsed, FILE_TILE_DIMENSION * FILE_TILE_DIMENSION * sizeof(LONGLONG));
}


DisplacementData::~DisplacementData(void)
{
    for(int y=0; y < FILE_TILE_DIMENSION; ++y)
    {
        for(int x=0; x < FILE_TILE_DIMENSION; ++x)
        {
            if(m_pTiles[y][x])
            {
                if(!this->UnloadTile(x, y))
                {
                    LI_ERROR("failed to save tile before deleting");
                }
                m_pMemory->Free(m_pTiles[y][x]);
            }
        }
    }
    m_gridFile.close();
}


bool DisplacementData::Init(std::string p_filename, unsigned int p_fileTileSize)
{
    ZeroMemory(m_pTiles, FILE_TILE_DIMENSION * FILE_TILE_DIMENSION * sizeof(GridPoint*));
    ZeroMemory(m_lastUsed, FILE_TILE_DIMENSION * FILE_TILE_DIMENSION * sizeof(LONGLONG));

    m_gridFile.open(p_filename, std::ios::out | std::ios::binary | std::ios::app);
    if(!m_gridFile.is_open())
    {
        LI_ERROR("failed to open " + p_filename);
        return false;
    }
    m_gridFile.close();
    m_gridFile.open(p_filename, std::ios::in | std::ios::out | std::ios::binary);
    m_gridFile.read((char*)&m_directory, sizeof(Directory));
    if(m_gridFile.eof())
    {
        m_gridFile.clear();
        ZeroMemory(&m_directory, sizeof(Directory));
        m_directory.version = CURRENT_VERSION;
        m_directory.tileSize = p_fileTileSize;
        this->SaveDirectory();
    }
        
    if(m_directory.version != CURRENT_VERSION)
    {
        LI_ERROR("gridfile version and code version do not match");
        return false;
    }
    if(p_fileTileSize != 0 && m_directory.tileSize != p_fileTileSize)
    {
        LI_WARNING("given file gridsize parameter does not correspond to stored file gridsize");
    }

    m_pMemory.reset(new MemoryPool);
    if(!m_pMemory->Init(TILE_BYTES, MAX_LOADED_TILES, false))
    {
        LI_ERROR("initialization of MemoryPool failed");
    }

    return true;
}


DisplacementData::GridPoint* DisplacementData::GetData(int p_x, int p_y)
{
    if(p_x < 0 || p_y < 0)
    {
        return 0;
    }
    unsigned int tileX = p_x / m_directory.tileSize;
    unsigned int tileY = p_y / m_directory.tileSize;
    p_x %= m_directory.tileSize;
    p_y %= m_directory.tileSize;
    if(tileX < FILE_TILE_DIMENSION && tileY < FILE_TILE_DIMENSION && this->UseTile(tileX, tileY))
    {
        return &m_pTiles[tileY][tileX][p_y * m_directory.tileSize + p_x];
    }
    else
    {
        return 0;
    }
}


float DisplacementData::GetHeight(int p_x, int p_y)
{
    GridPoint* point = this->GetData(p_x, p_y);
    return !point ? 0 : point->height;
}


unsigned short DisplacementData::GetFlags(int p_x, int p_y)
{
    GridPoint* point = this->GetData(p_x, p_y);
    return !point ? 0 : point->flags;
}


void DisplacementData::SetHeight(int p_x, int p_y, float p_height)
{
    GridPoint* point = this->GetData(p_x, p_y);
    if(point)
    {
        point->height = p_height;
    }
}


bool DisplacementData::UseTile(unsigned int p_tileX, unsigned int p_tileY)
{
    if(!m_pTiles[p_tileY][p_tileX])
    {
        while(m_activeTiles >= MAX_LOADED_TILES)
        {
            if(!this->UnloadTile())
            {
                return false;
            }
        }
        m_pTiles[p_tileY][p_tileX] = (GridPoint*)m_pMemory->Alloc();
        ++m_activeTiles;
        if(m_directory.offsets[p_tileY][p_tileX] != 0)
        {
            m_gridFile.seekg(m_directory.offsets[p_tileY][p_tileX]);
            m_gridFile.read((char*)m_pTiles[p_tileY][p_tileX], TILE_BYTES);
        }
        else
        {
            ZeroMemory(m_pTiles[p_tileY][p_tileX], TILE_BYTES);
        }
    }
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    m_lastUsed[p_tileY][p_tileX] = now.QuadPart;
    return true;
}


bool DisplacementData::UnloadTile(void)
{
    unsigned int toFree[2] = { 0, 0 };
    LONGLONG lastused = LONGLONG_MAX;
    for(int y=0; y < FILE_TILE_DIMENSION; ++y)
    {
        for(int x=0; x < FILE_TILE_DIMENSION; ++x)
        {
            if(m_lastUsed[y][x] != 0 && m_lastUsed[y][x] < lastused)
            {
                lastused = m_lastUsed[y][x];
                toFree[0] = x;
                toFree[1] = y;
            }
        }
    }
    if(lastused != LONGLONG_MAX)
    {
        return this->UnloadTile(toFree[0], toFree[1]);
    }
    else
    {
        LI_ERROR("no tile to unload O.o");
        return false;
    }
}


bool DisplacementData::UnloadTile(unsigned int p_tileX, unsigned int p_tileY)
{
    if(m_pTiles[p_tileY][p_tileX])
    {
        if(m_directory.offsets[p_tileY][p_tileX] == 0)
        {
            m_gridFile.seekp(0, std::ios::end);
            m_directory.offsets[p_tileY][p_tileX] = m_gridFile.tellp();
            this->SaveDirectory();
        }
        m_gridFile.seekp(m_directory.offsets[p_tileY][p_tileX]);
        m_gridFile.write((char*)m_pTiles[p_tileY][p_tileX], TILE_BYTES);
        m_lastUsed[p_tileY][p_tileX] = 0;
        m_pMemory->Free(m_pTiles[p_tileY][p_tileX]);
        m_pTiles[p_tileY][p_tileX] = 0;
        --m_activeTiles;
        return true;
    }
    else
    {
        LI_WARNING("trying to unload already unloaded tile");
        return false;
    }
}


void DisplacementData::SaveDirectory(void)
{
    m_gridFile.seekp(0);
    m_gridFile.write((char*)&m_directory, sizeof(Directory));
}


void DisplacementData::GenerateNormals(void)
{
    for(int y=1; y < FILE_TILE_DIMENSION * m_directory.tileSize - 1; ++y)
    {
        for(int x=1; x < FILE_TILE_DIMENSION * m_directory.tileSize - 1; ++x)
        {
            
        }
    }
}


#include "Grid3D.h"
void DisplacementData::GenerateTestData(void)
{
//     std::ifstream file("./Displacements/test.ndm", std::ios::binary | std::ios::in);
//     file.seekg(sizeof(Directory));
//     std::ostringstream str;
//     GridPoint point;
//     while(!file.eof())
//     {
//         file.read((char*)&point, sizeof(GridPoint));
//         str << point.height << " ";
//     }
//     
//     LI_INFO(str.str());

    Grid3D noise;
    noise.Init(8);
    noise.LoadNoise();

    if(!this->Init("./Displacements/test.ndm", 16))
    {
        return;
    }
    std::ostringstream info;
    float alpha = noise.SampleLinear(0.0f, 0.0f, 0.0f);
    for(int y=0; y < 16 * 64; ++y)
    {
        for(int x=0; x < 16 * 64; ++x)
        {
            float vx = (float)x / 1024.0f;
            float vy = (float)y / 1024.0f;
            
            float nx = cos(alpha) * vx - sin(alpha) * vy;
            float ny = sin(alpha) * vx + cos(alpha) * vy;

            float height = 0.0f;
            height += 1.0f * noise.SampleLinear(1.0f * nx, 1.0f * ny, 0);
            height += 0.52f * noise.SampleLinear(2.01f * vx, 2.01f * vy, 0);
            height += 0.261f * noise.SampleLinear(4.07f * nx, 4.07f * ny, 0);
            height += 0.124f * noise.SampleLinear(7.89f * vx, 7.98f * vy, 0);
            height += 0.0637f * noise.SampleLinear(16.17f * nx, 16.17f * ny, 0);        

            this->SetHeight(x, y, 4.0f * height);
        }
    }
}
