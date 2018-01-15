#include "scene.h"

// Needed for memset
#include <string.h>

Scene::Scene() : m_width(1), m_height(1), m_ratio(1.0f) {

}

/**
 * @brief Scene::resizeGL
 * @param width
 * @param height
 * @return
 */
GLvoid Scene::resizeGL(GLint width, GLint height) noexcept {
    m_width = width; m_height = height;
    m_ratio = m_width / (float) m_height;
}

/**
 * @brief Scene::CheckError retrieves the last GL error from the log
 * @param label: An arbitrary string used to identify what command you're checking
 * @return nothing!
 */
GLvoid Scene::CheckError( const char* label ) noexcept {
    GLenum error;
    error = glGetError();
    while ( GL_NO_ERROR != error ) {
        std::cerr << "<GLERROR>" << label << ": " << gluErrorString(error) << "\n";
        error = glGetError();
    }
}

/**
 * @brief Scene::CheckFrameBuffer Outputs result of test on the Framebuffer as nice string.
 * @return Nothing!
 */
GLvoid Scene::CheckFrameBuffer() noexcept {
    switch(glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
    case GL_FRAMEBUFFER_UNDEFINED:
        std::cerr<<"GL_FRAMEBUFFER_UNDEFINED: returned if target is the default framebuffer, but the default framebuffer does not exist.\n";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        std::cerr<<"GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: returned if any of the framebuffer attachment points are framebuffer incomplete.\n";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        std::cerr<<"GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: returned if the framebuffer does not have at least one image attached to it.\n";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        std::cerr<<"GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: returned if the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment point(s) named by GL_DRAWBUFFERi.\n";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        std::cerr<<"GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: returned if GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER.\n";
        break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
        std::cerr<<"GL_FRAMEBUFFER_UNSUPPORTED: returned if the combination of internal formats of the attached images violates an implementation-dependent set of restrictions. GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE is also returned if the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not GL_TRUE for all attached textures.\n";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
        std::cerr<<"GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: returned if any framebuffer attachment is layered, and any populated attachment is not layered, or if all populated color attachments are not from textures of the same target.\n";
        break;
    case GL_FRAMEBUFFER_COMPLETE:
        std::cerr<<"GL_FRAMEBUFFER_COMPLETE: returned if everything is groovy!\n";
        break;
    default:
        std::cerr<<glCheckFramebufferStatus(GL_FRAMEBUFFER)<<": Undefined framebuffer return value: possible error elsewhere?\n";
        break;
    }
}

/**
 * @brief Scene::CheckShader Outputs the status of shader id
 * @param id: Shader handle
 * @param name: A user name for the shader so output makes sense.
 * @return Nothing!
 */
GLvoid Scene::CheckShader(GLuint id, const char* name) noexcept {
    GLint testval;
    glGetShaderiv(id, GL_COMPILE_STATUS, &testval);
    if(testval == GL_FALSE) {
        char infolog[1024];
        glGetShaderInfoLog(id,1024,NULL,infolog);
        std::cout << "Shader \""<<name<<"\" failed to compile. Error:" << infolog << "\n";
        exit(0); // Bomb out
    }
}

/**
 * @brief Scene::CheckProgram
 * @param id
 * @param name
 * @return Nothing!
 */
GLvoid Scene::CheckProgram(GLuint id, const char *name) noexcept {
    const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    GLsizei length = 0;

    memset(buffer, 0, BUFFER_SIZE);
    glGetProgramInfoLog(id, BUFFER_SIZE, &length, buffer);
    if (length > 0) {
        std::cerr << "Program " << name << " Link Message: " << buffer << std::endl;
        exit(0);
    }

    glValidateProgram(id);
    GLint status;
    glGetProgramiv(id, GL_VALIDATE_STATUS, &status);
    if (status == GL_FALSE) {
        std::cerr << "Program " << name << " Validation Failed" << std::endl;
        exit(0);
    }
}

/**
 * @brief DofScene::hsv2rgb
 * @param rgb
 * @param hsv
 * Canabolized from here: https://gist.github.com/fairlight1337/4935ae72bcbcc1ba5c72
 */
void Scene::hsv2rgb(glm::vec3& rgb, const glm::vec3& hsv) {
    float fC = hsv.z * hsv.y; // Chroma
    float fHPrime = fmod(hsv.x * 6, 6);
    float fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
    float fM = hsv.z - fC;

    if(0 <= fHPrime && fHPrime < 1) {
        rgb = glm::vec3(fC, fX, 0.0f);
    } else if(1 <= fHPrime && fHPrime < 2) {
        rgb = glm::vec3(fX, fC, 0.0f);
    } else if(2 <= fHPrime && fHPrime < 3) {
        rgb = glm::vec3(0.0f, fC, fX);
    } else if(3 <= fHPrime && fHPrime < 4) {
        rgb = glm::vec3(0.0f, fX, fC);
    } else if(4 <= fHPrime && fHPrime < 5) {
        rgb = glm::vec3(fX, 0.0f, fC);
    } else if(5 <= fHPrime && fHPrime < 6) {
        rgb = glm::vec3(fC, 0.0f, fX);
    } else {
        rgb = glm::vec3(0.0f, 0.0f, 0.0f);
    }
    rgb += glm::vec3(fM, fM, fM);
}

