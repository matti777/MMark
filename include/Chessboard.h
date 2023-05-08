#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include "OpenGLAPI.h"

/**
 * Represents the chessboard.
 *
 * @author Matti Dahlbom
 * @since 0.1
 */
class Chessboard
{
public:
    static Chessboard* Create();
    ~Chessboard();

public:
    void Render();

private:
    Chessboard();
    bool Setup();

private: // Data
    // Vertex/index buffers
    GLuint m_vertexBuffer;
};

#endif // CHESSBOARD_H
