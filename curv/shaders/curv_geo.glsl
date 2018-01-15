#version 330 core
layout(triangles) in;
layout(line_strip, max_vertices = 18) out;

in vec4 normal[];
in vec4 K1[];
in vec4 K2[];


out vec4 perNormalColour;

void main() {
    for(int i = 0; i<gl_in.length(); ++i) {
        // Emit the vertices needed for the normals
        gl_Position = gl_in[i].gl_Position;
        perNormalColour=vec4(1,0,0,1);
        EmitVertex();

        gl_Position = gl_in[i].gl_Position+ 2.0 * normal[i];
        perNormalColour=vec4(1,0,0,1);
        EmitVertex();
        EndPrimitive();

        // Now emit the vertices needed for the maximum curvature direction
        gl_Position = gl_in[i].gl_Position;
        perNormalColour = vec4(0,1,0,1);
        EmitVertex();

        gl_Position = gl_in[i].gl_Position + 10.0 * K1[i];
        perNormalColour = vec4(0,1,0,1);
        EmitVertex();
        EndPrimitive();

        // ... And the vertices needed for the minimum curvature direction
        gl_Position = gl_in[i].gl_Position;
        perNormalColour = vec4(0,0,1,1);
        EmitVertex();

        gl_Position = gl_in[i].gl_Position + 10.0 * K2[i];
        perNormalColour = vec4(0,0,1,1);
        EmitVertex();
        EndPrimitive();
  }
}
