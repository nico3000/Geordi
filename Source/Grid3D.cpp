#include "StdAfx.h"
#include "Grid3D.h"
#include "TerrainData.h"


void Grid3D::Init(int p_size)
{
    SAFE_DELETE(m_pGrid);
    m_pGrid = new float[p_size * p_size * p_size];
    m_dx = 0;
    m_dy = 0;
    m_dz = 0;
    m_size = p_size;
}


void Grid3D::LoadNoise(void)
{
    for(int i=0; i < m_size * m_size * m_size; ++i)
    {
        m_pGrid[i] = 2.0f * (float)rand() / (float)RAND_MAX - 1.0f;
    }
}


float Grid3D::SampleLinear(float p_x, float p_y, float p_z, float p_frequency, float p_amplitude) const
{
    return p_amplitude * this->SampleLinear(p_frequency * p_x, p_frequency * p_y, p_frequency * p_z);
}


float Grid3D::SampleLinear(float p_x, float p_y, float p_z) const
{
    float r = p_x - floor(p_x);
    float s = p_y - floor(p_y);
    float t = p_z - floor(p_z);

    float y0 = MIX(this->GetValue((int)floor(p_x),     (int)floor(p_y), (int)floor(p_z)),     this->GetValue((int)floor(p_x),     (int)floor(p_y) + 1, (int)floor(p_z)), s);
    float y1 = MIX(this->GetValue((int)floor(p_x) + 1, (int)floor(p_y), (int)floor(p_z)),     this->GetValue((int)floor(p_x) + 1, (int)floor(p_y) + 1, (int)floor(p_z)), s);
    float y2 = MIX(this->GetValue((int)floor(p_x) + 1, (int)floor(p_y), (int)floor(p_z) + 1), this->GetValue((int)floor(p_x) + 1, (int)floor(p_y) + 1, (int)floor(p_z) + 1), s);
    float y3 = MIX(this->GetValue((int)floor(p_x),     (int)floor(p_y), (int)floor(p_z) + 1), this->GetValue((int)floor(p_x),     (int)floor(p_y) + 1, (int)floor(p_z) + 1), s);

    float x0 = MIX(y0, y1, r);
    float x1 = MIX(y3, y2, r);

    return MIX(x0, x1, t);
}


float Grid3D::SampleNearest(float p_x, float p_y, float p_z) const
{
    float sampleX = (float)m_size * p_x;
    float sampleY = (float)m_size * p_y;
    float sampleZ = (float)m_size * p_z;
    return this->GetValue((int)floor(sampleX + 0.5f), (int)floor(sampleY + 0.5f), (int)floor(sampleZ + 0.5f));
}


void Grid3D::GenerateGradient(float p_x, float p_y, float p_z, XMFLOAT3& p_target) const
{
    const static float epsilon = 1.0f;
    float val = this->SampleLinear(p_x, p_y, p_z);
    p_target.x = this->SampleLinear(p_x + epsilon, p_y, p_z) - this->SampleLinear(p_x - epsilon, p_y, p_z);
    p_target.y = this->SampleLinear(p_x, p_y + epsilon, p_z) - this->SampleLinear(p_x, p_y - epsilon, p_z);
    p_target.z = this->SampleLinear(p_x, p_y, p_z + epsilon) - this->SampleLinear(p_x, p_y, p_z - epsilon);
//     float len = p_target.x * p_target.x + p_target.y * p_target.y + p_target.z * p_target.z;
//     if(len != 0)
//     {
//         len = 1.0f / sqrt(len);
//         p_target.x *= len;
//         p_target.y *= len;
//         p_target.z *= len;
//     }
}


bool Grid3D::SingleIndexToCoords(int p_singleIndex, int& p_x, int& p_y, int& p_z) const
{
    p_x = p_singleIndex % m_size;
    p_singleIndex = (p_singleIndex - p_x) / m_size;
    p_y = p_singleIndex % m_size;
    p_singleIndex = (p_singleIndex - p_y) / m_size;
    p_z = p_singleIndex % m_size;
    return p_singleIndex - p_z == 0;
}



