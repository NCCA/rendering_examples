#ifndef SCENE_H
#define SCENE_H

// Includes the GL headers in platform independent and order specific way
#include "glinclude.h"

// Standard includes for console output etc.
// #include <stdlib.h>
#include <iostream>

// Includes for GLM
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Needed to read the file string (for shaders)
#include <string>
#include <fstream>
#include <streambuf>

/**
 * @brief The Scene class
 */
class Scene
{
public:
    /// Constructor - don't call OpenGL stuff in this
    Scene();

    /// Called when the scene needs to be painted
    virtual void paintGL() noexcept = 0;

    /// Called when the scene is to be initialised
    virtual void initGL() noexcept = 0;

    /// Called when the window has been resized
    virtual void resizeGL(GLint /*width*/, GLint /*height*/) noexcept;

    /// Set the view matrix from somewhere else
    void setViewMatrix(glm::mat4 _V) {m_V = _V;}

    /// Set the projection matrix from somewhere else
    void setProjMatrix(glm::mat4 _P) {m_P = _P;}

protected:
    /// Check for generic OpenGL errors
    static GLvoid CheckError( const char* label ) noexcept;

    /// Check the currently bound framebuffer for errors
    static GLvoid CheckFrameBuffer() noexcept;

    /// Check the shader for compilation errors
    static GLvoid CheckShader(GLuint id, const char* name) noexcept;

    /// Check a program for linking errors
    static GLvoid CheckProgram(GLuint id, const char* name) noexcept;

    /// Dimensions of the window
    GLint m_width, m_height;

    /// Ratio of width / height
    GLfloat m_ratio;

    /// Keep a view and projection matrix here to render our scene
    glm::mat4 m_V, m_P;

    /// Function to convert HSV to RGB
    static void hsv2rgb(glm::vec3& rgb, const glm::vec3& hsv);

    /// Read a string from a file with name fileName and return it in str
    static int readTextFile(std::string &str, const std::string& fileName) {
        // Open our file to be ready for action
        std::ifstream file(fileName.c_str());

        // Check to see if the file exists and is ready for reading
        if (!file.good()) return EXIT_FAILURE;

        // Clear the string as it is (probably not necessary)
        str.clear();

        // Determine the length of the file by seeking the end of the file
        file.seekg(0, std::ios::end);

        // Create enough storage in our string to store the text file
        str.reserve(file.tellg());

        // Reset the file to the beginning
        file.seekg(0, std::ios::beg);

        // Copy the contents of the file stream into the string using iterators
        str.assign((std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());

        // Close up our file
        file.close();

        // Return something positive assuming we got this far
        return EXIT_SUCCESS;
    }
};

#endif // SCENE_H
