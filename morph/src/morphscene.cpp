#include "morphscene.h"

// GLM includes
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/spline.hpp>

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

MorphScene::MorphScene() : Scene() {
    m_startTime = std::chrono::high_resolution_clock::now();
}


void MorphScene::initGL() noexcept {
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
    shader->loadShader("MorphProgram",
                       "shaders/morph_vert.glsl",
                       "shaders/morph_frag.glsl");

    initMeshes();
}

void MorphScene::initMeshes() {
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();

    // Register a new VAO factory for our indexed buffer array object
    ngl::VAOFactory::registerVAOCreator("multiBufferIndexVAO", MultiBufferIndexVAO::create);

    typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> MatrixXfr;
    typedef Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> MatrixXir;

    // Read a mesh from a file into igl - note that the faces are all the same
    MatrixXfr V0, V1, V2, V3, V4;
    MatrixXir F;
    igl::read_triangle_mesh("data/face_mesh_neutral.off", V0, F);
    igl::read_triangle_mesh("data/face_mesh_disgust.off", V1, F);
    igl::read_triangle_mesh("data/face_mesh_scared.off", V2, F);
    igl::read_triangle_mesh("data/face_mesh_happy.off", V3, F);
    igl::read_triangle_mesh("data/face_mesh_oh.off", V4, F);

    // Determine our bounding box by finding the min and max corner of the data
    Eigen::Vector3f minCorner = V0.colwise().minCoeff();
    Eigen::Vector3f maxCorner = V0.colwise().maxCoeff();
    Eigen::Vector3f ones(1.0f, 1.0f, 1.0f);

    // Create a 1/(max-min) matrix in one line using Eigen
    float maxScale = ones.cwiseQuotient(maxCorner - minCorner).maxCoeff();

    MatrixXfr scaleMat = (Eigen::Vector3f(maxScale,maxScale,maxScale)).rowwise().replicate(V0.rows()).transpose();
    MatrixXfr minMat = minCorner.rowwise().replicate(V0.rows()).transpose();
    MatrixXfr cntrMat = ((maxCorner-minCorner) * (-0.5f) * maxScale).rowwise().replicate(V0.rows()).transpose();
    // Scale our vertex data so that it fits within 0 and 1
    V0 = (V0 - minMat).cwiseProduct(scaleMat) + cntrMat;
    V1 = (V1 - minMat).cwiseProduct(scaleMat) + cntrMat;
    V2 = (V2 - minMat).cwiseProduct(scaleMat) + cntrMat;
    V3 = (V3 - minMat).cwiseProduct(scaleMat) + cntrMat;
    V4 = (V4 - minMat).cwiseProduct(scaleMat) + cntrMat;

    // Determine the smooth corner vertex normals
    MatrixXfr N0,N1,N2,N3,N4;
    igl::per_vertex_normals(V0,F,N0);
    igl::per_vertex_normals(V1,F,N1);
    igl::per_vertex_normals(V2,F,N2);
    igl::per_vertex_normals(V3,F,N3);
    igl::per_vertex_normals(V4,F,N4);

    // Now concatenate our per vertex data into a big chunk of data in Eigen
    MatrixXfr Vertices(V0.rows(), V0.cols()*5 + N0.cols()*5);
    Vertices << V0, V1-V0, V2-V0, V3-V0, V4-V0, N0, N1, N2, N3, N4;
    int sz = Vertices.cols(); // The total size of a data record

    // Retrieve the data from the vertex matrix as a raw array
    GLuint v_cnt = Vertices.rows() * Vertices.cols();
    GLuint f_cnt = F.rows() * F.cols();

    // create a vao as a series of GL_TRIANGLES
    m_vao = ngl::VAOFactory::createVAO("multiBufferIndexVAO", GL_TRIANGLES);
    m_vao->bind();

    // in this case we are going to set our data as the vertices above
    m_vao->setData(MultiBufferIndexVAO::VertexData(v_cnt * sizeof(float),Vertices.data()[0]));

    // Copy across the face indices
    static_cast<MultiBufferIndexVAO *>( m_vao.get())->setIndices(f_cnt, F.data(), GL_UNSIGNED_INT);

    // Don't know why I need to specify this twice . . .
    m_vao->setNumIndices(f_cnt);

    // Set the attribute pointers for both shaders
    (*shader)["MorphProgram"]->use();

    // Set the vertex attribute pointers for the vertices and normals in one loop
    int i;
    for (i=0;i<10;++i) {
        m_vao->setVertexAttributePointer(i, // GLuint _id
                                         3, // GLint _size
                                         GL_FLOAT, // GLenum _type
                                         sz * sizeof(GLfloat), // GLsizei _stride
                                         i * 3, // unsigned int _dataOffset
                                         (i<5)?false:true); // bool _normalise=false
    }
}

void MorphScene::paintGL() noexcept {
    // Clear the screen (fill with our glClearColor)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the viewport
    glViewport(0,0,m_width,m_height);

    // Use our shader for this draw
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    (*shader)["MorphProgram"]->use();
    GLint pid = shader->getProgramID("MorphProgram");

    // Our MVP matrices
    glm::mat4 M = glm::rotate(glm::mat4(1.0f),
                              -glm::pi<float>() * 0.5f,
                              glm::vec3(0.0,0.0,1.0));
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

    // Set the the weights on the shader to cycle between the input faces
    // We will make the vec4 w cycle between (0,0,0,0)->(1,0,0,0)->(0,1,0,0)->(0,0,1,0)->(0,0,0,1)->(0,0,0,0) based
    // on the elapsed time to give a lame face workout based on a 10 second cycle
    auto now = std::chrono::high_resolution_clock::now();
    double _t = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_startTime).count()  * 0.001;
    double t = fmod(_t, 5.0); // Find our 5 second cycle using modulus

    // Declare our points used for the workout cycle
    glm::vec4 p0(0.0f,0.0f,0.0f,0.0f);
    glm::vec4 p1(1.5f,0.0f,0.0f,0.0f);
    glm::vec4 p2(0.0f,1.5f,0.0f,0.0f);
    glm::vec4 p3(0.0f,0.0f,1.5f,0.0f);
    glm::vec4 p4(0.0f,0.0f,0.0f,1.5f);

    // w will store our resulting weights
    glm::vec4 w;
    switch(int(t)) {
    case 1:
        w = glm::catmullRom(p1,p2,p3,p4, float(t - 1.0));
        break;
    case 2:
        w = glm::catmullRom(p2,p3,p4,p0, float(t - 2.0));
        break;
    case 3:
        w = glm::catmullRom(p3,p4,p0,p1, float(t - 3.0));
        break;
    case 4:
        w = glm::catmullRom(p4,p0,p1,p2, float(t - 4.0));
        break;
    default:
        w = glm::catmullRom(p0,p1,p2,p3, float(t));
        break;
    }

    // Copy our vector over the the GPU
    glUniform4fv(glGetUniformLocation(pid, "w"), 1, glm::value_ptr(w));

    // Draw our buffer
    m_vao->draw();
}
