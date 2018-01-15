#include "firstpersoncamera.h"

/**
 * @brief FirstPersonCamera::FirstPersonCamera
 * @param width
 * @param height
 */
FirstPersonCamera::FirstPersonCamera() : Camera(),
    m_position(0.0),
    m_rotation(0.0),
    m_speed(0.0),
    m_movementSpeedFactor(1.0),
    m_pitchSensitivity(0.2),
    m_yawSensitivity(0.2),
    m_holdingForward(false),
    m_holdingBackward(false),
    m_holdingLeftStrafe(false),
    m_holdingRightStrafe(false)
{
}

/**
 * Function to deal with mouse position changes
 */
void FirstPersonCamera::handleMouseMove(double mouseX, double mouseY) {
    // Apply the mouse movement to our rotation vector. The vertical (look up and down)
    // movement is applied on the X axis, and the horizontal (look left and right)
    // movement is applied on the Y Axis
    m_rotation.x += (mouseX-m_lastX)*m_yawSensitivity;
    m_rotation.y += (mouseY-m_lastY)*m_pitchSensitivity;

    // Limit loking up to vertically up
    if (m_rotation.x < -glm::pi<float>()*0.5f) m_rotation.x = -glm::pi<float>()*0.5f;

    // Limit looking down to vertically down
    if (m_rotation.x > glm::pi<float>()*0.5f) m_rotation.x = glm::pi<float>()*0.5f;

    // Looking left and right - keep angles in the range 0.0 to 360.0
    // 0 degrees is looking directly down the negative Z axis "North", 90 degrees is "East", 180 degrees is "South", 270 degrees is "West"
    // We can also do this so that our 360 degrees goes -180 through +180 and it works the same, but it's probably best to keep our
    // range to 0 through 360 instead of -180 through +180.
    if (m_rotation.y < 0.0f) m_rotation.y = 0.0f;
    if (m_rotation.y > glm::pi<float>()*2.0f) m_rotation.y = glm::pi<float>()*2.0f;

    // Set the last mouse position
    m_lastX = mouseX; m_lastY = mouseY;
}

/**
 * Function to calculate which direction we need to move the camera and by what amount
 */
void FirstPersonCamera::update() {
    // Vector to break up our movement into components along the X, Y and Z axis
    glm::dvec3 movement(0.0f);

    // Get the sine and cosine of our X and Y axis rotation
    double sinXRot = sin( toRads( m_rotation.x ) );
    double cosXRot = cos( toRads( m_rotation.x ) );
    double sinYRot = sin( toRads( m_rotation.y ) );
    double cosYRot = cos( toRads( m_rotation.y ) );

    // This cancels out moving on the Z axis when we're looking up or down
    double pitchLimitFactor = cosXRot;

    if (m_holdingForward) {
        movement.x += (sinYRot * pitchLimitFactor);
        movement.y -= sinXRot;
        movement.z += -cosYRot * pitchLimitFactor;
    }

    if (m_holdingBackward) {
        movement.x += -sinYRot * pitchLimitFactor;
        movement.y += sinXRot;
        movement.z += cosYRot * pitchLimitFactor;
    }
    if (m_holdingLeftStrafe) {
        movement.x += -cosYRot;
        movement.z += -sinYRot;
    }
    if (m_holdingRightStrafe) {
        movement.x += cosYRot;
        movement.z += sinYRot;
    }

    // Normalise our movement vector and integrate
    double t = elapsedTime();
    if (length(movement) > 0.0f) {
        m_position += normalize(movement) * m_movementSpeedFactor * t;
    }

    // Calculate the view matrix
    // - Set to identity
    m_V = glm::mat4(1.0f);
    // - Rotate in X and Y
    m_V = glm::rotate(m_V, float(m_rotation.x), glm::vec3(1.0f,0.0f,0.0f));
    m_V = glm::rotate(m_V, float(m_rotation.y), glm::vec3(0.0f,1.0f,0.0f));
    // - Translate to the position of the viewer
    m_V = glm::translate(m_V, -glm::vec3(m_position));

    // Calculate our projection matrix (same as base class)
    m_P = glm::perspective(m_fovy, m_aspect, m_zNear, m_zFar);

    std::cout << "r = " << glm::to_string(m_rotation) << "\n";
    std::cout << "t = " << glm::to_string(m_position) << "\n";
    //std::cout << "V = " << glm::to_string(m_V) << "\n";
    //std::cout << "P = " << glm::to_string(m_P) << "\n";
}

/**
 * @brief FirstPersonCamera::handleKey
 * @param theKey
 */
void FirstPersonCamera::handleKey(int theKey, bool isPress) {
    switch (theKey) {
    case 'W':
        m_holdingForward = isPress;
        break;
    case 'S':
        m_holdingBackward = isPress;
        break;
    case 'A':
        m_holdingLeftStrafe = isPress;
        break;
    case 'D':
        m_holdingRightStrafe = isPress;
        break;
    default:
        break;
    }
}
