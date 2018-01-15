#ifndef MORPHSCENE_H
#define MORPHSCENE_H

#include <ngl/Obj.h>
#include "scene.h"
#include "MultiBufferIndexVAO.h"
#include <chrono>

class MorphScene : public Scene
{
public:
    MorphScene();

    /// Called when the scene needs to be painted
    void paintGL() noexcept;

    /// Called when the scene is to be initialised
    void initGL() noexcept;

private:
    /// Create our meshes and store them in the vertex buffer object
    void initMeshes();

    /// Store a unique pointer to the vertex array object to be rendered in our scene
    std::unique_ptr<MultiBufferIndexVAO> m_vao;

    /// Keep track of the last time
    std::chrono::high_resolution_clock::time_point m_startTime;
};

#endif // MORPHSCENE_H
