#include "GLController.h"
#include "Chesspiece.h"
#include "Chesspawn_data.h"
#include "Chessrook_data.h"
#include "Chessbishop_data.h"
#include "Chessknight_data.h"
#include "Chessqueen_data.h"
#include "Chessking_data.h"
#include "CommonFunctions.h"

Chesspiece::Chesspiece()
    : m_numPolygons(0),
      m_indicesDatatype(0),
      m_vertexBuffer(0),
      m_indexBuffer(0)
{
}

Chesspiece::~Chesspiece()
{
    // Release all OpenGL resources
    glDeleteBuffers(1, &m_vertexBuffer);
    glDeleteBuffers(1, &m_indexBuffer);
}

void Chesspiece::Render()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glVertexAttribPointer(COORD_INDEX, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribs),
                          (const GLvoid*)offsetof(VertexAttribs, x));
    glVertexAttribPointer(TEXCOORD_INDEX, 2, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribs),
                          (const GLvoid*)offsetof(VertexAttribs, u));
    glVertexAttribPointer(NORMAL_INDEX, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribs),
                          (const GLvoid*)offsetof(VertexAttribs, nx));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    glDrawElements(GL_TRIANGLES, m_numPolygons * 3, m_indicesDatatype, NULL);
}

bool Chesspiece::Setup(Chesspiece::Type type)
{
    // Create vertex/index buffers
    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

    glGenBuffers(1, &m_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);

    size_t arrayDataSize = 0;
    const void* arrayData = NULL;
    size_t indexDataSize = 0;
    const void* indexData = NULL;

    switch ( type )
    {
    case Pawn:
        arrayDataSize = sizeof(Chesspawn_vertices);
        arrayData = Chesspawn_vertices;
        indexDataSize = sizeof(Chesspawn_indices);
        indexData = Chesspawn_indices;
        m_indicesDatatype = ChesspawnIndicesDatatype;
        m_numPolygons = ChesspawnNumPolygons;
        break;
    case Rook:
        arrayDataSize = sizeof(Chessrook_vertices);
        arrayData = Chessrook_vertices;
        indexDataSize = sizeof(Chessrook_indices);
        indexData = Chessrook_indices;
        m_indicesDatatype = ChessrookIndicesDatatype;
        m_numPolygons = ChessrookNumPolygons;
        break;
    case Bishop:
        arrayDataSize = sizeof(Chessbishop_vertices);
        arrayData = Chessbishop_vertices;
        indexDataSize = sizeof(Chessbishop_indices);
        indexData = Chessbishop_indices;
        m_indicesDatatype = ChessbishopIndicesDatatype;
        m_numPolygons = ChessbishopNumPolygons;
        break;
    case Knight:
        arrayDataSize = sizeof(Chessknight_vertices);
        arrayData = Chessknight_vertices;
        indexDataSize = sizeof(Chessknight_indices);
        indexData = Chessknight_indices;
        m_indicesDatatype = ChessknightIndicesDatatype;
        m_numPolygons = ChessknightNumPolygons;
        break;
    case Queen:
        arrayDataSize = sizeof(Chessqueen_vertices);
        arrayData = Chessqueen_vertices;
        indexDataSize = sizeof(Chessqueen_indices);
        indexData = Chessqueen_indices;
        m_indicesDatatype = ChessqueenIndicesDatatype;
        m_numPolygons = ChessqueenNumPolygons;
        break;
    case King:
        arrayDataSize = sizeof(Chessking_vertices);
        arrayData = Chessking_vertices;
        indexDataSize = sizeof(Chessking_indices);
        indexData = Chessking_indices;
        m_indicesDatatype = ChesskingIndicesDatatype;
        m_numPolygons = ChesskingNumPolygons;
        break;
    default:
        LOG_DEBUG("Unsupported Chesspiece type: %d", type);
        return false;
        break;
    }

    // Upload geometry
    glBufferData(GL_ARRAY_BUFFER, arrayDataSize, arrayData, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSize,
                    indexData, GL_STATIC_DRAW);

    return (glGetError() == GL_NO_ERROR);
}

Chesspiece* Chesspiece::Create(Chesspiece::Type type)
{
    Chesspiece* piece = new Chesspiece();
    if ( !piece->Setup(type) )
    {
        LOG_DEBUG("Chesspiece::Create(): Setup() failed for type: %d", type);
        delete piece;
        return NULL;
    }

    return piece;
}

