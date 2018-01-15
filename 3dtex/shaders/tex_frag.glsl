#version 430

uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iTime;                 // shader playback time (in seconds)
out vec4 fragColor;                      // Output colour value from this shader
uniform sampler3D texBlock;              // A chunk of 3D texture
uniform float cycleLength = 128.0;       // The number of seconds to play through a full cycle

// Cosine based palette from http://iquilezles.org/www/articles/palettes/palettes.htm
uniform vec3 ColorPalette[4] = vec3[](vec3(0.5, 0.5, 0.5), vec3(0.5, 0.5, 0.5), vec3(1.0, 1.0, 1.0), vec3(0.00, 0.10, 0.20));
vec3 Palette(in float t) {
    return ColorPalette[0] + ColorPalette[1]*cos( 6.28318*(ColorPalette[2]*t+ColorPalette[3]) );
}

void main() {
    // Calculate the 3D position in the texBlock based on the time and resolution of the frame
    vec4 texCol = texture(texBlock, vec3(gl_FragCoord.xy / iResolution.xy, mod(iTime,cycleLength)/cycleLength));

    // Determine the colour using our pretty palette cycler
    fragColor = vec4(Palette(texCol.r),1); 
}
