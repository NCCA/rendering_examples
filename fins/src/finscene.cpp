#include "finscene.h"

// The headers below are needed to get matrices from GLM
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

// The NGL includes are needed for the creation of VAO's from geometry
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/VAOFactory.h>
#include <ngl/ShaderLib.h>

FinScene::FinScene() : Scene() {
    m_finScale = 0.01f;
}


void FinScene::initGL() noexcept {
    // Fire up the NGL machinary (not doing this will make it crash)
    ngl::NGLInit::instance();

    // Set background colour
    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

    // Enable 2D texturing
    glEnable(GL_TEXTURE_2D);

    // enable depth testing for drawing
    glEnable(GL_DEPTH_TEST);

    // enable multisampling for smoother drawing
    glEnable(GL_MULTISAMPLE);

    // Create and compile the vertex and fragment shader
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();

    // Create the basic shader used to render the scene with Gouraud shading
    shader->loadShader("ToonShader",               // Name of program
                       "shaders/toon_vert.glsl",   // Vertex shader
                       "shaders/toon_frag.glsl");  // Fragment shader

    // Build the curvature shader the old fashioned way as we have an additional pipeline stage
    shader->createShaderProgram("FinShader");

    shader->attachShader("FinVertex",ngl::ShaderType::VERTEX);
    shader->loadShaderSource("FinVertex","shaders/fins_vert.glsl");

    shader->attachShader("FinFragment",ngl::ShaderType::FRAGMENT);
    shader->loadShaderSource("FinFragment","shaders/fins_frag.glsl");

    shader->attachShader("FinGeometry",ngl::ShaderType::GEOMETRY);
    shader->loadShaderSource("FinGeometry","shaders/fins_geo.glsl");

    shader->compileShader("FinVertex");
    shader->compileShader("FinFragment");
    shader->compileShader("FinGeometry");
    shader->attachShaderToProgram("FinShader","FinVertex");
    shader->attachShaderToProgram("FinShader","FinFragment");
    shader->attachShaderToProgram("FinShader","FinGeometry");
    shader->linkProgramObject("FinShader");

    // Load the Obj file and create a Vertex Array Object
    m_mesh.reset(new ngl::Obj("../common/models/dragon_lowres.obj"));
    m_mesh->createVAO();
}

void FinScene::paintGL() noexcept {
    // Clear the screen (fill with our glClearColor)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the viewport
    glViewport(0,0,m_width,m_height);

    // Use our shader for this draw
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();

    // Our MVP matrices
    glm::mat4 M, MVP, MV;
    glm::mat3 N;

    // Note the matrix multiplication order as we are in COLUMN MAJOR storage
    MVP = m_P * m_V * M;
    MV = m_V * M;
    N = glm::inverse(glm::mat3(MV));

    (*shader)["ToonShader"]->use();
    GLint pid = shader->getProgramID("ToonShader");
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
    m_mesh->draw();

    (*shader)["FinShader"]->use();
    pid = shader->getProgramID("FinShader");
    glUniformMatrix4fv(glGetUniformLocation(pid, "MVP"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MVP)); // a raw pointer to the data
    glUniformMatrix3fv(glGetUniformLocation(pid, "N"), //location of uniform
                       1, // how many matrices to transfer
                       true, // whether to transpose matrix
                       glm::value_ptr(N)); // a raw pointer to the data
    glUniform1f(glGetUniformLocation(pid,"finScale"), m_finScale);


    // Draw our Obj mesh
    m_mesh->draw();
}
