#version 420 core

// Attributes passed on from the vertex shader
smooth in vec3 FragmentPosition;
smooth in vec3 FragmentNormal;
smooth in vec2 FragmentTexCoord;

/// @brief our output fragment colour
layout (location=0) out vec4 FragColour;

// A texture unit for storing the 3D texture
uniform samplerCube envMap;

// Set the maximum environment level of detail (cannot be queried from GLSL apparently)
uniform int envMaxLOD = 8;

// Set our gloss map texture
uniform sampler2D glossMap;

// The inverse View matrix
uniform mat4 invV;

// A toggle allowing you to set it to reflect or refract the light
uniform bool isReflect = true;

// Specify the refractive index for refractions
uniform float refractiveIndex = 1.0;

void main () {
    // Calculate the normal (this is the expensive bit in Phong)
    vec3 n = normalize( FragmentNormal );

    // Calculate the eye vector
    vec3 v = normalize(vec3(-FragmentPosition));

    vec3 lookup;

    if (isReflect) {
        lookup = reflect(v,n);
    } else {
        lookup = refract(v,n,refractiveIndex);
    }

    // The mipmap level is determined by log_2(resolution), so if the texture was 4x4,
    // there would be 8 mipmap levels (128x128,64x64,32x32,16x16,8x8,4x4,2x2,1x1).
    // The LOD parameter can be anything inbetween 0.0 and 8.0 for the purposes of
    // trilinear interpolation.

    // This call actually finds out the current LOD
    float lod = textureQueryLod(envMap, lookup).x;

    // Determine the gloss value from our input texture, and scale it by our LOD resolution
    float gloss = (1.0 - texture(glossMap, FragmentTexCoord*2).r) * float(envMaxLOD);

    // This call determines the current LOD value for the texture map
    vec4 colour = textureLod(envMap, lookup, gloss);

    // This call just retrieves whatever you want from the environment map
    //vec4 colour = texture(envMap, lookup);

    FragColour = colour;
}

