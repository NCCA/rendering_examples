#version 430

// The vertex position attribute
layout (location=0) in vec3 VertexPosition;

// The UV's of the vertex
layout (location=1) in vec2 VertexUV;

// We need an MVP because the plane needs to be rotated
uniform mat4 MVP;

// Pass through the UV coordinates
out vec2 FragmentUV; 

void main() {
    // Pass through the vertex UV's to be interpolated through rasterizations
    FragmentUV = VertexUV;

    // Set the position of the current vertex
    gl_Position = MVP * vec4(VertexPosition, 1.0);
}