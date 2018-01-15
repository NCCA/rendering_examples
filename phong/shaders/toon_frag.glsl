#version 420                                            // Keeping you on the bleeding edge!
#extension GL_EXT_gpu_shader4 : enable

// Attributes passed on from the vertex shader
smooth in vec3 WSVertexPosition;
smooth in vec3 WSVertexNormal;
smooth in vec2 WSTexCoord;

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

// Specify the thickness of the rim in range [0,1]
uniform float RimThickness = 0.2;

// This is no longer a built-in variable
out vec4 FragColor;

// This function determines the "buckets" into which different colours evaluate
float clampIntensity(float intensity) {
    if (intensity > 0.9) return 1.0;
    else if (intensity > 0.7) return 0.8;
    else if (intensity > 0.5) return 0.6;
    else if (intensity > 0.3) return 0.4;
    else if (intensity > 0.1) return 0.2;
    else return 0.0;
}

// Advanced: Develop antialiasing!
void main() {
    // Calculate the normal
    vec3 n = normalize( WSVertexNormal );

    // Calculate the light vector
    vec3 s = normalize( vec3(Light.Position) - WSVertexPosition );

    // Calculate the vertex position
    vec3 v = normalize(vec3(-WSVertexPosition));

    // Reflect the light about the surface normal
    vec3 r = reflect( -s, n );

    vec3 lightColor;
    if (abs(dot(v, n)) < RimThickness) {
        lightColor = vec3(0.0,0.0,0.0);
    } else {
        // Compute the light from the ambient, diffuse and specular components
        lightColor = (
                Light.La * Material.Ka +
                Light.Ld * Material.Kd * max( clampIntensity(dot(s, n)), 0.0 ) +
                Light.Ls * Material.Ks * pow( max( clampIntensity(dot(r,v)), 0.0 ), Material.Shininess ));
    }
    FragColor = vec4(lightColor,1.0);
}
