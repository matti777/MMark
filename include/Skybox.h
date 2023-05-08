#ifndef SKYBOX_H
#define SKYBOX_H

#include "OpenGLAPI.h"

// Default skybox (half)size
static const float SkyboxDefaultSize = 1.0;

/**
 * Represents the sky around the objects. This object must be drawn with
 * rotation only.
 */
class Skybox
{
public:
    virtual ~Skybox();

    /** Constructs this object */
    static Skybox* Create(float size = SkyboxDefaultSize);

public:
    /** Renders this object */
    void Render();

private:
    Skybox();
    bool Setup(float size);

private:
    // vertex/index buffers
    GLuint m_vertexBuffer;
    GLuint m_indexBuffer;
};

#endif // SKYBOX_H
