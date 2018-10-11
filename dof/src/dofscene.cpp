#include "dofscene.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <ngl/Obj.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>

/**
 * @brief DofScene::DofScene
 */
DofScene::DofScene() : Scene() {
}

void DofScene::resizeGL(GLint width, GLint height) noexcept {
    Scene::resizeGL(width,height);
    m_isFBODirty = true;
}

/**
 * @brief DofScene::initGL
 */
void DofScene::initGL() noexcept {
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
    shader->loadShader("GouraudProgram",              // Name of program
                       "../common/shaders/gouraud_vert.glsl",   // Vertex shader
                       "../common/shaders/gouraud_frag.glsl");  // Fragment shader

    // Create the depth of field shader program, which combines pixels in the fragment shader
    shader->loadShader("DofProgram",                // Name of program
                       "shaders/dof_vert.glsl",     // Vertex shader
                       "shaders/dof_frag.glsl");    // Fragment shader

    // Create a screen oriented plane
    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
    prim->createTrianglePlane("plane",2,2,1,1,ngl::Vec3(0,1,0));   
}

/**
 * @brief DofScene::initFBO
 */
void DofScene::initFBO() {
    // First delete the FBO if it has been created previously
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE) {
        glDeleteTextures(1, &m_fboTextureId);
        glDeleteTextures(1, &m_fboDepthId);
        glDeleteFramebuffers(1, &m_fboId);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Generate a texture to write the FBO result to
    glGenTextures(1, &m_fboTextureId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_fboTextureId);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 m_width,
                 m_height,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // The depth buffer is rendered to a texture buffer too
    glGenTextures(1, &m_fboDepthId);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_fboDepthId);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_DEPTH_COMPONENT,
                 m_width,
                 m_height,
                 0,
                 GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_BYTE,
                 NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Create the frame buffer
    glGenFramebuffers(1, &m_fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTextureId, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_fboDepthId, 0);

    // Set the fragment shader output targets (DEPTH_ATTACHMENT is done automatically)
    GLenum drawBufs[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBufs);

    // Check it is ready to rock and roll
    CheckFrameBuffer();

    // Unbind the framebuffer to revert to default render pipeline
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
 * @brief DofScene::paintGL
 * This is a two stage render process: firstly we render a bunch of teapots into our FBO, which
 * writes the colour and depth buffer areas to the textures m_fboTextureId and m_fboDepthId.
 * In the second render pass we bind these two textures and draw a screen aligned plane with
 * these textures applied. The depth of field effect is made by combining these two stages on
 * the fragment shader.
 */
void DofScene::paintGL() noexcept {
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
    (*shader)["GouraudProgram"]->use();

    // Our MVP matrices
    glm::mat4 MVP, MV;
    glm::mat3 N;

    // Grab and instance of the VAO primitives path
    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();

    // Needed for colour cycling
    float colourStep = 1.0f / float(m_numObjects);
    glm::vec3 hsv, rgb;

    // Store the program id
    GLint pid = shader->getProgramID("GouraudProgram");

    int i;
    for (i=0; i<m_numObjects; ++i) {
        // Calculate colour by cycling in HSV colour space
        hsv = glm::vec3(float(i) * colourStep, 1.0f, 1.0f);
        hsv2rgb(rgb, hsv);

        // Translate in depth and x to get a cascade effect
        glm::mat4 M = glm::translate(glm::mat4(1.0f), glm::vec3(float(i), 0.0f, float(i)));

        // Note the matrix multiplication order as we are in COLUMN MAJOR storage
        MVP = m_P * m_V * M;
        MV = m_V * M;
        N = glm::inverse(glm::mat3(MV));

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

        glUniform3fv(glGetUniformLocation(pid, "Material.Kd"), 1, glm::value_ptr(rgb));

        // Draw a teapot primitive
        prim->draw("teapot");

    }
    // Unbind our FBO
    glBindFramebuffer(GL_FRAMEBUFFER,0);

    // Find the depth of field shader
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0,0,m_width,m_height);

    // Now bind our rendered image which should be in the frame buffer for the next render pass
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_fboTextureId);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_fboDepthId);

    (*shader)["DofProgram"]->use();
    pid = shader->getProgramID("DofProgram");

    glUniform1i(glGetUniformLocation(pid, "colourTex"), 1);
    glUniform1i(glGetUniformLocation(pid, "depthTex"), 2);
    glUniform1f(glGetUniformLocation(pid, "focalDepth"), m_focalDepth);
    glUniform1f(glGetUniformLocation(pid, "blurRadius"), m_blurRadius);
    glUniform2f(glGetUniformLocation(pid, "windowSize"), m_width, m_height);
    setShaderSubroutine();

    MVP = glm::rotate(glm::mat4(1.0f), glm::pi<float>() * 0.5f, glm::vec3(1.0f,0.0f,0.0f));
    glUniformMatrix4fv(glGetUniformLocation(pid, "MVP"), 1, false, glm::value_ptr(MVP));

    prim->draw("plane");
    glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 * @brief DofScene::setShaderSubroutine
 * GLSL handles subroutines very badly. Each shader can have many subroutines, and these
 * are all set at once using an array with the indices of each of the subroutines to use.
 */
void DofScene::setShaderSubroutine() {
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    (*shader)["DofProgram"]->use();
    GLint pid = shader->getProgramID("DofProgram");

    // Determine the different subroutine indices for our blur filters
    GLuint GaussianFilter = glGetSubroutineIndex(pid, GL_FRAGMENT_SHADER, "GaussianFilter");
    GLuint PoissonFilter = glGetSubroutineIndex(pid, GL_FRAGMENT_SHADER, "PoissonFilter");

    // Technically these must all be set, but we should only have 1
    GLuint blurFilter;
    switch(m_blurFilter) {
    case BLUR_POISSON:
        blurFilter = PoissonFilter;
        break;
    default:
        blurFilter = GaussianFilter;
        break;
    }

    // Set the subroutine on the shader
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &blurFilter);
}


