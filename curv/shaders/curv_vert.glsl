#version 330 core
/// @brief the vertex passed in
layout (location = 0) in vec3 inVert;
/// @brief the in uv
layout (location = 1) in vec2 inUV;
/// @brief the normal passed in
layout (location = 2) in vec3 inNormal;

layout (location = 3) in vec3 inK1;

layout (location = 4) in vec3 inK2;

uniform mat4 MVP;
out vec4 normal;
out vec4 K1;
out vec4 K2;

void main(void)
{
        gl_Position = MVP*vec4(inVert,1);
        normal=MVP*vec4(inNormal,0);
        K1=MVP*vec4(inK1,0);
        K2=MVP*vec4(inK2,0);
}

