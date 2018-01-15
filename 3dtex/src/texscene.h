#ifndef SDFSCENE_H
#define SDFSCENE_H

// The parent class for this scene
#include "scene.h"
#include <chrono>
#include <ngl/Obj.h>

class TexScene : public Scene
{
public:
    /// Constructor
    TexScene();

    /// Called when the scene needs to be painted
    void paintGL() noexcept;

    /// Called when the scene is to be initialised
    void initGL() noexcept;

    /// Set the target and eye position
    void setEye(const glm::vec3& eye) {m_eye = eye;}
    void setTarget(const glm::vec3& target) {m_target = target;}

    void load3DTex();
private:
    /// Keep track of the last time
    std::chrono::high_resolution_clock::time_point m_startTime;

    /// Sets the target and eye position on the shader
    glm::vec3 m_eye, m_target;

    /// A handle for our big 3D texture
    GLuint m_texBlock;

    /// The texture dimensions for our block
    GLint m_texHeight, m_texWidth, m_texDepth;
};

#endif // TEXSCENE_H
