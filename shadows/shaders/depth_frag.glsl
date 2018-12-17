#version 430

// The output colour. At location 0 it will be sent to the screen.
layout (location=0) out float fragDepth;

void main() {    
    // Write out the depth to the output colour (not really needed)
    fragDepth = gl_FragCoord.z;
}

