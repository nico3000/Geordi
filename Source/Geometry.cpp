#include "StdAfx.h"
#include "Geometry.h"


Geometry::Geometry(void) :
m_pIndices(0), m_pVertices(0)
{
}


Geometry::~Geometry(void)
{
}


void Geometry::Draw(void)
{
    m_pIndices->Bind();
    m_pVertices->Bind();
    LostIsland::g_pGraphics->GetContext()->DrawIndexed(m_pIndices->GetIndexCount(), 0, 0);
}