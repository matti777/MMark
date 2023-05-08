#include "ChesspieceInstance.h"
#include "MatrixOperations.h"
#include "CommonFunctions.h"

ChesspieceInstance::ChesspieceInstance(Chesspiece& object, GLuint texture,
                                       const PiecePosition& initialPosition)
    : m_object(object),
      m_texture(texture),
      m_fade(1.0)
{
    MatrixCreateTranslation(m_transform,
                            initialPosition.x, 0.0, initialPosition.z);
}

ChesspieceInstance::~ChesspieceInstance()
{
}

