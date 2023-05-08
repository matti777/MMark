#include "Chessboard.h"
#include "GLController.h"
#include "CommonFunctions.h"

// Half the board's width - top of board is 2*size x 2*size
static const float HalfWidth = 5.0;

const VertexAttribs BoardVertices[] = {
    // side 0 ("top")
    {-HalfWidth, 0.0, -HalfWidth,  0.0, 1.0,        0.0, 1.0, 0.0},
    {-HalfWidth, 0.0, HalfWidth,   0.0, 0.0,        0.0, 1.0, 0.0},
    {HalfWidth, 0.0, -HalfWidth,   1.0, 1.0,        0.0, 1.0, 0.0},
    {HalfWidth, 0.0, -HalfWidth,   1.0, 1.0,        0.0, 1.0, 0.0},
    {-HalfWidth, 0.0, HalfWidth,   0.0, 0.0,        0.0, 1.0, 0.0},
    {HalfWidth, 0.0, HalfWidth,    1.0, 0.0,        0.0, 1.0, 0.0},
};

const int NumVertices = sizeof(BoardVertices) / sizeof(VertexAttribs);

bool Chessboard::Setup()
{
    // Create vertex/index buffers
    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

    // Upload geometry
    glBufferData(GL_ARRAY_BUFFER, sizeof(BoardVertices),
                 BoardVertices, GL_STATIC_DRAW);

    return (glGetError() == GL_NO_ERROR);
}

Chessboard* Chessboard::Create()
{
    Chessboard* chessboard = new Chessboard();
    if ( !chessboard->Setup() )
    {
        delete chessboard;
        return NULL;
    }
    else
    {
        return chessboard;
    }
}

Chessboard::~Chessboard()
{
    // Release all OpenGL resources
    glDeleteBuffers(1, &m_vertexBuffer);
}

Chessboard::Chessboard()
    : m_vertexBuffer(0)
{
}

void Chessboard::Render()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    SetVertexAttribsPointers();
//    glVertexAttribPointer(COORD_INDEX, 3, GL_FLOAT, GL_FALSE,
//                          sizeof(VertexAttribs),
//                          (const GLvoid*)offsetof(VertexAttribs, x));
//    glVertexAttribPointer(TEXCOORD_INDEX, 2, GL_FLOAT, GL_FALSE,
//                          sizeof(VertexAttribs),
//                          (const GLvoid*)offsetof(VertexAttribs, u));
//    glVertexAttribPointer(NORMAL_INDEX, 3, GL_FLOAT, GL_FALSE,
//                          sizeof(VertexAttribs),
//                          (const GLvoid*)offsetof(VertexAttribs, nx));

    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

