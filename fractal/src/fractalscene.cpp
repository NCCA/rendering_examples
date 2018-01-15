#include "fractalscene.h"

#include <glm/gtc/type_ptr.hpp>
#include <ngl/Obj.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>

FractalScene::FractalScene() : Scene() {
    // Set the time since we started running the scene
    m_startTime = std::chrono::high_resolution_clock::now();
}


void FractalScene::initGL() noexcept {
    // Fire up the NGL machinary (not doing this will make it crash)
    ngl::NGLInit::instance();

    // Set background colour
    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

    // enable depth testing for drawing
    glEnable(GL_DEPTH_TEST);

    // enable multisampling for smoother drawing
    glEnable(GL_MULTISAMPLE);

    // Create and compile the vertex and fragment shader
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    shader->loadShader("FractalProgram",
                       "shaders/fractal_vert.glsl",
                       "shaders/fractal_frag.glsl");

    // Create a screen oriented plane
    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
    prim->createTrianglePlane("plane",2,2,1,1,ngl::Vec3(0,1,0));
}

void FractalScene::paintGL() noexcept {
    // Clear the screen (fill with our glClearColor)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the viewport
    glViewport(0,0,m_width,m_height);

    // Use our shader for this draw
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    (*shader)["FractalProgram"]->use();
    GLint pid = shader->getProgramID("FractalProgram");


    // For animating scenes we need to send an elapsed time to the shader
    double t = 0.0;
    if (m_animating) {
        auto now = std::chrono::high_resolution_clock::now();
        t = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_startTime).count()  * 0.001;
    }
    glUniform1f(glGetUniformLocation(pid, "t"), t);

    // Set which shader is currently in use
    setShaderSubroutine();


    // Our MVP matrix for transforms
    glm::mat4 MVP;

    // Grab a handle to the primitive singleton
    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();

    // Determine which object is currently being rendered
    switch(m_renderObject) {
    case OBJECT_TEAPOT:
        MVP = m_P * m_V;
        glUniformMatrix4fv(glGetUniformLocation(pid, "MVP"), 1, false, glm::value_ptr(MVP));
        prim->draw("teapot");
        break;
    default:
        // The default NGL plane isn't screen oriented so we still have to rotate it around the x-axis
        // to align with the screen
        MVP = glm::rotate(glm::mat4(1.0f), glm::pi<float>() * 0.5f, glm::vec3(1.0f,0.0f,0.0f));
        glUniformMatrix4fv(glGetUniformLocation(pid, "MVP"), 1, false, glm::value_ptr(MVP));
        // Draw the plane that we've created
        prim->draw("plane");
        break;
    }
}

/**
 * @brief FractalScene::setShaderSubroutine
 */
void FractalScene::setShaderSubroutine() {
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    (*shader)["FractalProgram"]->use();
    GLint pid = shader->getProgramID("FractalProgram");

    // Technically these must all be set, but we should only have 1 subroutine
    GLuint fractalChoice;
    switch(m_currentFractal) {
    case FRACTAL_MANDELBROT:
        fractalChoice = glGetSubroutineIndex(pid, GL_FRAGMENT_SHADER, "MandelbrotFunction");
        break;
    default:
        fractalChoice = glGetSubroutineIndex(pid, GL_FRAGMENT_SHADER, "JuliaFunction");
        break;
    }

    // Set the subroutine on the shader
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &fractalChoice);
}
