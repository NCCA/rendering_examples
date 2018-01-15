#include "texscene.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>
#include <ngl/Obj.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <dirent.h>
#include <list>
#include <iostream>
#include <iterator>

TexScene::TexScene() : Scene() {
    // Set the time since we started running the scene
    m_startTime = std::chrono::high_resolution_clock::now();
    m_eye = glm::vec3(0.0, 0.0, 2.0);
    m_target = glm::vec3(0.0, 0.0, 0.0);
}

/**
 * @brief ObjLoaderScene::initGL
 */
void TexScene::initGL() noexcept {
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
    shader->loadShader("TexProgram","shaders/tex_vert.glsl","shaders/tex_frag.glsl");

    // Create a screen oriented plane
    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
    prim->createTrianglePlane("plane",2,2,1,1,ngl::Vec3(0,1,0));

    load3DTex();
}

void TexScene::load3DTex() {
    // Retrieve all the file names and sort them appropriately
    DIR *dir;
    struct dirent *ent;
    std::string suffix(".png");
    int suffixLen = 4;
    std::list<std::string> fileList;
    std::string basepath("./data");
    int nameLen;

    if ((dir = opendir (basepath.c_str())) != NULL) {
      while ((ent = readdir(dir)) != NULL) {
          // Check to see we're dealing with a .png file
          nameLen = strlen(ent->d_name);
          if ((nameLen > suffixLen) || (strncmp(ent->d_name+nameLen-suffixLen, suffix.c_str(), suffixLen) == 0)) {
            // Append the names to a string list
            fileList.push_back(basepath + "/" + std::string(ent->d_name));            
          }
      }
      // Now sort the the list of filenames
      fileList.sort();
      std::copy(fileList.begin(), fileList.end(), std::ostream_iterator<std::string>(std::cerr, "\n"));
      closedir (dir);
    } else {
        /* could not open directory */
        std::cerr << "Error! Files could not be found in "<<basepath << "\n";
        exit(0);
    }

    // Now load the textures into a big 3D texture block
    glGenTextures(1, &m_texBlock);
    glActiveTexture(GL_TEXTURE0);    
    glBindTexture(GL_TEXTURE_3D, m_texBlock);

    // We're going to use the NGL image API to load up the images - the first one will determine width and height
    ngl::Image img(fileList.front());
    m_texWidth = img.width(); m_texHeight = img.height(); m_texDepth = fileList.size();
    std::cerr <<m_texWidth <<"x"<< m_texHeight<<"x"<<m_texDepth<<"\n";

    // This command theoretically makes space for our 3D texture
    glTexImage3D(GL_TEXTURE_3D, // Type of storage
                 0,             // Mipmap levels
                 img.format(),  // Storage format
                 m_texWidth,    // Width of texture 
                 m_texHeight,   // Height of texture 
                 m_texDepth,    // Depth of texture (number of layers)
                 0,             // border
                 GL_RGB,       // internal format
                 GL_UNSIGNED_BYTE, // internal type
                 NULL);            // pointer to data (0 means nothing is copied)

    std::list<std::string>::iterator fit = fileList.begin();
    for (int i=0; i<m_texDepth; ++i, ++fit) {
        std::cerr << "Loading "<<*fit<<"\n"; 
        img.load(*fit);
        glTexSubImage3D(GL_TEXTURE_3D, 
                        0,        // Mipmap level
                        0,        // x offset
                        0,        // y offset
                        i,        // Layer index offset
                        m_texWidth, // Width of the image
                        m_texHeight, // The height of the image
                        1,        // The number of layers to copy across
                        GL_RGB,  // Data format
                        GL_UNSIGNED_BYTE, // Data type
                        img.getPixels()); // The actual image data
    }
    // Set up parameters for our texture
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    
}

void TexScene::paintGL() noexcept {
    // Clear the screen (fill with our glClearColor)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the viewport
    glViewport(0,0,m_width,m_height);

    // Use our shader for this draw
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    (*shader)["TexProgram"]->use();
    GLint pid = shader->getProgramID("TexProgram");

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
    
    // Bind the 3D texture location
    glUniform1i(glGetUniformLocation(pid, "texBlock"), 0);

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
