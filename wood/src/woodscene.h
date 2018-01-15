#ifndef WOODSCENE_H
#define WOODSCENE_H

#include <ngl/Obj.h>
#include "scene.h"

#include "woodnoisetexture.h"

class WoodScene : public Scene
{
public:
    WoodScene();

    /// Called when the scene needs to be painted
    void paintGL() noexcept;

    /// Called when the scene is to be initialised
    void initGL() noexcept;

private:
    /// A texture storing blocks of 3D noise
    WoodNoiseTexture<3> m_diffuseTex;

};

#endif // WOODSCENE_H
