#ifndef CURVSCENE_H
#define CURVSCENE_H

// The parent class for this scene
#include "scene.h"
#include <ngl/Obj.h>
#include "MultiBufferIndexVAO.h"

class CurvScene : public Scene {
public:
    /// Construct our scene
    CurvScene();

    /// Called when the scene needs to be painted
    void paintGL() noexcept;

    /// Called when the scene is to be initialised
    void initGL() noexcept;

    /// Toggle whether the vectors are visible
    void toggleVectors() {m_vectors = !m_vectors;}

    /// Get / Set methods for the anisotropic parameter
    float getAlphaX() const {return m_alphaX;};
    void setAlphaX(const float &_alphaX) {m_alphaX = _alphaX;};
    float getAlphaY() const {return m_alphaY;};
    void setAlphaY(const float &_alphaY) {m_alphaY = _alphaY;};

private:
    /// Store a unique pointer to the vertex array object to be rendered in our scene
    std::unique_ptr<ngl::AbstractVAO> m_vao;

    /// Build the Vertex Array Object to store all the curvature information
    void buildVAO();

    /// These two parameters set our anisotropic properties on the shader
    float m_alphaX = 1.0f;
    float m_alphaY = 1.0f;

    /// Whether or not the curvature vectors are visible
    bool m_vectors = false;
};

#endif // CURVSCENE_H
