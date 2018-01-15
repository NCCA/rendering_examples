#ifndef FRACTALSCENE_H
#define FRACTALSCENE_H

#include <ngl/Obj.h>
#include "scene.h"
#include <chrono>

class FractalScene : public Scene
{
public:
    typedef enum {
        FRACTAL_MANDELBROT,
        FRACTAL_JULIA
    } FractalType;

    typedef enum {
        OBJECT_PLANE,
        OBJECT_TEAPOT
    } ObjectType;

    /// Constructor
    FractalScene();

    /// Called when the scene needs to be painted
    void paintGL() noexcept;

    /// Called when the scene is to be initialised
    void initGL() noexcept;

    /// An interface to allow you to switch which fractal is currently active
    void setCurrentFractal(FractalType f) {m_currentFractal = f;}

    /// Set the render object from the interface
    void setRenderObject(ObjectType o) {m_renderObject = o;}

    /// Toggle whether the scene is currently animating
    void toggleAnimating() {m_animating = !m_animating;}

private:
    /// A type to keep track of which fractal is currently active
    FractalType m_currentFractal = FRACTAL_JULIA;

    /// The currently rendered object
    ObjectType m_renderObject = OBJECT_PLANE;

    /// Set the current subroutine on the shader
    void setShaderSubroutine();

    /// Keep track of the last time
    std::chrono::high_resolution_clock::time_point m_startTime;

    /// Set whether the fractal is animated
    bool m_animating = true;
};

#endif // FRACTALSCENE_H
