#version 420                                            // Keeping you on the bleeding edge!
#extension GL_EXT_gpu_shader4 : enable

// Attributes passed on from the vertex shader
smooth in vec3 WSVertexPosition;
smooth in vec3 WSVertexNormal;
smooth in vec2 WSTexCoord;

// Our texture
uniform sampler2D noiseTex;

// This is no longer a built-in variable
out vec4 FragColor;

void main() {
    // Set the output color of our current pixel
    FragColor = vec4(vec3(texture(noiseTex, WSTexCoord).r),1.0);

}
