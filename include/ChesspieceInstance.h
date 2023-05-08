#ifndef CHESSPIECEINSTANCE_H
#define CHESSPIECEINSTANCE_H

#include "Chesspiece.h"
#include "MatrixOperations.h"

/**
 * Describes a piece's position on the XZ plane (Y=0)
 */
struct PiecePosition
{
    float x, z;
};

/**
 * Incorporates a chesspiece object, a texture and a transformation.
 *
 * @author Matti Dahlbom
 * @since 0.1
 */
class ChesspieceInstance
{
public:
    ChesspieceInstance(Chesspiece& object, GLuint texture,
                       const PiecePosition& initialPosition);
    virtual ~ChesspieceInstance();

public:
    Chesspiece& Object() { return m_object; }
    const float* Transform() const { return m_transform; }
    GLuint Texture() const { return m_texture; }
    float* PositionPtr() { return &m_transform[MatrixTranslationOffset]; }
    float Fade() { return m_fade; }
    float* FadePtr() { return &m_fade; }

private: // Data
    Chesspiece& m_object;
    float m_transform[16];
    GLuint m_texture;
    float m_fade;
};

#endif // CHESSPIECEINSTANCE_H
