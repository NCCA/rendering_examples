#ifndef TRACKBALLCAMERA_H
#define TRACKBALLCAMERA_H

#include "camera.h"
#include <glm/gtc/quaternion.hpp>

/**
 * @brief The TrackballCamera class
 */
class TrackballCamera : public Camera
{
    typedef enum {
        TRACKBALL_PASSIVE,
        TRACKBALL_ROTATING,
        TRACKBALL_ZOOMING
    } TrackballState;
public:
    /// Construct our empty trackball (no destructor needed as we don't allocate dynamic memory)
    TrackballCamera();

    /// Call this before you need to retrieve the matrices from the camera
    virtual void update();

    /// Mouse movement handler to look around
    virtual void handleMouseMove(double /*mouseX*/, double /*mouseY*/);

    /// Mouse click handler
    virtual void handleMouseClick(double /*mouseX*/, double /*mouseY*/, int /*button*/, int /*action*/, int /*mods*/);

    /// Set the direction you're looking
    void setTarget(const double& x, const double& y, const double& z) {m_target = glm::dvec3(x,y,z);}

    /// Set the position that our camera is
    void setEye(const double& x, const double& y, const double& z) {m_eye = glm::dvec3(x,y,z);}

    /// Set the zoom by scaling the eye position
    void setZoom(const double& zoom = 1.0) noexcept {m_zoom = zoom;}

    /// Controller sensitivity
    void setSensitivity(const double& sensitivity=0.01) noexcept {m_sensitivity = sensitivity;}

    /// Return some parameters for specialist renderers
    glm::vec3 getTarget() const {return glm::vec3(m_target);}
    glm::vec3 getTransformedEye() const {return glm::vec3(m_transformedEye);}

private:   
    /// Rotate based on the current mouse position and the mouse click position
    void mouseRotate(double /*xpos*/, double /*ypos*/);

    /// Zoom based on the current mouse position and the position of the mouse click
    void mouseZoom(double /*xpos*/, double /*ypos*/);

    /// Keep track of our current trackball state
    TrackballState m_state;

    /// Store the yaw and pitch
    double m_yaw, m_pitch, m_zoom, m_sensitivity;

    /// Store the target and position with this class
    glm::dvec3 m_target, m_eye, m_transformedEye;

    /// Store the last yaw and pitch so that the trackball stays put when the mouse is released
    double m_lastYaw, m_lastPitch;

    /// Keep track of whether the matrices need to be rebuilt
    bool m_dirty;
};

#endif // TRACKBALLCAMERA_H
