#include "sdfscene.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>
#include <ngl/Obj.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>

SDFScene::SDFScene() : Scene() {
    // Set the time since we started running the scene
    m_startTime = std::chrono::high_resolution_clock::now();
    m_colourMode = 1;
    m_shapeType = 0;
    m_isBlending = true;
    m_eye = glm::vec3(0.0, 0.0, 2.0);
    m_target = glm::vec3(0.0, 0.0, 0.0);
}

/**
 * @brief ObjLoaderScene::initGL
 */
void SDFScene::initGL() noexcept {
    // Fire up the NGL machinary (not doing this will make it crash)
    ngl::NGLInit::instance();

    // Set background colour
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // enable depth testing for drawing
    glEnable(GL_DEPTH_TEST);

    // enable multisampling for smoother drawing
    glEnable(GL_MULTISAMPLE);

    // Create and compile all of our shaders
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    shader->loadShader("SDFProgram","shaders/sdf_vert.glsl","shaders/sdf_frag.glsl");

    // Create a screen oriented plane
    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
    prim->createTrianglePlane("plane",2,2,1,1,ngl::Vec3(0,1,0));
}

void SDFScene::paintGL() noexcept {
    // Clear the screen (fill with our glClearColor)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the viewport
    glViewport(0,0,m_width,m_height);

    // Use our shader for this draw
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    (*shader)["SDFProgram"]->use();
    GLint pid = shader->getProgramID("SDFProgram");

    // Our MVP matrices
    glm::mat4 M = glm::mat4(1.0f);
    glm::mat4 MVP, MV;
    glm::mat3 N;

    // Note the matrix multiplication order as we are in COLUMN MAJOR storage
    MV = m_V * M;
    N = glm::inverse(glm::mat3(MV));
    MVP = m_P * MV;

    // Calculate the elapsed time since the programme started
    auto now = std::chrono::high_resolution_clock::now();
    double t = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_startTime).count()  * 0.001;

    // Set the viewport resolution
    glUniform3fv(glGetUniformLocation(pid, "iResolution"), 1, glm::value_ptr(glm::vec3(float(m_width), float(m_height), 0.0f)));

    // Set the time elapsed since the programme started
    glUniform1f(glGetUniformLocation(pid, "iTime"), float(t));

    // Set the current colour mode (0,1 or 2)
    glUniform1i(glGetUniformLocation(pid, "colourMode"), m_colourMode);

    // Set the current shape to render
    glUniform1i(glGetUniformLocation(pid, "shapeType"), m_shapeType);

    // Set whether blending is used between shapes
    glUniform1i(glGetUniformLocation(pid, "isBlending"), (m_isBlending)?1:0);    
    
    // The default NGL plane isn't screen oriented so we still have to rotate it around the x-axis
    // to align with the screen
    MVP = glm::rotate(glm::mat4(1.0f), glm::pi<float>() * 0.5f, glm::vec3(1.0f,0.0f,0.0f));
    glUniformMatrix4fv(glGetUniformLocation(pid, "MVP"), 1, false, glm::value_ptr(MVP));

    // Transfer over the eye and target position
    glUniform3fv(glGetUniformLocation(pid, "eyepos"), 1, glm::value_ptr(m_eye));
    glUniform3fv(glGetUniformLocation(pid, "target"), 1, glm::value_ptr(m_target));
    
    // Draw the plane that we've created
    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
    prim->draw("plane");    
}

void SDFScene::setColourMode(const int& _mode) {    
    if ((_mode >= 1) && (_mode <= 5)) {
        m_colourMode = _mode;
    }
}