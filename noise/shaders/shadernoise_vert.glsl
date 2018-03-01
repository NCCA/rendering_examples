#version 420                                            // Keeping you on the bleeding edge!
#extension GL_EXT_gpu_shader4 : enable
//#extension GL_ARB_shading_language_420pack: enable    // Use for GLSL versions before 420.

// The modelview and projection matrices are no longer given in OpenGL 4.2
uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 P;
uniform mat3 N; // This is the inverse transpose of the mv matrix

// The vertex position attribute
layout (location=0) in vec3 VertexPosition;

// The texture coordinate attribute
layout (location=1) in vec2 TexCoord;

// The vertex normal attribute
layout (location=2) in vec3 VertexNormal;

// These attributes are passed onto the shader (should they all be smoothed?)
smooth out vec3 WSVertexPosition;
smooth out vec3 WSVertexNormal;
smooth out vec2 WSTexCoord;

void main()
{
    // Transform the vertex normal by the inverse transpose modelview matrix
    WSVertexNormal = normalize(N * VertexNormal);

    // Compute the unprojected vertex position
    WSVertexPosition = vec3(MV * vec4(VertexPosition, 1.0) );

    // Copy across the texture coordinates
    WSTexCoord = TexCoord;

    // Compute the position of the vertex
    gl_Position = MVP * vec4(VertexPosition,1.0);
}
