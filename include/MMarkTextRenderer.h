#ifndef MMARKTEXTRENDERER_H
#define MMARKTEXTRENDERER_H

#include "TextRenderer.h"

/**
 * MMark project text renderer.
 *
 * @author Matti Dahlbom
 * @since 0.1
 */
class MMarkTextRenderer : public TextRenderer
{
public: // Constructors and destructor
    MMarkTextRenderer(GLuint program, GLuint mvpLoc, GLuint textureLoc,
                      GLuint indexBuffer);
    virtual ~MMarkTextRenderer();
    bool Setup();
};

#endif // MMARKTEXTRENDERER_H
