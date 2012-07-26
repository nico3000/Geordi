#pragma once

class Geometry;
class ShaderProgram;

class ClipmapTerrain
{
private:
    bool m_initialized;
    std::shared_ptr<Geometry> m_mxmBlock;
    std::shared_ptr<Geometry> m_mx3Block;
    std::shared_ptr<Geometry> m_pLShape[4];
    std::shared_ptr<ShaderProgram> m_pProgram;

public:
    enum Orientation
    {
        NORTH_EAST = 0,
        NORTH_WEST = 1,
        SOUTH_WEST = 2,
        SOUTH_EAST = 3,
    };

    ClipmapTerrain(void);
    ~ClipmapTerrain(void);

    void Draw(void);

    bool Init(unsigned int p_gridSizePerLevel, unsigned int p_levels);

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
};

