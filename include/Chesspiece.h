#ifndef CHESSPIECE_H
#define CHESSPIECE_H

#include "OpenGLAPI.h"

/**
 * Represents a chess piece; the type is selected when creating the instance.
 *
 * @author Matti Dahlbom
 * @since 0.1
 */
class Chesspiece
{
public:
    enum Type {
        Pawn,
        Rook,
        Bishop,
        Knight,
        Queen,
        King
    };

public:
    static Chesspiece* Create(Chesspiece::Type type);
    virtual ~Chesspiece();

public:
    void Render();

private:
    bool Setup(Chesspiece::Type type);
    Chesspiece();

private: // Data
    // Geometry info
    int m_numPolygons;
    GLenum m_indicesDatatype;

    // Vertex/index buffers
    GLuint m_vertexBuffer;
    GLuint m_indexBuffer;
};

#endif // CHESSPIECE_H
