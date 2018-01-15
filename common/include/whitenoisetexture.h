/*
 * Copyright (c) 2016 Richard Southern
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 * IN THE SOFTWARE.
 */

#ifndef WHITENOISETEXTURE_H
#define WHITENOISETEXTURE_H

#include "noisetexture.h"

#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

#ifdef BOOST_NO_STDC_NAMESPACE
namespace std {
  using ::time;
}
#endif

/**
 * @brief The WhiteNoiseTexture class
 */
template<size_t DIM>
class WhiteNoiseTexture : public NoiseTexture<DIM>
{
public:
    /// Try boost::mt19937 or boost::ecuyer1988 instead of boost::minstd_rand
    typedef boost::minstd_rand base_generator_type;

    /// Constructor
    explicit WhiteNoiseTexture(float /*lower*/ = 0.0f,
                               float /*upper*/ = 1.0f,
                               size_t /*resolution*/ = 64);

    /// Dtor
    ~WhiteNoiseTexture() {}

protected:
    /// Specialisation of this class to generate pure white noise
    inline GLfloat generator_func(const typename NoiseTexture<DIM>::CoordinateArrayf &);

    /// Boost seed function
    base_generator_type m_seed;

    /// Boost distribution
    boost::uniform_real<float> m_dist;

    /// This horrible line defines a generator for creating random data within the specified range
    boost::variate_generator<base_generator_type&, boost::uniform_real<float> > mf_generator;
};

/**
 *
 */
template<size_t DIM>
WhiteNoiseTexture<DIM>::WhiteNoiseTexture(float _lower,
                                          float _upper,
                                          size_t _resolution)
    : NoiseTexture<DIM>(_lower,_upper,_resolution),
      m_seed(42u), // Picking an arbitrary seed here
      m_dist(_lower,_upper), // Maximum and minimum values of our distribution
      mf_generator(m_seed, m_dist) // Initialise our generator with seed and distribution values
{
}

/**
 *
 */
template<size_t DIM>
inline GLfloat WhiteNoiseTexture<DIM>::generator_func(const typename NoiseTexture<DIM>::CoordinateArrayf &coord) {
    return mf_generator();
}

#endif // WHITENOISETEXTURE_H
