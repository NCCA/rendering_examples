#ifndef SHADERSCENE_H
#define SHADERSCENE_H

// The parent class for this scene
#include "scene.h"

#include <ngl/Obj.h>

class ShaderScene : public Scene
{
public:
    /// This enumerated type allows the user to flip between the shader methods
    typedef enum {
        SHADER_GOURAUD,
        SHADER_PHONG,
        SHADER_COOKTORRANCE,
        SHADER_TOON
    } ShaderMethod;

    ShaderScene();

    /// Called when the scene needs to be painted
    void paintGL() noexcept;

    /// Called when the scene is to be initialised
    void initGL() noexcept;

    /// Allow the user to set the currently active shader method
    void setShaderMethod(ShaderMethod method) {m_shaderMethod = method;}

private:
    /// Keep track of the currently active shader method
    ShaderMethod m_shaderMethod = SHADER_GOURAUD;
};

#endif // SHADERSCENE_H
