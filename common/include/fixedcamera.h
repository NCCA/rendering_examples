#ifndef FIXEDCAMERA_H
#define FIXEDCAMERA_H

#include "camera.h"

/**
 * @brief The FixedCamera class
 */
class FixedCamera : public Camera {
public:
    /// Construct the camera
    FixedCamera();

    /// Call this before you need to retrieve the matrices from the camera
    void update();

    /// Set the direction you're looking
    void setTarget(float x, float y, float z) {m_target = glm::vec3(x,y,z);}

    /// Set the position that our camera is
    void setEye(float x, float y, float z) {m_eye = glm::vec3(x,y,z);}

private:
    /// Store the target and position with this class
    glm::vec3 m_target, m_eye;
};

#endif // FIXEDCAMERA_H
