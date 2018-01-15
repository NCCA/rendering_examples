#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec4 GeoPosition[];   // Input fragment position
in vec3 GeoNormal[];     // Input fragment normal
in vec3 GeoK1[];         // Input fragment max curvature
in vec3 GeoK2[];         // Input fragment min curvature

out vec4 FragPosition;   // Input fragment position
out vec3 FragNormal;     // Input fragment normal
out vec3 FragK1;         // Input fragment max curvature
out vec3 FragK2;         // Input fragment min curvature

void main() {
    // The first triangle vertex is just going to be copied across
    gl_Position = gl_in[0].gl_Position;
    FragPosition = GeoPosition[0];
    FragNormal = GeoNormal[0];
    FragK1 = GeoK1[0];
    FragK2 = GeoK2[0];
    EmitVertex();

    for(int i = 1; i < 3;  ++i) {
        // Copy across the vertex position
        gl_Position = gl_in[i].gl_Position;
        FragPosition = GeoPosition[i];
        FragNormal = GeoNormal[i];

        // Determine whether the curvature is flipped, if so, correct the curvature normal
        FragK1 = ((dot(GeoK1[0], GeoK1[i]) < 0.0)?-1.0:1.0) * GeoK1[i];
        FragK2 = ((dot(GeoK2[0], GeoK2[i]) < 0.0)?-1.0:1.0) * GeoK2[i];        

        // Emit the vertex of this primitive
        EmitVertex();
    }

    // Finish the triangle (strip)
    EndPrimitive();
}
