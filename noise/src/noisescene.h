#ifndef NOISESCENE_H
#define NOISESCENE_H

// The parent class for this scene
#include "scene.h"
#include <perlinnoisetexture.h>
#include <ngl/Obj.h>

class NoiseScene : public Scene
{
public:
    typedef enum {NOISE_DATA, NOISE_SHADER} NoiseMethod;
    NoiseScene();

    /// Called when the scene needs to be painted
    void paintGL() noexcept;

    /// Called when the scene is to be initialised
    void initGL() noexcept;

    /// Allow the user to set the currently active shader method
    void setNoiseMethod(NoiseMethod method) {m_noiseMethod = method;}

private:
    /// Keep track of the currently active shader method
    NoiseMethod m_noiseMethod = NOISE_DATA;

    /// Create a 2D noise texture object
    PerlinNoiseTexture<2> m_noiseTex;
};

#endif // NOISESCENE_H
