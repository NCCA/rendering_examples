#ifndef PERLINNOISETEXTURE_H
#define PERLINNOISETEXTURE_H

#include "noisetexture.h"
#include "noise.h"

template<size_t DIM>
class PerlinNoiseTexture : public NoiseTexture<DIM> {
public:
    /// Ctor
    explicit PerlinNoiseTexture(size_t /*octaves*/ = 6,
                                float /*frequency*/ = 1.0f,
                                float /*persistence*/ = 0.5f,
                                float /*lower*/ = 0.0f,
                                float /*upper*/ = 1.0f,
                                size_t /*resolution*/ = 64);

    /// Dtor
    ~PerlinNoiseTexture() {}

protected:
    /// Generates the data using simplex noise
    inline GLfloat generator_func(const typename NoiseTexture<DIM>::CoordinateArrayf &);

    /// Precompute the inverse resolution for the purposes of coordinate generation
    float m_inv_resf;

    /// Perlin noise module from libnoise
    noise::module::Perlin m_module;

    /// A convenience function for scaling the output from libnoise [-1,1] to our bounds
    float scaleNoise(float /*value*/);
};

/**
 *
 */
template<size_t DIM>
PerlinNoiseTexture<DIM>::PerlinNoiseTexture(size_t _octaves,
                                            float _frequency,
                                            float _persistence,
                                            float _lower,
                                            float _upper,
                                            size_t _resolution)
    : NoiseTexture<DIM>(_lower,_upper,_resolution) {
    // Set the properties of our module for noise generation
    m_module.SetOctaveCount(_octaves);
    m_module.SetFrequency(_frequency);
    m_module.SetPersistence(_persistence);
}

template<>
inline GLfloat PerlinNoiseTexture<2>::generator_func(const typename NoiseTexture<2>::CoordinateArrayf &coordf) {
    return scaleNoise(m_module.GetValue(coordf[0], coordf[1], 0.0f));
}

template<>
inline GLfloat PerlinNoiseTexture<3>::generator_func(const typename NoiseTexture<3>::CoordinateArrayf &coordf) {
    return scaleNoise(m_module.GetValue(coordf[0], coordf[1], coordf[2]));
}

template<size_t DIM>
float PerlinNoiseTexture<DIM>::scaleNoise(float _v) {
    // Note that libnoise returns a value between -1 and 1 so this needs to be corrected
    float f = 0.5f * (1.0f + _v);

    // Scale the result according to our upper and lower bounds
    return (NoiseTexture<DIM>::m_upper - NoiseTexture<DIM>::m_lower) * f + NoiseTexture<DIM>::m_lower;
}

template<size_t DIM>
inline GLfloat PerlinNoiseTexture<DIM>::generator_func(const typename NoiseTexture<DIM>::CoordinateArrayf &coordf) {
    std::cerr << "PerlinNoiseTexture<"<<DIM<<">::generator_func() - no function defined.\n";
    return 0.0f;
}


#endif // PERLINNOISETEXTURE_H
