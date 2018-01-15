#ifndef FIRSTPERSONCAMERA_H
#define FIRSTPERSONCAMERA_H

#include "camera.h"

/**
 * @brief The FirstPersonCamera class
 * This was customized from http://r3dux.org/2012/12/a-c-camera-class-for-simple-opengl-fps-controls/
 */
class FirstPersonCamera : public Camera {
    protected:
        /// Camera position
        glm::dvec3 m_position;

        /// Camera rotation
        glm::dvec3 m_rotation;

        /// Camera movement speed. When we call the move() function on a camera, it moves using these speeds
        glm::dvec3 m_speed;

        double m_movementSpeedFactor; //< Controls how fast the camera moves
        double m_pitchSensitivity;    //< Controls how sensitive mouse movements affect looking up and down
        double m_yawSensitivity;      //< Controls how sensitive mouse movements affect looking left and right

        /// Holding any keys down?
        bool m_holdingForward;
        bool m_holdingBackward;
        bool m_holdingLeftStrafe;
        bool m_holdingRightStrafe;

    private:


    public:
        /// Constructor
        FirstPersonCamera();

        /// Handle keypress / release events
        void handleKey(int /*glfw key*/, bool /*isPress*/);

        /// Mouse movement handler to look around
        void handleMouseMove(double mouseX, double mouseY);

        /// Method to move the camera based on the current direction
        void update();

        /// Setters to allow for change of vertical (pitch) and horizontal (yaw) mouse movement sensitivity
//        float getPitchSensitivity()            { return pitchSensitivity;  }
//        void  setPitchSensitivity(float value) { pitchSensitivity = value; }
//        float getYawSensitivity()              { return yawSensitivity;    }
//        void  setYawSensitivity(float value)   { yawSensitivity   = value; }
};

#endif // FIRSTPERSONCAMERA_H

