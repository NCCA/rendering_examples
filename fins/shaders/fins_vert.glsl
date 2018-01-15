#version 330 core
/// @brief the vertex passed in
layout (location = 0) in vec3 inVert;
/// @brief the in uv
layout (location = 1) in vec2 inUV;
/// @brief the normal passed in
layout (location = 2) in vec3 inNormal;

uniform mat4 MVP;
uniform mat3 N;

out vec3 normal;

void main(void)
{
    // Transform the output vertex position
    gl_Position = MVP * vec4(inVert,1.0f);

    // Transform the normal for the geometry shader
    normal = N * inNormal;
}

