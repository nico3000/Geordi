#pragma once
#define FILE_TILE_DIMENSION 64

class MemoryPool;

class DisplacementData
{
private:
    struct Directory
    {
        unsigned short version;
        unsigned short tileSize;
        unsigned int offsets[FILE_TILE_DIMENSION][FILE_TILE_DIMENSION];
    };
    struct GridPoint
    {
        unsigned short flags;
        float height;
        XMFLOAT2 normalXZ;
    };

    std::fstream m_gridFile;
    Directory m_directory;
    GridPoint* m_pTiles[FILE_TILE_DIMENSION][FILE_TILE_DIMENSION];
    LONGLONG m_lastUsed[FILE_TILE_DIMENSION][FILE_TILE_DIMENSION];
    std::shared_ptr<MemoryPool> m_pMemory;
    unsigned int m_activeTiles;

    bool UseTile(unsigned int p_tileX, unsigned int p_tileY);
    bool UnloadTile(unsigned int p_tileX, unsigned int p_tileY);
    bool UnloadTile(void);
    GridPoint* GetData(int p_x, int p_y);
    void SaveDirectory(void);

public:
    DisplacementData(void);
    ~DisplacementData(void);

    bool Init(std::string p_filename, unsigned int p_fileTileSize);
    unsigned short GetFlags(int p_x, int p_y);
    float GetHeight(int p_x, int p_y);
    void SetHeight(int p_x, int p_y, float p_height);
    void GetNormalXZ(void);
    void GenerateNormals(void);

    void GenerateTestData(void);

};

