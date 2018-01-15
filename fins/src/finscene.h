#ifndef FINSCENE_H
#define FINSCENE_H

// The parent class for this scene
#include "scene.h"
#include <ngl/Obj.h>

class FinScene : public Scene {
public:
    /// Construct our scene
    FinScene();

    /// Called when the scene needs to be painted
    void paintGL() noexcept;

    /// Called when the scene is to be initialised
    void initGL() noexcept;

    /// Increase and decrease the fin scaling
    void increaseFins() {m_finScale += m_finScaleIncrement;}
    void decreaseFins() {m_finScale = ((m_finScale-m_finScaleIncrement) < 0.0f)?0.0f:m_finScale - m_finScaleIncrement;}

private:
    /// A unique pointer storing our mesh object
    std::unique_ptr<ngl::Obj> m_mesh;

    /// The scalable value for the fin scaling
    GLfloat m_finScale;

    const GLfloat m_finScaleIncrement = 0.001;
};

#endif // FINSCENE_H
