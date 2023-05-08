#ifndef PILLAR_H
#define PILLAR_H

#include <btBulletDynamicsCommon.h>

#include "OpenGLAPI.h"

/**
 * Represents a pillar, an object with physics interaction.
 *
 * @author Matti Dahlbom
 * @since 0.1
 */
class Pillar
{
public: // Construction and deconstruction
    static Pillar* Create();
   ~Pillar();

public: // Public API
    /** Binds the VBOs and sets the attrib pointers. */
    void PrepareRender();
    void Render();

    /** Returns the extents for the Bullet collision shape. */
    static btVector3 GetExtents();

private:
    Pillar();
    bool Setup();

private: // Data
    // Vertex/index buffers
    GLuint m_vertexBuffer;
    GLuint m_indexBuffer;
};

#endif // PILLAR_H
