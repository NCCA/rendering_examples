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


// set important material values
uniform float roughnessValue = 0.1; // 0 : smooth, 1: rough
uniform float F0 = 0.5; // fresnel reflectance at normal incidence
uniform float k = 0.9;  // fraction of diffuse reflection (specular reflection = 1 - k)


// This is no longer a built-in variable
out vec4 FragColor;

void main() {
    // Calculate the normal
    vec3 n = normalize( WSVertexNormal );

    // Calculate the light vector
    vec3 s = normalize( vec3(Light.Position) - WSVertexPosition );

    // Calculate the view vector
    vec3 v = normalize(vec3(-WSVertexPosition));

    // Reflect the light about the surface normal
    vec3 r = reflect( -s, n );


    // do the lighting calculation for each fragment.
    float NdotL = max(dot(n,s), 0.0);

    float specular = 0.0;
    if(NdotL > 0.0)
    {
        // calculate intermediary values
        vec3 h = normalize(s + v);
        float NdotH = max(dot(n, h), 0.0);
        float NdotV = max(dot(n, v), 0.0); // note: this could also be NdotL, which is the same value
        float VdotH = max(dot(v, h), 0.0);
        float mSquared = roughnessValue * roughnessValue;

        // geometric attenuation
        float NH2 = 2.0 * NdotH;
        float invVdotH = 1.0 / VdotH;
        float g1 = (NH2 * NdotV) * invVdotH;
        float g2 = (NH2 * NdotL) * invVdotH;
        float geoAtt = min(1.0, min(g1, g2));

        // roughness (or: microfacet distribution function)
        // beckmann distribution function
        float r1 = 1.0 / ( 4.0 * mSquared * pow(NdotH, 4.0));
        float r2 = (NdotH * NdotH - 1.0) / (mSquared * NdotH * NdotH);
        float roughness = r1 * exp(r2);

        // fresnel
        // Schlick approximation
        float fresnel = pow(1.0 - VdotH, 5.0);
        fresnel *= (1.0 - F0);
        fresnel += F0;

        specular = (fresnel * geoAtt * roughness) / (NdotV * NdotL * 3.14);
    }
    vec3 finalValue = Light.Ld * NdotL * (k + specular * (1.0 - k));
    FragColor = vec4(finalValue,1.0);
}
