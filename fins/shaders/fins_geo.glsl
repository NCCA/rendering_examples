#version 150
#define M_PI 3.1415926535897932384626433832795

// Input is the triangles
layout(triangles) in;

// Each triangle will spawn a maximum strip of 4 verts (e.g. a quad)
layout(triangle_strip, max_vertices = 4) out;

// This is the scaling applied to our fins
uniform float finScale = 0.005;

// Pass the normal data from the vertex shader (note these are in world space rather than normalised device coordinates)
in vec3 normal[];

// The output colour for our fin (in case some fancy blending is required?)
out vec4 finColour;

/** From http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
  * Calculate a 3x3 rotation matrix based on the angle and axis
  */
mat3 rotationMatrix(vec3 axis, float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    return mat3(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c);
}

/** This is where most of the magic happens - it determines the interpolation value based on the z values of the two normals
  * in order to detemrine a normal pointing out orthogonal to the view direction.
  * \param n0,n1 The two input normals
  * \return The output interpolation value (between 0 and 1 if edge is on the silhouette)  
  */
float isSilhouette(in vec3 n0, in vec3 n1) {
    // Trivial case: the normals are the same, so we'll just pick a point in the middle
    if (n1.z == n0.z) {
        return 0.5;
    } else {
        // Use our formula to determine the interpolation value and return a boolean based
        // on whether the interpolant is within the two input vectors
        return - n0.z / (n1.z - n0.z);        
    }
}

void main() {
    // First make a local copy of our transformed vertices
    vec3 pos[3];
    int i;
    for (i = 0; i < 3; ++i)
        pos[i] = gl_in[i].gl_Position.xyz / gl_in[i].gl_Position.w;

    // index of first and second fin vertex
    vec3 finVerts[2];
    vec3 finNorms[2];
    int cnt = 0;
    vec3 t;

    // Iterate over all the edges in our triangle
    int j;
    for (int i = 0; i < 3; ++i) {
        j = (i+1)%3;
        t[cnt] = isSilhouette(normal[i], normal[j]);
        if ((t[cnt] >= 0.0) && (t[cnt] <= 1.0)) {
            finVerts[cnt] = mix(pos[i], pos[j], t[cnt]);
            finNorms[cnt] = normalize(mix(normal[i], normal[j], t[cnt]));
            ++cnt;
        }
    }

    // If count is less than 2 don't do anything as there isn't a fin
    if (cnt >= 2) {
        // Create our triangle strip from the two input vertices and normals        
        finColour = vec4(0,0,0,1); // You might want to visualise something with the color
        gl_Position = vec4(finVerts[0],1.0);
        EmitVertex();
        gl_Position = vec4(finVerts[1],1.0);
        EmitVertex();
        gl_Position = vec4(finVerts[0] + finScale * finNorms[0],1.0);
        EmitVertex();
        gl_Position = vec4(finVerts[1] + finScale * finNorms[1],1.0);
        EmitVertex();

        EndPrimitive();
    }
}
