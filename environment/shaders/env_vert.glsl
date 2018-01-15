#version 420

// The vertex position attribute
layout (location=0) in vec3 VertexPosition;

// The texture coordinate attribute
layout (location=1) in vec2 TexCoord;

// The vertex normal attribute
layout (location=2) in vec3 VertexNormal;

// These attributes are passed onto the shader (should they all be smoothed?)
smooth out vec3 FragmentPosition;
smooth out vec3 FragmentNormal;
smooth out vec2 FragmentTexCoord;

uniform mat4 MV;            // model view matrix calculated in the App
uniform mat4 MVP;           // model view projection calculated in the app
uniform mat3 N;             // normal matrix calculated in the app

void main() {
    // Transform the vertex normal by the inverse transpose modelview matrix
    FragmentNormal = normalize(N * VertexNormal);

    // Compute the unprojected vertex position
    FragmentPosition = vec3(MV * vec4(VertexPosition, 1.0) );

    // Copy across the texture coordinates
    FragmentTexCoord = TexCoord;

    // Compute the position of the vertex
    gl_Position = MVP * vec4(VertexPosition,1.0);
}





