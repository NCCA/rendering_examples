#ifndef WOODNOISETEXTURE_H
#define WOODNOISETEXTURE_H

#include "noisetexture.h"
#include "noise.h"

template<size_t DIM>
class WoodNoiseTexture : public NoiseTexture<DIM> {
public:
    /// Ctor
    explicit WoodNoiseTexture(float /*lower*/ = 0.0f,
                              float /*upper*/ = 1.0f,
                              size_t /*resolution*/ = 64);

    /// Dtor
    ~WoodNoiseTexture() {}

protected:
    /// Generates the data using simplex noise
    inline GLfloat generator_func(const typename NoiseTexture<DIM>::CoordinateArrayf &);

    /// Precompute the inverse resolution for the purposes of coordinate generation
    float m_inv_resf;

    /// Perlin noise modules - nicked from http://libnoise.sourceforge.net/examples/textures/ texturewood.cpp
    noise::module::Perlin m_woodGrainNoise;
    noise::module::Cylinders m_baseWood;
    noise::module::ScalePoint m_scaledBaseWoodGrain;
    noise::module::ScaleBias m_woodGrain;
    noise::module::Add m_combinedWood;
    noise::module::Turbulence m_perturbedWood;
    noise::module::TranslatePoint m_translatedWood;
    noise::module::RotatePoint m_rotatedWood;
    noise::module::Turbulence m_finalWood;

    /// A convenience function for scaling the output from libnoise [-1,1] to our bounds
    float scaleNoise(float /*value*/);
};

/**
 * See http://libnoise.sourceforge.net/examples/textures/
 */
template<size_t DIM>
WoodNoiseTexture<DIM>::WoodNoiseTexture(float _lower,
                                        float _upper,
                                        size_t _resolution)
    : NoiseTexture<DIM>(_lower,_upper,_resolution) {

    // Base wood texture.  The base texture uses concentric cylinders aligned
    // on the z axis, like a log.
    m_baseWood.SetFrequency (8.0);

    // Perlin noise to use for the wood grain.
    m_woodGrainNoise.SetSeed (0);
    m_woodGrainNoise.SetFrequency (48.0);
    m_woodGrainNoise.SetPersistence (0.5);
    m_woodGrainNoise.SetLacunarity (2.20703125);
    m_woodGrainNoise.SetOctaveCount (3);
    m_woodGrainNoise.SetNoiseQuality(noise::QUALITY_STD);

    // Stretch the Perlin noise in the same direction as the center of the
    // log.  This produces a nice wood-grain texture.
    m_scaledBaseWoodGrain.SetSourceModule (0, m_woodGrainNoise);
    m_scaledBaseWoodGrain.SetYScale (0.25);

    // Scale the wood-grain values so that they may be added to the base wood
    // texture.
    m_woodGrain.SetSourceModule (0, m_scaledBaseWoodGrain);
    m_woodGrain.SetScale (0.25);
    m_woodGrain.SetBias (0.125);

    // Add the wood grain texture to the base wood texture.
    m_combinedWood.SetSourceModule (0, m_baseWood);
    m_combinedWood.SetSourceModule (1, m_woodGrain);

    // Slightly perturb the wood texture for more realism.
    m_perturbedWood.SetSourceModule (0, m_combinedWood);
    m_perturbedWood.SetSeed (1);
    m_perturbedWood.SetFrequency (4.0);
    m_perturbedWood.SetPower (1.0 / 256.0);
    m_perturbedWood.SetRoughness (4);

    // Cut the wood texture a small distance from the center of the "log".
    m_translatedWood.SetSourceModule (0, m_perturbedWood);
    m_translatedWood.SetZTranslation (1.48);

    // Cut the wood texture on an angle to produce a more interesting wood
    // texture.
    m_rotatedWood.SetSourceModule (0, m_translatedWood);
    m_rotatedWood.SetAngles (0.0, 0.0, 0.0);

    // Finally, perturb the wood texture to produce the final texture.
    m_finalWood.SetSourceModule (0, m_rotatedWood);
    m_finalWood.SetSeed (2);
    m_finalWood.SetFrequency (2.0);
    m_finalWood.SetPower (1.0 / 64.0);
    m_finalWood.SetRoughness (4);
}

template<>
inline GLfloat WoodNoiseTexture<2>::generator_func(const typename NoiseTexture<2>::CoordinateArrayf &coordf) {
    return scaleNoise(m_finalWood.GetValue(coordf[0], coordf[1], 0.0f));
}

template<>
inline GLfloat WoodNoiseTexture<3>::generator_func(const typename NoiseTexture<3>::CoordinateArrayf &coordf) {
    return scaleNoise(m_finalWood.GetValue(coordf[0], coordf[1], coordf[2]));
}

template<size_t DIM>
float WoodNoiseTexture<DIM>::scaleNoise(float _v) {
    // Note that libnoise returns a value between -1 and 1 so this needs to be corrected
    float f = 0.5f * (1.0f + _v);

    // Scale the result according to our upper and lower bounds
    return (NoiseTexture<DIM>::m_upper - NoiseTexture<DIM>::m_lower) * f + NoiseTexture<DIM>::m_lower;
}

template<size_t DIM>
inline GLfloat WoodNoiseTexture<DIM>::generator_func(const typename NoiseTexture<DIM>::CoordinateArrayf &coordf) {
    std::cerr << "WoodNoiseTexture<"<<DIM<<">::generator_func() - no function defined.\n";
    return 0.0f;
}


#endif // WOODNOISETEXTURE_H
