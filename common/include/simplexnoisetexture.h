#ifndef SIMPLEXNOISETEXTURE_H
#define SIMPLEXNOISETEXTURE_H

#include "noisetexture.h"
#include "simplexnoise.h"
#include <array>

/**
 * @brief The SimplexNoiseTexture class
 */
template <size_t DIM>
class SimplexNoiseTexture : public NoiseTexture<DIM>
{
public:
    /// Ctor
    explicit SimplexNoiseTexture(float /*octaves*/ = 4.0f,
                                 float /*persistence*/ = 0.5f,
                                 float /*scale*/ = 1.0f,
                                 float /*lower*/ = 0.0f,
                                 float /*upper*/ = 1.0f,
                                 size_t /*resolution*/ = 64);

    /// Dtor
    ~SimplexNoiseTexture() {}

protected:
    /// Parameters required for simplex noise generation
    float m_octaves;
    float m_persistence;
    float m_scale;

    /// Generates the data using simplex noise
    inline GLfloat generator_func(const typename NoiseTexture<DIM>::CoordinateArrayf &);

    /// Precompute the inverse resolution for the purposes of coordinate generation
    float m_inv_resf;
};

/**
 *
 */
template<size_t DIM>
SimplexNoiseTexture<DIM>::SimplexNoiseTexture(float _octaves,
                                         float _persistence,
                                         float _scale,
                                         float _lower,
                                         float _upper,
                                         size_t _resolution)
    : NoiseTexture<DIM>(_lower,_upper,_resolution),
      m_octaves(_octaves),
      m_persistence(_persistence),
      m_scale(_scale)
{
}

//template<>
//GLfloat SimplexNoiseTexture<1>::generator_func(const typename NoiseTexture<1>::CoordinateArrayf &coordf) {
//    return scaled_octave_noise_1d(m_octaves,
//                                  m_persistence,
//                                  m_scale,
//                                  m_lower,
//                                  m_upper,
//                                  coordf[0]);
//}

template<>
inline GLfloat SimplexNoiseTexture<2>::generator_func(const typename NoiseTexture<2>::CoordinateArrayf &coordf) {
    return scaled_octave_noise_2d(m_octaves,
                                  m_persistence,
                                  m_scale,
                                  m_lower,
                                  m_upper,
                                  coordf[0],
                                  coordf[1]);
}

template<>
inline GLfloat SimplexNoiseTexture<3>::generator_func(const typename NoiseTexture<3>::CoordinateArrayf &coordf) {
    return scaled_octave_noise_3d(m_octaves,
                                  m_persistence,
                                  m_scale,
                                  m_lower,
                                  m_upper,
                                  coordf[0],
                                  coordf[1],
                                  coordf[2]);
}

template<size_t DIM>
inline GLfloat SimplexNoiseTexture<DIM>::generator_func(const typename NoiseTexture<DIM>::CoordinateArrayf &coordf) {
    std::cerr << "SimplexNoiseTexture<"<<DIM<<">::generator_func() - no function defined.\n";
    return 0.0f;
}

#endif // SIMPLEXNOISETEXTURE_H
