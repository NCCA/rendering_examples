#include "shadowscene.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <ngl/Obj.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>

/**
 * @brief ShadowScene::ShadowScene
 */
ShadowScene::ShadowScene() : Scene() {
}

void ShadowScene::resizeGL(GLint width, GLint height) noexcept {
    Scene::resizeGL(width,height);
    m_isFBODirty = true;
    m_lightPos = glm::vec3(0.2f,0.2f,1.0f);
}

/**
 * @brief ShadowScene::initGL
 */
void ShadowScene::initGL() noexcept {
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
    shader->loadShader("DepthProgram",              // Name of program
                       "shaders/depth_vert.glsl",   // Vertex shader
                       "shaders/depth_frag.glsl");  // Fragment shader

    // Create the depth of field shader program, which combines pixels in the fragment shader
    shader->loadShader("ShadowProgram",                // Name of program
                       "shaders/shadow_vert.glsl",     // Vertex shader
                       "shaders/shadow_frag.glsl");    // Fragment shader

    // Create a ground plane
    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
    prim->createTrianglePlane("plane",2,2,1,1,ngl::Vec3(0,1,0));   
}

/**
 * @brief ShadowScene::initFBO
 */
void ShadowScene::initFBO() {
    // First delete the FBO if it has been created previously
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE) {
        glDeleteTextures(1, &m_fboDepthId);
        glDeleteFramebuffers(1, &m_fboId);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // The depth buffer is rendered to a texture buffer too
    glGenTextures(1, &m_fboDepthId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fboDepthId);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_DEPTH_COMPONENT16,
                 m_shadowRes,
                 m_shadowRes,
                 0,
                 GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_BYTE,
                 NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Create the frame buffer
    glGenFramebuffers(1, &m_fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_fboDepthId, 0);

    // Set the fragment shader output targets (DEPTH_ATTACHMENT is done automatically)
    GLenum drawBufs[] = {GL_NONE};
    glDrawBuffers(1, drawBufs);

    // Check it is ready to rock and roll
    CheckFrameBuffer();

    // Unbind the framebuffer to revert to default render pipeline
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
 * @brief ShadowScene::paintGL
 * This is a two stage render process: firstly we render a bunch of teapots into our FBO, which
 * writes the colour and depth buffer areas to the textures m_fboTextureId and m_fboDepthId.
 * In the second render pass we bind these two textures and draw a screen aligned plane with
 * these textures applied. The depth of field effect is made by combining these two stages on
 * the fragment shader.
 */
void ShadowScene::paintGL() noexcept {
    // Check if the FBO needs to be recreated. This occurs after a resize.
    if (m_isFBODirty) {
        initFBO();
        m_isFBODirty = false;
    }

    // Bind the FBO to specify an alternative render target
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

    // Set up the viewport
    glViewport(0,0,m_width,m_height);

    // Clear the screen (fill with our glClearColor)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader for this draw
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    (*shader)["DepthProgram"]->use();
    GLint pid = shader->getProgramID("DepthProgram");

    glm::mat4 depthProjectionMatrix = glm::ortho<float>(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 10.0f);
    glm::mat4 depthViewMatrix = glm::lookAt(-m_lightPos, glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f));

    // Draw the scene from the light perspective
    drawScene(pid, depthViewMatrix, depthProjectionMatrix, glm::mat4(1.0f));

    // Unbind our FBO
    glBindFramebuffer(GL_FRAMEBUFFER,0);

    // Find the depth of field shader
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0,0,m_width,m_height);

    // Now bind our rendered image which should be in the frame buffer for the next render pass    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fboDepthId);
    (*shader)["ShadowProgram"]->use();
    pid = shader->getProgramID("ShadowProgram");

    // This is the active texture unit!
    glUniform1i(glGetUniformLocation(pid, "depthTex"), 0);

    // Set the light position on the shader
    glUniform4fv(glGetUniformLocation(pid, "Light.Position"), 1, glm::value_ptr(glm::vec4(m_lightPos,1.0f)));

    // Draw the scene, this time from the camera perspective
    drawScene(pid, m_V, m_P, depthViewMatrix * depthProjectionMatrix);
    glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 * @brief drawScene
 * @param pid
 * @param V The view matrix
 * @param P The projection matrix
 */
void ShadowScene::drawScene(const GLuint &pid, const glm::mat4 &V, const glm::mat4 &P, const glm::mat4 &depthVP) {
    int i;
    glm::vec3 hsv, rgb;

    // Grab and instance of the VAO primitives path
    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();

    // Needed for colour cycling
    float colourStep = 1.0f / float(m_numObjects);
    glm::mat4 M, MV, MVP;
    glm::mat3 N;
    glm::mat4 depthBiasMVP;
    glm::mat4 biasMatrix(0.5, 0.0, 0.0, 0.0,
                         0.0, 0.5, 0.0, 0.0,
                         0.0, 0.0, 0.5, 0.0,
                         0.5, 0.5, 0.5, 1.0);
    GLuint loc;

    for (i=0; i<m_numObjects; ++i) {
        // Calculate colour by cycling in HSV colour space
        hsv = glm::vec3(float(i) * colourStep, 1.0f, 1.0f);
        hsv2rgb(rgb, hsv);

        // Translate in depth and x to get a cascade effect
        M = glm::translate(glm::mat4(1.0f), glm::vec3(float(i), 0.0f, float(i)));

        // Set this MVP on the GPU
        MVP = P * V * M;
        glUniformMatrix4fv(glGetUniformLocation(pid, "MVP"), //location of uniform
                           1, // how many matrices to transfer
                           false, // whether to transpose matrix
                           glm::value_ptr(MVP)); // a raw pointer to the data

        // Check if these uniforms exist before setting them
        if ((loc = glGetUniformLocation(pid, "MV")) != -1) {
            MV = V * M;
            glUniformMatrix4fv(loc, //location of uniform
                               1, // how many matrices to transfer
                               false, // whether to transpose matrix
                               glm::value_ptr(MV)); // a raw pointer to the data
        }
        if ((loc = glGetUniformLocation(pid, "N")) != -1) {
            N = glm::inverse(glm::mat3(MV));
            glUniformMatrix3fv(loc, //location of uniform
                               1, // how many matrices to transfer
                               true, // whether to transpose matrix
                               glm::value_ptr(N)); // a raw pointer to the data
        }
        if ((loc = glGetUniformLocation(pid, "Material.Kd")) != -1) {
            glUniform3fv(loc, 1, glm::value_ptr(rgb));
        }
        if ((loc = glGetUniformLocation(pid, "depthBiasMVP")) != -1) {
            depthBiasMVP = biasMatrix*depthVP*M;
            glUniformMatrix4fv(loc, //location of uniform
                               1, // how many matrices to transfer
                               false, // whether to transpose matrix
                               glm::value_ptr(depthBiasMVP)); // a raw pointer to the data
        }
        // Draw a teapot primitive
        prim->draw("teapot");
    }

}


