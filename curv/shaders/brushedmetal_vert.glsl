#version 420 core
layout (location = 0) in vec3 VertPos;    // The input vertex position
layout (location = 1) in vec2 VertUV;     // The input UV coordinate
layout (location = 2) in vec3 VertNormal; // The input normal
layout (location = 3) in vec3 VertK1;     // The input maximum principle curvature
layout (location = 4) in vec3 VertK2;     // The input minimum principle curvature

uniform mat4 MVP;                         // The Model View Projection matrix
uniform mat4 MV;                          // The Model View matrix
uniform mat3 N;                           // The inverse transpose of the MV matrix

out vec4 GeoPosition;                    // The output position
out vec3 GeoNormal;                      // The output normal
out vec3 GeoK1;                          // The output minimum curvature
out vec3 GeoK2;                          // The output maximum curvature

void main(void)
{
    // The transformed position, required for rasterization
    gl_Position = MVP*vec4(VertPos,1.0);

    // Passing on the unprojected vertex for shading
    GeoPosition = MV * vec4(VertPos, 1.0);

    // The transformed normal and principle curvature directions
    GeoNormal = N * VertNormal;
    GeoK1 = N * VertK1;
    GeoK2 = N * VertK2;
}
