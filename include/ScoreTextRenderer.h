#ifndef SCORETEXTRENDERER_H
#define SCORETEXTRENDERER_H

#include "TextRenderer.h"

/**
 * MMark score text renderer.
 *
 * @author Matti Dahlbom
 * @since 0.1
 */
class ScoreTextRenderer : public TextRenderer
{
public: // Constructors and destructor
    ScoreTextRenderer(GLuint program, GLuint mvpLoc, GLuint textureLoc,
                      GLuint indexBuffer);
    virtual ~ScoreTextRenderer();
    bool Setup();
};

#endif // SCORETEXTRENDERER_H
