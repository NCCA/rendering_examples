#include "shaderscene.h"

#include <glm/gtc/type_ptr.hpp>
#include <ngl/Obj.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>

ShaderScene::ShaderScene() : Scene() {}

/**
 * @brief ObjLoaderScene::initGL
 */
void ShaderScene::initGL() noexcept {
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
    shader->loadShader("GouraudProgram","../common/shaders/gouraud_vert.glsl","../common/shaders/gouraud_frag.glsl");
    shader->loadShader("PhongProgram","shaders/phong_vert.glsl","shaders/phong_frag.glsl");
    shader->loadShader("CookTorranceProgram","shaders/phong_vert.glsl","shaders/cooktorrance_frag.glsl");
    shader->loadShader("ToonProgram","shaders/phong_vert.glsl","shaders/toon_frag.glsl");
}

void ShaderScene::paintGL() noexcept {
    // Clear the screen (fill with our glClearColor)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the viewport
    glViewport(0,0,m_width,m_height);

    // Use our shader for this draw
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    GLint pid;

    // Allow the user to select the current shader method
    switch(m_shaderMethod) {
    case SHADER_PHONG:
        (*shader)["PhongProgram"]->use();
        pid = shader->getProgramID("PhongProgram");
        break;
    case SHADER_TOON:
        (*shader)["ToonProgram"]->use();
        pid = shader->getProgramID("ToonProgram");
        break;
    case SHADER_COOKTORRANCE:
        (*shader)["CookTorranceProgram"]->use();
        pid = shader->getProgramID("CookTorranceProgram");
        break;
    default:
        (*shader)["GouraudProgram"]->use();
        pid = shader->getProgramID("GouraudProgram");
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
