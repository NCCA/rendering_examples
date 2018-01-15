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

#ifndef NOISETEXTURE_H
#define NOISETEXTURE_H

#include <GL/gl.h>
#include <iostream>
#include <array>

/**
 * @brief The NoiseTexture class
 */
template <size_t DIM>
class NoiseTexture
{
public:
    /// A static (i.e. not dynamic allocated) vector
    typedef std::array<size_t,DIM> CoordinateArray;
    typedef std::array<float, DIM> CoordinateArrayf;

    /// Constructor
    explicit NoiseTexture(float /*lower*/ = 0.0f,
                          float /*upper*/ = 1.0f,
                          size_t /*resolution*/ = 64);

    /// dtor - must remove memory associated with bound texture
    virtual ~NoiseTexture();

    /// Create the texture. This must be called within a GL context. You might want to do this in parallel.
    virtual void generate();

    /// Bind this texture to the current rendering context.
    void bind() const;

    /// Return the id of this texture
    GLuint getID() const {return m_texID;}

    /// Delete this texture off the GPU
    void destroy();

    void dumpTexture();

protected:
    /// Function to copy the raw data onto the GPU as texture
    void copyTextureDataToGPU(GLfloat */*data*/);

    /// Recursively generate the data
    virtual void generate_recurse(const size_t &/*dim*/, const CoordinateArray &/*coord*/, GLfloat */*data*/);

    /// A generator function to make the process of building noise easy
    virtual inline GLfloat generator_func(const CoordinateArrayf &) = 0;

    /// Keep track of whether the texture has been initialised
    bool m_isInit;

    /// The OpenGL texture ID associated with the noise (memory address on GPU)
    GLuint m_texID;

    /// The current resolution of our block of texture (dimensions the same at present)
    size_t m_res;

    /// Lower and upper bound of the noise values
    float m_lower;
    float m_upper;

    /// Evaluate the target at compile time based on the input template parameter
    constexpr GLuint target() const {
        switch(DIM) {
        case 1:
            return GL_TEXTURE_1D;
        case 2:
            return GL_TEXTURE_2D;
        case 3:
            return GL_TEXTURE_3D;
        }
        return 0;
    }
    const GLuint m_target = target();

private:
    /// Precompute the inverse of the resolution
    float m_inv_resf;
};

/**
 * @brief NoiseTexture::NoiseTexture
 */
template <size_t DIM>
NoiseTexture<DIM>::NoiseTexture(float _lower,
                                float _upper,
                                size_t _resolution)
    : m_isInit(false),
      m_res(_resolution),
      m_lower(_lower),
      m_upper(_upper)
{
    m_inv_resf = 1.0f / float(m_res-1);
}

template <size_t DIM>
NoiseTexture<DIM>::~NoiseTexture() {
}

/**
 * @brief NoiseTexture::destroy
 */
template <size_t DIM>
void NoiseTexture<DIM>::destroy() {
    if (m_isInit) {
        glDeleteTextures(1, &m_texID);
        m_isInit = false;
    }
}


/**
 * Note: I could have implemented this with template specialisations. It would have resulted in
 * more code though.
 */
template <size_t DIM>
void NoiseTexture<DIM>::bind() const {
    if (m_isInit) {
        glBindTexture(m_target, m_texID);
    }
}


/**
 * @brief NoiseTexture::copyTextureToGPU
 * @param data
 * Function for internal use that copies the texture data to the GPU.
 */
template <size_t DIM>
void NoiseTexture<DIM>::copyTextureDataToGPU(GLfloat *data) {
    // Transfer this data to our texture
    glGenTextures(1, &m_texID);
    glBindTexture(m_target, m_texID);

    // Repeat the texture for coordinates <0 or >1
    glTexParameteri(m_target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(m_target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(m_target, GL_TEXTURE_WRAP_R, GL_REPEAT);

    // Use blending when texels are bigger or smaller than pixels
    glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Upload the texture data to the GPU
    switch(DIM) {
    case 1:
        glTexImage1D(m_target,      // Target
                     0,             // Level
                     GL_RGB,        // Internal Format
                     m_res,         // width
                     0,             // border
                     GL_RGB,        // format
                     GL_FLOAT,      // type
                     data);
        break;
    case 2:
        glTexImage2D(m_target,      // Target
                     0,             // Level
                     GL_RGB,        // Internal Format
                     m_res,         // width
                     m_res,         // height
                     0,             // border
                     GL_RGB,        // format
                     GL_FLOAT,      // type
                     data);
        break;
    case 3:
        glTexImage3D(m_target,      // Target
                     0,             // Layer
                     GL_RGB,        // Input format
                     m_res,         // Width
                     m_res,         // Height
                     m_res,         // Depth
                     0,             // border
                     GL_RGB,        // Storage format
                     GL_FLOAT,      // Storage type
                     data);         // Actual data
        break;
    }
}

/**
 * @brief NoiseTexture<DIM>::generate_recurse Generates a block of noise texture using
 *
 * @param d The current dimension you're looping over (starts at DIM and ticks down to 0)
 * @param coord The array used to build the current index of your sample (dimension DIM)
 * @param data The data which we are writing to.
 */
template <size_t DIM>
void NoiseTexture<DIM>::generate_recurse(const size_t &d,
                                         const CoordinateArray& coord,
                                         GLfloat *data) {
    size_t dim_length = 1, data_pos = 0, i;
    CoordinateArrayf coordf, tmp;
    CoordinateArray ncoord;
    switch(d) {
    case 0:
        // At the end of the recursion chain we can evaluate the noise function
        for (i=0; i<DIM; ++i) {
            data_pos += coord[i] * dim_length;
            dim_length *= m_res;
            coordf[i] = m_inv_resf * float(coord[i]);
        }
        data_pos *= 3;

        // Fill up the data with data defined by the generator_func()
        for (i=0; i<3; ++i) {
            tmp = coordf; coordf[i] += 1.0f;
            data[data_pos + i] = generator_func(coordf);
        }
        break;
    default:
        // For the rest of the dimensions we recursively call all the remaining coordinates
        ncoord = coord;
        for (i=0; i<m_res; ++i) {
            ncoord[d-1] = i;
            generate_recurse(d-1, ncoord, data);
        }
    }
}

/**
 * @brief SimplexNoiseTexture::generate
 * Set up the noise texture using the parameters specified. Assume the appropriate
 * texture unit has been activated.
 */
template <size_t DIM>
void NoiseTexture<DIM>::generate() {
    if (m_isInit) return;

    // Allocate a slab of data for the stuffing
    GLfloat *data = (GLfloat*) malloc(sizeof(GLfloat) * pow(m_res,DIM) * 3);

    // Use the recursive function to generate the data recursively
    CoordinateArray coord;
    generate_recurse(DIM, coord, data);

    // Copy our data over to the GPU
    copyTextureDataToGPU(data);

    // Delete our data - it's been copied onto the GPU right?
    free(data);

    m_isInit = true;
}

#endif // NOISETEXTURE_H
