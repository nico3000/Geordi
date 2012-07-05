#pragma once
#include "Geometry.h"
#include "ShaderProgram.h"

class ScreenQuad
{
private:
    static std::shared_ptr<ScreenQuad> sm_pInstance;

    StrongGeometryPtr m_pGeometry;

    ScreenQuad(void) : m_pGeometry(new Geometry) {  }

public:
    ~ScreenQuad(void);

    void Draw(void);

    static bool CreateInputLayoutForShader(ShaderProgram& p_shader);
    static std::weak_ptr<ScreenQuad> GetScreenQuad(void);

};

