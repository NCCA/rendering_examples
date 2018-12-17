#ifndef SHADOWSCENE_H
#define SHADOWSCENE_H

// The parent class for this scene
#include "scene.h"

#include <ngl/Obj.h>

class ShadowScene : public Scene
{
public:

    /// Construct our scene
    ShadowScene();

    /// Called when the scene needs to be painted
    void paintGL() noexcept;

    /// Called when the scene is to be initialised
    void initGL() noexcept;

    /// Resize the window (must resize the FBO)
    void resizeGL(GLint /*width*/, GLint /*height*/) noexcept;

private:
    /// Draw some teapots given the particular View and Projection matrices
    void drawScene(const GLuint &/*pid*/,
                   const glm::mat4 &/*V*/,
                   const glm::mat4 &/*P*/,
                   const glm::mat4 &/*depthVP*/);

    /// Initialise the FBO
    void initFBO();

    /// Keep track of whether the FBO needs to be recreated
    bool m_isFBODirty = true;

    /// The number of objects to draw
    int m_numObjects = 5;

    /// A weighting of the blur radius (bit like inverse focal length)
    GLfloat m_blurRadius = 0.01f;

    /// Id's used for the frame buffer object and associated textures
    GLuint m_fboId, m_fboDepthId;

    /// The shadow resolution
    GLuint m_shadowRes = 1024;

    /// Keep track of the light position
    glm::vec3 m_lightPos;
};

#endif // SHADOWSCENE_H
