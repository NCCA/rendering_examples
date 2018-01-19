// Must include our scene first because of GL dependency order
#include "envscene.h"

// This will probably already be included by a scene file
#include "glinclude.h"
#include "fixedcamera.h"
#include "trackballcamera.h"

// Includes for GLFW
#include <GLFW/glfw3.h>

/// A scene object
EnvScene g_scene;

/// A camera object
TrackballCamera g_camera;

/******************************************************************
 * GLFW Callbacks
 * These functions are triggered on an event, such as a keypress
 * or mouse click. They need to be passed on to the relevant
 * handler, for example, the camera or scene.
 ******************************************************************/
/**
 * @brief error_callback Function to catch GLFW errors.
 * @param error GLFW error code
 * @param description Text description
 */
void error_callback(int error, const char* description) {
    std::cerr << "Error ("<<error<<"): " << description << "\n";
}

/**
 * @brief cursor_callback Function to catch GLFW cursor movement
 * @param xpos x position
 * @param ypos y position
 */
void cursor_callback(GLFWwindow* /*window*/, double xpos, double ypos) {
    g_camera.handleMouseMove(xpos, ypos);
}

/**
 * @brief mouse_button_callback Handle a mouse click or release
 * @param window Window handle (unused currently)
 * @param button Which button was pressed (e.g. left or right button)
 * @param action GLFW code for the action (GLFW_PRESS or GLFW_RELEASE)
 * @param mods Other keys which are currently being held down (e.g. GLFW_CTRL)
 */
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    // Retrieve the position of the mouse click
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Call the camera to handle the click action
    g_camera.handleMouseClick(xpos, ypos, button, action, mods);
}

/**
 * @brief key_callback Handle key press or release
 * @param window window handle (unused currently)
 * @param key The key that was pressed
 * @param action GLFW code for the action (GLFW_PRESS or GLFW_RELEASE)
 * @param mods Other keys which are currently being held down (e.g. GLFW_CTRL)
 */
void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int mods)
{
    // Escape exits the application
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);        
    } else {
        // Any other keypress should be handled by our camera
        g_camera.handleKey(key, (action == GLFW_PRESS) );
    }
}

/**
 * @brief resize_callback Handle a window resize event.
 * @param width New window width
 * @param height New window height
 */
void resize_callback(GLFWwindow */*window*/, int width, int height) {
    g_camera.resize(width,height);
    g_scene.resizeGL(width,height);
}


/**
 * @brief main The main application loop
 * @return Whatever glfw returns when you glfwTerminate()
 */
int main() {
    if (!glfwInit()) {
        // Initialisation failed
        glfwTerminate();        
    }
    
    // Register error callback
    glfwSetErrorCallback(error_callback);

    // Set our OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

    // Create our window in a platform agnostic manner
    int width = 1024; int height = 768;
    GLFWwindow* window = glfwCreateWindow(width, // width 
                                          height, // height
                                          "My Title", // title 
                                          nullptr, // monitor for full screen
                                          nullptr); // return value on failure

    if (window == nullptr) {
            // Window or OpenGL context creation failed
            glfwTerminate();
    }
    // Make the window an OpenGL window
    glfwMakeContextCurrent(window);

    // Initialise GLEW - note this generates an "invalid enumerant" error on some platforms
#if ( (!defined(__MACH__)) && (!defined(__APPLE__)) )
    glewExperimental = GL_TRUE;
    glewInit();
    glGetError(); // quietly eat errors from glewInit()
#endif

    // Set keyboard callback
    glfwSetKeyCallback(window, key_callback);

    // Disable the cursor for the FPS camera
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse move and click callback
    glfwSetCursorPosCallback(window, cursor_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Set the lastX,Y position on the FPS camera
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    g_camera.setInitialMousePos(mouseX, mouseY);

    // Needed for the fixed camera
    g_camera.setTarget(0.0f,0.0f,0.0f);
    g_camera.setEye(0.0f,0.0f,-2.0f);

    // Initialise our OpenGL scene
    g_scene.initGL();

    // Set the window resize callback and call it once
    glfwSetFramebufferSizeCallback(window, resize_callback);
    resize_callback(window, width, height);

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        // Poll events
        glfwPollEvents();

        // Update our camera matrices
        g_camera.update();

        // Set the view-projection matrix
        g_scene.setViewMatrix(g_camera.viewMatrix());
        g_scene.setProjMatrix(g_camera.projMatrix());

        // Draw our GL stuff
        g_scene.paintGL();

        // Swap the buffers
        glfwSwapBuffers(window);
    }

    // Close up shop
    glfwDestroyWindow(window);
    glfwTerminate();
}
