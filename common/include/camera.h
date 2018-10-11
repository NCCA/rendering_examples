#ifndef CAMERA_H
#define CAMERA_H

// For basic stream output
#include <iostream>

// Used only for sin() and cos() functions
#include <math.h>

// Needed for GLFW output
#include <GLFW/glfw3.h>

// Needed for matrix operations
#include <glm/glm.hpp>

// Used to maintain a timer
#include <chrono>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

/**
 * @brief The Camera class
 */
class Camera
{
public:
    /// Constructor
    Camera();

    /// Trigger this when the window resizes
    virtual void resize(int /*width*/, int /*height*/);

    /// Call this before you need to retrieve the matrices from the camera
    virtual void update() = 0;

    /// Handle keypress / release events
    virtual void handleKey(int /*glfw key*/, bool /*isPress*/) {}

    /// Mouse movement handler to look around
    virtual void handleMouseMove(double /*mouseX*/, double /*mouseY*/) {}

    /// Mouse click handler
    virtual void handleMouseClick(double /*mouseX*/, double /*mouseY*/, int /*glfw button*/, int /*glfw action*/, int /*glfw mods*/) {}

    /// Functions to retrieve matrices from the camera
    const glm::mat4 &viewMatrix() {return m_V;}
    const glm::mat4 &projMatrix() {return m_P;}

    /// Set the initial mouse position manually
    virtual void setInitialMousePos(float mouseX, float mouseY) {m_lastX=mouseX; m_lastY=mouseY;}

protected:
    /// Keep track of the camera dimensions
    int m_windowWidth;
    int m_windowHeight;

    /// Keep track of the camera parameters
    float m_fovy;   //< Field of view in y
    float m_aspect; //< Aspect ratio
    float m_zNear;  //< Near clipping plane
    float m_zFar;   //< Far clipping plane

    /// Keep track of the last time
    std::chrono::high_resolution_clock::time_point m_lastTime;

    /// Return the elapsed time since this was last called
    double elapsedTime();

    /// Our view and project matrices are stored with the camera and retrieved as needed
    glm::mat4 m_V, m_P;

    /// Last mouse coordinates
    double m_lastX, m_lastY;

    /// The value of 1 degree in radians
    const double TO_RADS = 3.141592654 / 180.0;

    /// A silly function to convert to radians (there are better ways to do this)
    double toRads(const double &degs) const {
        return degs * TO_RADS;
    }

};

#endif // CAMERA_H
