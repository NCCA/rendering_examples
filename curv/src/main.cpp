// Must include our scene first because of GL dependency order
#include "curvscene.h"
#include "trackballcamera.h"
#include "fixedcamera.h"

// Includes for GLFW
#include <GLFW/glfw3.h>

CurvScene g_scene;
TrackballCamera g_camera;

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
        case GLFW_KEY_RIGHT_BRACKET: // increase alphaX  
            g_scene.setAlphaX(g_scene.getAlphaX() + 0.1f);
            break;
        case GLFW_KEY_LEFT_BRACKET: // decrease alphaX
            if (g_scene.getAlphaX() >= 0.0f)
                g_scene.setAlphaX(g_scene.getAlphaX() - 0.1f);
        break;
        case GLFW_KEY_KP_ADD: // increase alphaY                       
            g_scene.setAlphaY(g_scene.getAlphaY() + 0.1f);
            break;
        case GLFW_KEY_KP_SUBTRACT: // decrease alphaY
            if (g_scene.getAlphaY() >= 0.0f)
                g_scene.setAlphaY(g_scene.getAlphaY() - 0.1f);
            break;
        case GLFW_KEY_SPACE: // toggle whether the vectors are visible
            g_scene.toggleVectors();
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

    // Output some instructions for use
    std::cout << "******************* USAGE **************************\n"
              << "[: Decrease alpha_x by 0.1\n"
              << "]: Increase alpha_x by 0.1\n"
              << "Keypad-: Decrease alpha_y by 0.1\n"
              << "Keypad+: Increase alpha_y by 0.1\n"
              << "<SPACE>: Toggle curvature vector visualisation\n"
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
