#pragma once

class Geometry;
class ShaderProgram;
class ConstantBuffer;
class DisplacementData;

class ClipmapTerrain
{
private:
    struct TileProperties
    {
        XMFLOAT3 globalTranslation;
        float scale;
        XMFLOAT3 tileTranslation;
        float level;
        float gridSize;
        XMFLOAT3 unused;
    };

    class ClipmapLevel
    {
    private:
        unsigned int m_level;
        unsigned int m_paramN;
        int m_bottomLeftX;
        int m_bottomLeftY;
        XMFLOAT3 m_globalTranslation;
        ID3D11Texture2D* m_pTexture;
        float* m_pData;

    public:
        ClipmapLevel(void);
        ~ClipmapLevel(void);

        void Init(unsigned int p_paramN, unsigned int p_level, ID3D11Texture2D* p_pTexture);
        void Update(int p_bottomLeftX, int p_bottomLeftY, std::shared_ptr<DisplacementData> p_pData);

        const XMFLOAT3& GetGlobalTranslation(void) const { return m_globalTranslation; }

    };

    bool m_initialized;
    float m_scale;
    int m_currentX, m_currentZ;
    std::shared_ptr<DisplacementData> m_pData;
    std::shared_ptr<Geometry> m_fullBlock;
    std::shared_ptr<Geometry> m_mxmBlock;
    std::shared_ptr<Geometry> m_mx3Block;
    std::shared_ptr<Geometry> m_3xmBlock;
    std::shared_ptr<Geometry> m_pLShape[4];
    std::shared_ptr<Geometry> m_degenerateTriangles;
    std::shared_ptr<Geometry>* m_pLShapeToDraw;
    std::shared_ptr<ShaderProgram> m_pProgram;
    std::shared_ptr<ConstantBuffer> m_pBuffer;
    ClipmapLevel* m_pLevels;
    unsigned int m_levelCount;
    unsigned int m_paramN;
    unsigned int m_paramM;
    XMFLOAT3 m_mxmTranslations[12];
    XMFLOAT3 m_mx3Translations[2];
    XMFLOAT3 m_3xmTranslations[2];
    ID3D11Texture2D* m_pTexture;
    ID3D11ShaderResourceView* m_pSRV;

    void Cleanup(void);

public:
    enum Orientation
    {
        NORTH = 0,
        SOUTH = 2,
        WEST = 1,
        EAST = 0,
        NORTH_EAST = NORTH | EAST,
        NORTH_WEST = NORTH | WEST,
        SOUTH_WEST = SOUTH | WEST,
        SOUTH_EAST = SOUTH | EAST,
    };

    ClipmapTerrain(void);
    ~ClipmapTerrain(void);

    void DrawLevel(unsigned int p_level) const;
    void Draw(void) const;
    void Update(const XMFLOAT3& p_viewPoint);
    bool Init(unsigned int p_gridSizePerLevel, unsigned int p_levelCount);

    bool IsInitialized(void) const { return m_initialized; }

    // p_sizeX = 4 and p_sizeY = 3 leads to:
    // O---O---O---O
    // | / | / | / |
    // O---O---O---O
    // | / | / | / |
    // O---O---O---O
    static std::shared_ptr<Geometry> CreateRectGrid(unsigned char p_sizeX, unsigned char p_sizeY, float p_scale);

    // p_size = 4 leads to:
    // O---O---O---O  O---O---O---O  O---O                  O---O
    // | / | / | / |  | / | / | / |  | / |  SOUTH    SOUTH  | / |
    // O---O---O---O  O---O---O---O  O---O  WEST     EAST   O---O
    //         | / |  | / |          | / |                  | / |
    //  NORTH  O---O  O---O  NORTH   O---O---O---O  O---O---O---O
    //  EAST   | / |  | / |  WEST    | / | / | / |  | / | / | / |
    //         O---O  O---O          O---O---O---O  O---O---O---O
    static std::shared_ptr<Geometry> CreateLShapedGrid(unsigned char p_size, Orientation p_orientation, float p_scale);

    // p_sizeX = 7 and p_sizeY = 3 leads to
    // O---O---O---O---O---O---O
    // |                       |
    // O                       O
    // |                       |
    // O---O---O---O---O---O---O
    static std::shared_ptr<Geometry> CreateDegenerateTriangles(unsigned char p_sizeX, unsigned char p_sizeY, float p_scale);

};

