#version 420                                            // Keeping you on the bleeding edge!
#extension GL_EXT_gpu_shader4 : enable

// Attributes passed on from the vertex shader
smooth in vec3 FragPosition;
smooth in vec3 WSVertexPosition;
smooth in vec3 WSVertexNormal;
smooth in vec2 WSTexCoord;

// Note - these three colours were yanked from a google image search for "wood grain"
// Feel free to experiment with your own.
uniform vec4 c1 = vec4(208.0/255.0, 176.0/255.0, 41.0/255.0, 1.0);
uniform vec4 c2 = vec4(193.0/255.0, 74.0/255.0, 8.0/255.0, 1.0);
uniform vec4 c3 = vec4(84.0/255.0, 9.0/255.0, 6.0/255.0, 1.0);

// Structure for holding light parameters
struct LightInfo {
    vec4 Position; // Light position in eye coords.
    vec3 La; // Ambient light intensity
    vec3 Ld; // Diffuse light intensity
    vec3 Ls; // Specular light intensity
};

// We'll have a single light in the scene with some default values
uniform LightInfo Light = LightInfo(
            vec4(2.0, 2.0, 10.0, 1.0),   // position
            vec3(0.2, 0.2, 0.2),        // La
            vec3(1.0, 1.0, 1.0),        // Ld
            vec3(1.0, 1.0, 1.0)         // Ls
            );

// The material properties of our object
struct MaterialInfo {
    vec3 Ka; // Ambient reflectivity
    vec3 Kd; // Diffuse reflectivity
    vec3 Ks; // Specular reflectivity
    float Shininess; // Specular shininess factor
};

// The object has a material
uniform MaterialInfo Material = MaterialInfo(
            vec3(0.1, 0.1, 0.1),    // Ka
            vec3(1.0, 1.0, 1.0),    // Kd
            vec3(1.0, 1.0, 1.0),    // Ks
            10.0                    // Shininess
            );

// This is no longer a built-in variable
layout (location=0) out vec4 fragColor;

/// A texture unit for storing the 3D texture
uniform sampler3D woodTex;

/// A constant that determines how much the perturbation of the normal will be
uniform float perturbFactor = 0.01;


/** From http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
  */
mat4 rotationMatrix(vec3 axis, float angle)
{
    //axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

/**
  * Rotate a vector vec by using the rotation that transforms from src to tgt.
  */
vec3 rotateVector(vec3 src, vec3 tgt, vec3 vec) {
    float angle = acos(dot(src,tgt));

    // Check for the case when src and tgt are the same vector, in which case
    // the cross product will be ill defined.
    if (angle == 0.0) {
        return vec;
    }
    vec3 axis = normalize(cross(src,tgt));
    mat4 R = rotationMatrix(axis,angle);

    // Rotate the vec by this rotation matrix
    vec4 _norm = R*vec4(vec,1.0);
    return _norm.xyz / _norm.w;
}

/**
  * Perturb the input normal by a single parametric value using a spiral formula.
  */
vec3 perturbNormalSpiral(vec3 normal, float t) {
    // Use a spiral formula to determine the 2D perturbation applied to our vertex normal
    float a = 0.3;   // Determines overall radius of spiral
    float b = 1000.0; // Determines frequency of spin
    float u = a * t * cos(b * t);
    float v = a * t * sin(b * t);

    // Find the rotation matrix to get from the vector [0,0,1] to our perturbed normal
    return rotateVector(vec3(0.0,0.0,1.0), normalize(vec3(u,v,1.0)), normal);
}

/**
  * Perturn a normal by using an input rotation vector
  */
vec3 perturbNormalVector(vec3 normal, vec3 pvec) {
    return rotateVector(vec3(0.0,0.0,1.0), pvec, normal);
}

/**
  * Compute the first difference around a point based on the surface normal.
  * The parametric formula for a point on the plane can be given by
  * x = (u, v, -(nx/nz)u - (ny/nz)v - (n.p)/nz)
  *   = (u, v, au + bv + c)
  */
vec3 firstDifferenceEstimator(sampler3D tex, vec3 p, vec3 n, float delta) {
    float a = -(n.x/n.z);
    float b = -(n.y/n.z);
    float c = (n.x*p.x+n.y*p.y+n.z*p.z)/n.z;
    float halfdelta = 0.5 * delta;
    float invdelta = 1.0 / delta;

    float u,v;
    u = -halfdelta; v = -halfdelta;
    //float c00 = texture(tex, p + vec3(u,v,a*u+b*v+c)).r;
    float c00 = texture(tex, p + vec3(u,v,0)).r;

    u = -halfdelta; v = halfdelta;
    //float c01 = texture(tex, p + vec3(u,v,a*u+b*v+c)).r;
    float c01 = texture(tex, p + vec3(u,v,0)).r;

    u = halfdelta; v = -halfdelta;
    //float c10 = texture(tex, p + vec3(u,v,a*u+b*v+c)).r;
    float c10 = texture(tex, p + vec3(u,v,0)).r;

    u = halfdelta; v = halfdelta;
    //float c11 = texture(tex, p + vec3(u,v,a*u+b*v+c)).r;
    float c11 = texture(tex, p + vec3(u,v,0)).r;

    return vec3( 0.5*((c10-c00)+(c11-c01))*invdelta,
                 0.5*((c01-c00)+(c11-c10))*invdelta,
                 1.0);
}

void main () {
    // Calculate the normal (this is the expensive bit in Phong)
    vec3 n = normalize( WSVertexNormal );

    // Calculate the light vector
    vec3 s = normalize( vec3(Light.Position) - WSVertexPosition );

    // Calculate the vertex position
    vec3 v = normalize(vec3(-WSVertexPosition));

    // The fragment position will be from -0.5 to 0.5 - we need it in [0,1] for texture lookup
    vec3 fpos = FragPosition + vec3(0.5,0.5,0.5);

    // Retrieve the noise texture from the texture map (do this once)
    vec4 tex = texture(woodTex, fpos).rgba;

    // Calculate the wood colour by using two gradient colour ramps, mixing between
    // c1 and c2 within [0,0.75) and c2 and c3 within [0.75,1].
    float w = tex.r;
    vec4 woodColour;
    if (w < 0.75) {
        woodColour = mix(c1, c2, w * 1.33333);
    } else {
        woodColour = mix(c2, c3, (w - 0.75)*4.0);
    }

    // Now calculate the specular component
    vec3 fd = normalize(vec3(perturbFactor,perturbFactor,1.0) * firstDifferenceEstimator(woodTex, fpos, n, 0.1));

    // Calls our normal perturbation function
    vec3 n1 = perturbNormalVector(n, fd);

    // Reflect the light about the surface normal
    vec3 r = reflect( -s, n1 );

    // Compute the light from the ambient, diffuse and specular components
    vec3 lightColor = (
            Light.La * Material.Ka +
            Light.Ld * Material.Kd * max( dot(s, n1), 0.0 ) +
            Light.Ls * Material.Ks * pow( max( dot(r,v), 0.0 ), Material.Shininess ));

    // Now determine the specular component by
    fragColor = woodColour * vec4(lightColor,1.0) + 0.1*vec4(lightColor,1.0);
}

