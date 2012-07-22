#pragma once
class Grid3D
{
private:
    int m_size;
    float* m_pGrid;
    int m_dx, m_dy, m_dz;

public:
    Grid3D(void) : m_pGrid(0), m_dx(0), m_dy(0), m_dz(0), m_size(0) {}
    ~Grid3D(void) { SAFE_DELETE(m_pGrid); }

    void Init(int p_size);
    void LoadNoise(void);
    float SampleLinear(float p_x, float p_y, float p_z) const;
    float SampleNearest(float p_x, float p_y, float p_z) const;
    
    int GetSize(void) const { return m_size; }
    float GetValue(int p_x, int p_y, int p_z) const { return this->GetValue(this->GetSingleIndex(p_x, p_y, p_z)); }
    float GetValue(int p_singleIndex) const { return m_pGrid[p_singleIndex]; }
    void SetValue(int p_x, int p_y, int p_z, float p_value) { this->SetValue(this->GetSingleIndex(p_x, p_y, p_z), p_value); }
    void SetValue(int p_singleIndex, float p_value) { m_pGrid[p_singleIndex] = p_value; }
    int GetSingleIndex(int p_x, int p_y, int p_z) const { return m_size * m_size * MOD(p_z + m_dz, m_size) + m_size * MOD(p_y + m_dy, m_size) + MOD(p_x + m_dx, m_size); }
    bool SingleIndexToCoords(int p_singleIndex, int& p_x, int& p_y, int& p_z) const;
    void ShiftValues(int p_dx, int p_dy, int p_dz) { m_dx -= p_dx; m_dy -= p_dy; m_dz -= p_dz; }

};

