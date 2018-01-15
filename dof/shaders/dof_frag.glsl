#version 430

// The texture to be mapped
uniform sampler2D colourTex;
uniform sampler2D depthTex;

// The depth at which we want to focus
uniform float focalDepth = 0.5;

// A scale factor for the radius of blur
uniform float blurRadius = 0.008;

// The output colour. At location 0 it will be sent to the screen.
layout (location=0) out vec4 fragColor;

// We pass the window size to the shader.
uniform vec2 windowSize;

// The signature for our blur function
subroutine vec4 blurFunctionType(vec2 texpos, float sigma);

// This uniform variable indicates which blur function to use. 0: Gaussian (default), 1: Poisson.
subroutine uniform blurFunctionType blurFunction;

/***************************************************************************************************
 * Gaussian Blur functions and constants
 ***************************************************************************************************/
// Gaussian coefficients
const float G5x5[25] = {0.0035,    0.0123,    0.0210,    0.0123,    0.0035,
                        0.0123,    0.0543,    0.0911,    0.0543,    0.0123,
                        0.0210,    0.0911,    0.2224,    0.0911,    0.0210,
                        0.0123,    0.0543,    0.0911,    0.0543,    0.0123,
                        0.0035,    0.0123,    0.0210,    0.0123,    0.0035};

const float G9x9[81] = {0,         0,    0.0039,    0.0039,    0.0039,    0.0039,    0.0039,         0,         0,
                        0,    0.0039,    0.0078,    0.0117,    0.0117,    0.0117,    0.0078,    0.0039,         0,
                        0.0039,    0.0078,    0.0117,    0.0234,    0.0273,    0.0234,    0.0117,    0.0078,    0.0039,
                        0.0039,    0.0117,    0.0234,    0.0352,    0.0430,    0.0352,    0.0234,    0.0117,    0.0039,
                        0.0039,    0.0117,    0.0273,    0.0430,    0.0469,    0.0430,    0.0273,    0.0117,    0.0039,
                        0.0039,    0.0117,    0.0234,    0.0352,    0.0430,    0.0352,    0.0234,    0.0117,    0.0039,
                        0.0039,    0.0078,    0.0117,    0.0234,    0.0273,    0.0234,    0.0117,    0.0078,    0.0039,
                        0,    0.0039,    0.0078,    0.0117,    0.0117,    0.0117,    0.0078,    0.0039,         0,
                        0,         0,    0.0039,    0.0039,    0.0039,    0.0039,    0.0039,         0,         0};

// These define which Gaussian kernel and the size to use (G5x5 and 5 also possible)
#define SZ 9
#define G  G9x9

// Gaussian filter for regular smooth blur
subroutine (blurFunctionType) vec4 GaussianFilter(vec2 texpos, float sigma) {
    // We need to know the size of half the window
    int HSZ = int(floor(SZ / 2));

    int i,j;
    vec4 colour = vec4(0.0);
    vec2 samplepos;

    // Note that this loops over n^2 values. Is there a more efficient way to do this?
    for (i=0; i<SZ; ++i) {
        for (j=0; j<SZ; ++j) {
            samplepos = texpos + sigma*vec2(float(i)-HSZ, float(j)-HSZ);
            colour += G[i*SZ+j] * texture(colourTex, samplepos);
        }
    }
    return colour;
}

/***************************************************************************************************
 * Poisson Blur functions and constants
 ***************************************************************************************************/
// Tap locations for 13 sample Poisson unit disc
const vec2 PoissonDisc[12] = {vec2(-0.326212,-0.40581),vec2(-0.840144,-0.07358),
                              vec2(-0.695914,0.457137),vec2(-0.203345,0.620716),
                              vec2(0.96234,-0.194983),vec2(0.473434,-0.480026),
                              vec2(0.519456,0.767022),vec2(0.185461,-0.893124),
                              vec2(0.507431,0.064425),vec2(0.89642,0.412458),
                              vec2(-0.32194,-0.932615),vec2(-0.791559,-0.59771)};

// A pseudorandom generator for the Poisson disk rotation
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

// The Poisson filter for irregular blurring
subroutine (blurFunctionType) vec4 PoissonFilter(vec2 texpos, float sigma) {
    int i;
    vec4 colour = texture(colourTex, texpos);
    float angle = rand(texpos);
    mat2 rot = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
    for (i = 0; i < 12; ++i) {
        vec2 samplepos = texpos + 2 * sigma * rot * PoissonDisc[i];
        colour += texture(colourTex, samplepos);
    }
    return colour * 0.076923077; // Same as "/ 13.0"
}

void main() {    
    // Determine the texture coordinate from the window size
    vec2 texpos = gl_FragCoord.xy / windowSize;

    // Determine sigma, the blur radius of this pixel
    float sigma = abs(focalDepth - texture(depthTex, texpos).x) * blurRadius;

    // Now execute the use specified blur function on this pixel based on the depth difference
    fragColor = blurFunction(texpos, sigma);
}

