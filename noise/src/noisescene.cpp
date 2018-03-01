#include "noisescene.h"

#include <glm/gtc/type_ptr.hpp>
#include <ngl/Obj.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>

NoiseScene::NoiseScene() : Scene(), m_noiseTex(12, 10.0f, 0.5f, 0.0f, 1.0f, 1024) {}

/**
 * @brief ObjLoaderScene::initGL
 */
void NoiseScene::initGL() noexcept {
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
    shader->loadShader("DataNoiseProgram","shaders/datanoise_vert.glsl","shaders/datanoise_frag.glsl");
    shader->loadShader("ShaderNoiseProgram","shaders/shadernoise_vert.glsl","shaders/shadernoise_frag.glsl");

    // Our third 3D texture is for diffuse and specular variation, and is simplex noise
    glActiveTexture(GL_TEXTURE0);

    // Generate our diffuse texture (this is the slow bit)
    m_noiseTex.generate();

    // This binds the noise texture to the current texture unit
    m_noiseTex.bind();

    ngl::ShaderLib::instance()->use("DataNoiseProgram");
    shader->setUniform("noiseTex", 0); // The "0" here is the Active Texture unit
}

void NoiseScene::paintGL() noexcept {
    // Clear the screen (fill with our glClearColor)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the viewport
    glViewport(0,0,m_width,m_height);

    // Use our shader for this draw
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    GLint pid;

    // Allow the user to select the current shader method
    switch(m_noiseMethod) {
    case NOISE_DATA:
        (*shader)["DataNoiseProgram"]->use();
        pid = shader->getProgramID("DataNoiseProgram");
        break;
    default:
        (*shader)["ShaderNoiseProgram"]->use();
        pid = shader->getProgramID("ShaderNoiseProgram");
        break;    
    }

    // Our MVP matrices
    glm::mat4 M = glm::mat4(1.0f);
    glm::mat4 MVP, MV;
    glm::mat3 N;

    // Note the matrix multiplication order as we are in COLUMN MAJOR storage
    MV = m_V * M;
    N = glm::inverse(glm::mat3(MV));
    MVP = m_P * MV;

    // Set this MVP on the GPU
    glUniformMatrix4fv(glGetUniformLocation(pid, "MVP"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MVP)); // a raw pointer to the data
    glUniformMatrix4fv(glGetUniformLocation(pid, "MV"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MV)); // a raw pointer to the data
    glUniformMatrix3fv(glGetUniformLocation(pid, "N"), //location of uniform
                       1, // how many matrices to transfer
                       true, // whether to transpose matrix
                       glm::value_ptr(N)); // a raw pointer to the data

    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
    prim->draw("teapot");
}
