#include "fixedcamera.h"
#include <glm/gtc/matrix_transform.hpp>

FixedCamera::FixedCamera() :
    m_target(0.0f,0.0f,0.0f),
    m_eye(0.0f,0.0f,0.0f) {
}

void FixedCamera::update() {
    // Call the base class to set up the projection matrix
    Camera::update();

    // Now use lookat function to set the view matrix (assume y is up)
    m_V = glm::lookAt(m_eye, m_target, glm::vec3(0.0f,1.0f,0.0f));
}
