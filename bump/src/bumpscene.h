#ifndef BUMPSCENE_H
#define BUMPSCENE_H

#include <ngl/Obj.h>
#include "scene.h"

class BumpScene : public Scene
{
public:
    BumpScene();

    /// Called when the scene needs to be painted
    void paintGL() noexcept;

    /// Called when the scene is to be initialised
    void initGL() noexcept;

private:
    GLuint m_colourTex, m_normalTex;

    /// Initialise a texture
    void initTexture(const GLuint& /*texUnit*/, GLuint &/*texId*/, const char */*filename*/);
};

#endif // BUMPSCENE_H
