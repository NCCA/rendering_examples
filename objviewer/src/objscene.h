#ifndef SHADERSCENE_H
#define SHADERSCENE_H

// The parent class for this scene
#include "scene.h"
#include <ngl/Obj.h>
#include <memory>

class ObjScene : public Scene
{
public:
    /// Construct something
    ObjScene();

    /// Called when the scene needs to be painted
    void paintGL() noexcept;

    /// Called when the scene is to be initialised
    void initGL() noexcept;

private:
    /// A unique pointer storing our mesh object
    std::unique_ptr<ngl::Obj> m_mesh;
};

#endif // SHADERSCENE_H
