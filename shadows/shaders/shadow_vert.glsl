#version 430

// The modelview and projection matrices are no longer given in OpenGL 4.2
uniform mat4 MV;
uniform mat4 MVP;
uniform mat3 N; // This is the inverse transpose of the MV matrix

/// Store the depth bias model view projection matrix
uniform mat4 depthBiasMVP;

// The vertex position attribute
layout (location=0) in vec3 VertexPosition;

// The vertex normal attribute
layout (location=1) in vec3 VertexNormal;

// The texture coordinate attribute
layout (location=2) in vec2 TexCoord;

// Passed onto the fragment shader
out vec3 FragmentNormal;
out vec4 FragmentPosition;
out vec4 ShadowCoord;


/************************************************************************************/
void main() {
    // Set the position of the current vertex
    gl_Position = MVP * vec4(VertexPosition, 1.0);
    // Transform the normal
    FragmentNormal = N * VertexNormal;

    // Transform the world space fragment coordinates for shading
    FragmentPosition = MV * vec4(VertexPosition, 1.0);

    // Transform the model space coordinates for shadow coordinate projection
    ShadowCoord = depthBiasMVP * vec4(VertexPosition, 1.0);
}

