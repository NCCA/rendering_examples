#ifndef DOFSCENE_H
#define DOFSCENE_H

// The parent class for this scene
#include "scene.h"

#include <ngl/Obj.h>

class DofScene : public Scene
{
public:
    /// An enumerated type to allow the user to select an alternative blur function
    typedef enum {BLUR_GAUSSIAN, BLUR_POISSON} BlurFilter;

    /// Construct our scene
    DofScene();

    /// Called when the scene needs to be painted
    void paintGL() noexcept;

    /// Called when the scene is to be initialised
    void initGL() noexcept;

    /// Set the depth for this DOF shader
    void setFocalDepth(const GLfloat &d) {m_focalDepth = d;}

    /// Toggle which blurring filter is active
    void toggleBlurFilter() {m_blurFilter = (m_blurFilter==BLUR_GAUSSIAN)?BLUR_POISSON:BLUR_GAUSSIAN;}

    /// Resize the window (must resize the FBO)
    void resizeGL(GLint /*width*/, GLint /*height*/) noexcept;

private:
    /// Initialise the FBO
    void initFBO();

    /// Keep track of whether the FBO needs to be recreated
    bool m_isFBODirty = true;

    /// The number of objects to draw
    int m_numObjects = 5;

    /// The focal depth
    GLfloat m_focalDepth = 1.0f;

    /// A weighting of the blur radius (bit like inverse focal length)
    GLfloat m_blurRadius = 0.01f;

    /// Id's used for the frame buffer object and associated textures
    GLuint m_fboId, m_fboTextureId, m_fboDepthId;

    /// The default blur filter to use
    BlurFilter m_blurFilter = BLUR_GAUSSIAN;

    /// Set the currently desired shader subroutine on the shader (from m_blurFilter)
    void setShaderSubroutine();
};

#endif // DOFSCENE_H
