#include "Skybox.h"
#include "GLController.h"
#include "CommonFunctions.h"

Skybox::Skybox()
    : m_vertexBuffer(0),
      m_indexBuffer(0)
{
}

Skybox::~Skybox()
{
    // Release all OpenGL resources
    glDeleteBuffers(1, &m_vertexBuffer);
    glDeleteBuffers(1, &m_indexBuffer);
}

GLushort SkyboxIndices[] = {
    4, 0, 7, // top
    7, 0, 3,
    3, 2, 7, // right
    7, 2, 6,
    4, 5, 0, // left
    0, 5, 1,
    7, 6, 4, // back
    4, 6, 5,
    0, 1, 3, // front
    3, 1, 2,
    2, 1, 6, // bottom
    6, 1, 5
};

static const int NumIndices = sizeof(SkyboxIndices) / sizeof(GLushort);

bool Skybox::Setup(float size)
{
    // Create vertex/index buffers
    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

    glGenBuffers(1, &m_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);

    const VertexAttribsCoordsOnly SkyboxVertices[] = {
        {-size,  size,  size},
        {-size, -size,  size},
        {size, -size,  size},
        {size,  size,  size},
        {-size,  size, -size},
        {-size, -size, -size},
        {size, -size, -size},
        {size,  size, -size},
    };
    
    // Upload geometry
    glBufferData(GL_ARRAY_BUFFER, sizeof(SkyboxVertices),
                 SkyboxVertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(SkyboxIndices),
                 SkyboxIndices, GL_STATIC_DRAW);

    return (glGetError() == GL_NO_ERROR);
}

Skybox* Skybox::Create(float size)
{
    Skybox* me = new Skybox();
    if ( !me->Setup(size) )
    {
        LOG_DEBUG("Skybox::Create(): Setup() failed");
        delete me;
        return NULL;
    }

    return me;
}

void Skybox::Render()
{
    // Disable culling; always "facing" the inside of the box
    glDisable(GL_CULL_FACE);

    // Only using coords; disable others
    glDisableVertexAttribArray(TEXCOORD_INDEX);
    glDisableVertexAttribArray(NORMAL_INDEX);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);

    glVertexAttribPointer(COORD_INDEX, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribsCoordsOnly),
                          (const GLvoid*)offsetof(VertexAttribsCoordsOnly, x));

    glDrawElements(GL_TRIANGLES, NumIndices, GL_UNSIGNED_SHORT, 0);

    // Re-enable arrays
    glEnableVertexAttribArray(TEXCOORD_INDEX);
    glEnableVertexAttribArray(NORMAL_INDEX);

    // Restore culling
    glEnable(GL_CULL_FACE);
}
