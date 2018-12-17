#version 430

/// This is passed on from the vertex shader
in vec3 FragmentNormal;
in vec4 FragmentPosition;
in vec4 ShadowCoord;

/// A uniform variable to hold the depth texture used in shadowing
uniform sampler2D depthTex;

/// This is no longer a built-in variable
layout (location=0) out vec4 FragColor;

/// Tap locations for 13 sample Poisson unit disc
const vec2 PoissonDisc[12] = {vec2(-0.326212,-0.40581),vec2(-0.840144,-0.07358),
                              vec2(-0.695914,0.457137),vec2(-0.203345,0.620716),
                              vec2(0.96234,-0.194983),vec2(0.473434,-0.480026),
                              vec2(0.519456,0.767022),vec2(0.185461,-0.893124),
                              vec2(0.507431,0.064425),vec2(0.89642,0.412458),
                              vec2(-0.32194,-0.932615),vec2(-0.791559,-0.59771)};

/// A pseudorandom generator for the Poisson disk rotation
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

/// A random Poisson filter
vec4 PoissonFilter(vec2 texpos, float sigma) {
    int i;
    vec4 colour = texture(depthTex, texpos);
    float angle = rand(texpos);
    mat2 rot = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
    for (i = 0; i < 12; ++i) {
        vec2 samplepos = texpos + 2 * sigma * rot * PoissonDisc[i];
        colour += texture(depthTex, samplepos);
    }
    return colour * 0.076923077; // Same as "/ 13.0"
}

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


void main() {
    // Transform your input normal
    vec3 n = normalize( FragmentNormal );

    // Calculate the light vector
    vec3 s = normalize( vec3(Light.Position) - FragmentPosition.xyz );

    // Calculate the vertex position
    vec3 v = normalize(-FragmentPosition.xyz);

    // Reflect the light about the surface normal
    vec3 r = reflect( -s, n );

    // Compute shadow
    float visibility = 1.0;
    if ( texture( depthTex, ShadowCoord.xy ).z  <  ShadowCoord.z){
        visibility = 0.5;
    }

    // Compute the light from the ambient, diffuse and specular components
    vec3 LightIntensity = (
            Light.La * Material.Ka +
            visibility * Light.Ld * Material.Kd * max( dot(s, n), 0.0 ) +
            visibility * Light.Ls * Material.Ks * pow( max( dot(r,v), 0.0 ), Material.Shininess ));

    // Set the output color of our current pixel    
    FragColor = vec4(LightIntensity,1.0);
}
