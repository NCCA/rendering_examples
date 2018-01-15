#ifndef SHADERSCENE_H
#define SHADERSCENE_H

// The parent class for this scene
#include "scene.h"

#include <ngl/Obj.h>

class ShaderScene : public Scene
{
public:
    /// Constructor
    ShaderScene();

    /// Called when the scene needs to be painted
    void paintGL() noexcept;

    /// Called when the scene is to be initialised
    void initGL() noexcept;

private:
    /// Handles for our texture maps
    GLuint m_colourTex, m_normalTex, m_specularTex, m_glossTex, m_envTex;

    /// Initialise a texture with the given texUnit
    void initTexture(const GLint &texUnit, GLuint &texId, const char *filename);

    /// Initialise our Environment Texture Maps
    void initEnvironment(const GLint& texUnit);

    /// Initialise a single side of our environment map
    void initEnvironmentSide(GLenum target, const char *filename);
};

#endif // SHADERSCENE_H
