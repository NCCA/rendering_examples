#include "curvscene.h"

// The headers below are needed to get matrices from GLM
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

// The NGL includes are needed for the creation of VAO's from geometry
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/VAOFactory.h>
#include <ngl/ShaderLib.h>

// The headers below are required for the maths calculations on the geometry
#include <igl/per_vertex_normals.h>
#include <igl/principal_curvature.h>
#include <igl/read_triangle_mesh.h>
#include <Eigen/Core>
#include <Eigen/Dense>

/**
 * @brief DofScene::DofScene
 */
CurvScene::CurvScene() : Scene() {
}


/**
 * @brief DofScene::initGL
 */
void CurvScene::initGL() noexcept {
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
    shader->loadShader("GouraudProgram",                        // Name of program
                       "../common/shaders/gouraud_vert.glsl",   // Vertex shader
                       "../common/shaders/gouraud_frag.glsl");  // Fragment shader

    shader->loadShader("BrushedMetalProgram",
                       "shaders/brushedmetal_vert.glsl",
                       "shaders/brushedmetal_frag.glsl");


    // Build the brushed metal shader the old fashioned way as we have an additional pipeline stage
    shader->createShaderProgram("BrushedMetal");

    shader->attachShader("BrushedMetalVertex",ngl::ShaderType::VERTEX);
    shader->loadShaderSource("BrushedMetalVertex","shaders/brushedmetal_vert.glsl");

    shader->attachShader("BrushedMetalFragment",ngl::ShaderType::FRAGMENT);
    shader->loadShaderSource("BrushedMetalFragment","shaders/brushedmetal_frag.glsl");

    shader->attachShader("BrushedMetalGeometry",ngl::ShaderType::GEOMETRY);
    shader->loadShaderSource("BrushedMetalGeometry","shaders/brushedmetal_geo.glsl");

    shader->compileShader("BrushedMetalVertex");
    shader->compileShader("BrushedMetalFragment");
    shader->compileShader("BrushedMetalGeometry");
    shader->attachShaderToProgram("BrushedMetal","BrushedMetalVertex");
    shader->attachShaderToProgram("BrushedMetal","BrushedMetalFragment");
    shader->attachShaderToProgram("BrushedMetal","BrushedMetalGeometry");
    shader->linkProgramObject("BrushedMetal");

    // Build the curvature shader the old fashioned way as we have an additional pipeline stage
    shader->createShaderProgram("Curvature");

    shader->attachShader("CurvVertex",ngl::ShaderType::VERTEX);
    shader->loadShaderSource("CurvVertex","shaders/curv_vert.glsl");

    shader->attachShader("CurvFragment",ngl::ShaderType::FRAGMENT);
    shader->loadShaderSource("CurvFragment","shaders/curv_frag.glsl");

    shader->attachShader("CurvGeometry",ngl::ShaderType::GEOMETRY);
    shader->loadShaderSource("CurvGeometry","shaders/curv_geo.glsl");

    shader->compileShader("CurvVertex");
    shader->compileShader("CurvFragment");
    shader->compileShader("CurvGeometry");
    shader->attachShaderToProgram("Curvature","CurvVertex");
    shader->attachShaderToProgram("Curvature","CurvFragment");
    shader->attachShaderToProgram("Curvature","CurvGeometry");
    shader->linkProgramObject("Curvature");

    // Create the Vertex Array Object. This loads the geometry and calculates the curvature properties of the mesh.
    buildVAO();
}

/**
 * @brief NGLScene::buildVAO
 * This function creates the Vertex Array Object from an input file and copies the data to the GPU,
 * ready for rendering.
 */
void CurvScene::buildVAO() {
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();

    // Register a new VAO factory for our indexed buffer array object
    ngl::VAOFactory::registerVAOCreator("multiBufferIndexVAO", MultiBufferIndexVAO::create);

    // Currently the filename is hardcoded (sorry)
    //std::string filename = "../common/models/bust.off";
    std::string filename = "../common/models/fertility.off";

    typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> MatrixXfr;
    typedef Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> MatrixXir;

    // Read a mesh from a file into igl
    MatrixXfr V;
    MatrixXir F;
    igl::read_triangle_mesh(filename, V, F);

    // Determine the smooth corner normals
    MatrixXfr N;
    igl::per_vertex_normals(V,F,N);

    // Compute the principle curvature directions and magnitude using quadric fitting
    MatrixXfr K1,K2;
    Eigen::VectorXf KV1,KV2;
    igl::principal_curvature(V,F,K1,K2,KV1,KV2);
    MatrixXfr KV1_mat(K1.rows(), K1.cols()); KV1_mat << KV1,KV1,KV1;
    MatrixXfr KV2_mat(K2.rows(), K2.cols()); KV2_mat << KV2,KV2,KV2;
    K1.array() = KV1_mat.array() * K1.array();
    K2.array() = KV2_mat.array() * K2.array();

    // Now concatenate our per vertex data into a big chunk of data in Eigen
    MatrixXfr Vertices(V.rows(), V.cols() + N.cols() + K1.cols() + K2.cols());
    Vertices << V, N, K1, K2;

    // Retrieve the data from the vertex matrix as a raw array
    GLuint v_cnt = Vertices.rows() * Vertices.cols();
    GLuint f_cnt = F.rows() * F.cols();

    // create a vao as a series of GL_TRIANGLES
    m_vao.reset(static_cast<MultiBufferIndexVAO *>(ngl::VAOFactory::createVAO("multiBufferIndexVAO", GL_TRIANGLES)) );
    m_vao->bind();

    // in this case we are going to set our data as the vertices above
    m_vao->setData(MultiBufferIndexVAO::VertexData(v_cnt * sizeof(float),Vertices.data()[0]));

    // Copy across the face indices
    m_vao->setIndices(f_cnt, F.data(), GL_UNSIGNED_INT);

    // Don't know why I need to specify this twice . . .
    m_vao->setNumIndices(f_cnt);

    // Set the attribute pointers for both shaders
    (*shader)["GouraudProgram"]->use();
    // Set the vertex attribute pointer
    m_vao->setVertexAttributePointer(0, // GLuint _id
                                     3, // GLint _size
                                     GL_FLOAT, // GLenum _type
                                     12 * sizeof(GLfloat), // GLsizei _stride
                                     0, // unsigned int _dataOffset
                                     false); // bool _normalise=false

    // Set the normal attribute pointer
    m_vao->setVertexAttributePointer(2, // GLuint _id
                                     3, // GLint _size
                                     GL_FLOAT, // GLenum _type
                                     12 * sizeof(GLfloat), // GLsizei _stride
                                     3, // unsigned int _dataOffset
                                     true); // bool _normalise=false

    (*shader)["Curvature"]->use();
    // Set the vertex attribute pointer
    m_vao->setVertexAttributePointer(0, // GLuint _id
                                     3, // GLint _size
                                     GL_FLOAT, // GLenum _type
                                     12 * sizeof(GLfloat), // GLsizei _stride
                                     0, // unsigned int _dataOffset
                                     false); // bool _normalise=false
    // Set the normal attribute pointer
    m_vao->setVertexAttributePointer(2, // GLuint _id
                                     3, // GLint _size
                                     GL_FLOAT, // GLenum _type
                                     12 * sizeof(GLfloat), // GLsizei _stride
                                     3, // unsigned int _dataOffset
                                     true); // bool _normalise=false
    // Set the K1 attribute pointer (disable normalise for magnitude visualisation)
    m_vao->setVertexAttributePointer(3, // GLuint _id
                                     3, // GLint _size
                                     GL_FLOAT, // GLenum _type
                                     12 * sizeof(GLfloat), // GLsizei _stride
                                     6, // unsigned int _dataOffset
                                     true); // bool _normalise=false

    // Set the K2 attribute pointer (disable normalise for magnitude visualisation)
    m_vao->setVertexAttributePointer(4, // GLuint _id
                                     3, // GLint _size
                                     GL_FLOAT, // GLenum _type
                                     12 * sizeof(GLfloat), // GLsizei _stride
                                     9, // unsigned int _dataOffset
                                     true); // bool _normalise=false

    // Set the attribute pointers for the Brushed Metal shader
    (*shader)["BrushedMetalProgram"]->use();
    // Set the vertex attribute pointer
    m_vao->setVertexAttributePointer(0, // GLuint _id
                                     3, // GLint _size
                                     GL_FLOAT, // GLenum _type
                                     12 * sizeof(GLfloat), // GLsizei _stride
                                     0, // unsigned int _dataOffset
                                     false); // bool _normalise=false

    // Set the normal attribute pointer
    m_vao->setVertexAttributePointer(2, // GLuint _id
                                     3, // GLint _size
                                     GL_FLOAT, // GLenum _type
                                     12 * sizeof(GLfloat), // GLsizei _stride
                                     3, // unsigned int _dataOffset
                                     true); // bool _normalise=false

    // Set the K1 attribute pointer (disable normalise for magnitude visualisation)
    m_vao->setVertexAttributePointer(3, // GLuint _id
                                     3, // GLint _size
                                     GL_FLOAT, // GLenum _type
                                     12 * sizeof(GLfloat), // GLsizei _stride
                                     6, // unsigned int _dataOffset
                                     true); // bool _normalise=false

    // Set the K2 attribute pointer (disable normalise for magnitude visualisation)
    m_vao->setVertexAttributePointer(4, // GLuint _id
                                     3, // GLint _size
                                     GL_FLOAT, // GLenum _type
                                     12 * sizeof(GLfloat), // GLsizei _stride
                                     9, // unsigned int _dataOffset
                                     true); // bool _normalise=false

    // now unbind
//    m_vao->unbind();
}


void CurvScene::paintGL() noexcept {
    // Clear the screen (fill with our glClearColor)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the viewport
    glViewport(0,0,m_width,m_height);

    // Use our shader for this draw
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();

    // Our MVP matrices
    glm::mat4 M = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f));
    glm::mat4 MVP, MV;
    glm::mat3 N;

    // Note the matrix multiplication order as we are in COLUMN MAJOR storage
    MVP = m_P * m_V * M;
    MV = m_V * M;
    N = glm::inverse(glm::mat3(MV));

    (*shader)["BrushedMetal"]->use();
    GLint pid = shader->getProgramID("BrushedMetal");
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
    
    // The shading parameters
    glUniform1f(glGetUniformLocation(pid, "alphaX"), m_alphaX);
    glUniform1f(glGetUniformLocation(pid, "alphaY"), m_alphaY);

    m_vao->draw();

    // Now draw with the curvature vectors displayed
    if (m_vectors) {
        (*shader)["Curvature"]->use();
        pid = shader->getProgramID("Curvature");
        // Set this MVP on the GPU
        glUniformMatrix4fv(glGetUniformLocation(pid, "MVP"), //location of uniform
                        1, // how many matrices to transfer
                        false, // whether to transpose matrix
                        glm::value_ptr(MVP)); // a raw pointer to the data
        m_vao->draw();
    }
}
