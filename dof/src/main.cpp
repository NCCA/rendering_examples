// Must include our scene first because of GL dependency order
#include "dofscene.h"
#include "firstpersoncamera.h"
#include "trackballcamera.h"
#include "fixedcamera.h"

// Includes for GLFW
#include <GLFW/glfw3.h>

DofScene g_scene;
TrackballCamera g_camera;
GLfloat g_focalDepth = 1.0f;

void error_callback(int error, const char* description) {
    std::cerr << "Error ("<<error<<"): " << description << "\n";
}

void cursor_callback(GLFWwindow* /*window*/, double xpos, double ypos) {
    g_camera.handleMouseMove(xpos, ypos);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    // Retrieve the position of the mouse click
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Call the camera to handle the click action
    g_camera.handleMouseClick(xpos, ypos, button, action, mods);
}

void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int mods) {
    // Escape exits the application
    if (action == GLFW_PRESS) {
        switch(key) {
        case GLFW_KEY_ESCAPE: //exit the application
            glfwSetWindowShouldClose(window, true);
        case GLFW_KEY_LEFT_BRACKET: //decrease focal depth
            g_focalDepth -= (g_focalDepth > 0.0f)?0.1f:0.0f;
            g_scene.setFocalDepth(g_focalDepth);
            break;
        case GLFW_KEY_RIGHT_BRACKET: // increase focal depth
            g_focalDepth += (g_focalDepth < 3.0f)?0.1f:0.0f;
            g_scene.setFocalDepth(g_focalDepth);
            break;
        case GLFW_KEY_B: // toggle blur filter method
            g_scene.toggleBlurFilter();
            break;
        }
    }
    // Any other keypress should be handled by our camera
    g_camera.handleKey(key, (action == GLFW_PRESS) );
}


void resize_callback(GLFWwindow */*window*/, int width, int height) {
    // Resize event
    //int width, height;
    //glfwGetFramebufferSize(window, &width, &height);
    g_camera.resize(width,height);
    g_scene.resizeGL(width,height);
}

int main() {
    if (!glfwInit()) {
        // Initialisation failed
        glfwTerminate();
    }
    
    // Register error callback
    glfwSetErrorCallback(error_callback);

    // Set our OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Create our window in a platform agnostic manner
    int width = 640; int height = 480;
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

    // Output some instructions for usage
    std::cout << "******************* USAGE **************************\n"
              << "[: Decrease focal depth target\n"
              << "]: Increase focal depth target\n"
              << "b: Switch blurring method (either Gaussian or Poisson)\n"              
              << "<ESC>: Quit\n"
              << "****************************************************\n";
    
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
