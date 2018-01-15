#version 430

// This is passed on from the vertex shader
in vec3 LightIntensity;

// This is no longer a built-in variable
layout (location=0) out vec4 FragColor;

void main() {
    // Set the output color of our current pixel
    FragColor = vec4(LightIntensity,1.0);
}
