#include "Pillar.h"
#include "Pillar_data.h"
#include "CommonFunctions.h"

Pillar::Pillar()
    : m_vertexBuffer(0),
      m_indexBuffer(0)
{
}

Pillar::~Pillar()
{
    glDeleteBuffers(1, &m_vertexBuffer);
    glDeleteBuffers(1, &m_indexBuffer);
}

btVector3 Pillar::GetExtents()
{
    // Return the outer extents of the pillar
    return btVector3(PillarHalfWidth,
                     PillarHalfHeight + 0.10, // Account for raised walkway
                     PillarHalfDepth);
}

bool Pillar::Setup()
{
    // Create vertex/index buffers
    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

    glGenBuffers(1, &m_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);

    // Upload geometry
    glBufferData(GL_ARRAY_BUFFER, sizeof(Pillar_vertices),
                 Pillar_vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Pillar_indices),
                    Pillar_indices, GL_STATIC_DRAW);

    return (glGetError() == GL_NO_ERROR);
}

Pillar* Pillar::Create()
{
    Pillar* pillar = new Pillar();
    if ( !pillar->Setup() )
    {
        delete pillar;
        LOG_DEBUG("Pillar::Setup() failed");
        return NULL;
    }
    else
    {
        return pillar;
    }
}

void Pillar::PrepareRender()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    SetVertexAttribsPointers();
}

void Pillar::Render()
{
    glDrawElements(GL_TRIANGLES, PillarNumPolygons * 3,
                   PillarIndicesDatatype, NULL);
}

