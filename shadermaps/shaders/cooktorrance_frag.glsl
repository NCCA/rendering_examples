#version 420                                            // Keeping you on the bleeding edge!
#extension GL_EXT_gpu_shader4 : enable

// Attributes passed on from the vertex shader
smooth in vec3 FragmentPosition;
smooth in vec3 FragmentNormal;
smooth in vec2 FragmentTexCoord;

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

// An input texture to add a material diffuse colour for the fragment
uniform sampler2D ColourTexture;

// An input texture for normals for bump mapping
uniform sampler2D NormalTexture;

// An input texture for the scaling of the importance of the specular term
uniform sampler2D SpecularTexture;

// An input texture for gloss - I'm going to repurpose this for the roughness maps
uniform sampler2D GlossTexture;

// An input texture for our environment light sources
uniform samplerCube EnvironmentTexture;

// The number of mipmap levels in our environment texture
uniform int EnvironmentLOD = 8;

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

// set important material values
uniform float F0 = 0.5; // fresnel reflectance at normal incidence

// This is no longer a built-in variable
out vec4 FragColor;

void main() {
    // Calculate the normal
    vec3 nt = normalize( FragmentNormal );

    // Keep a constant for scaling the texture coordinates
    float texScale = 8.0;

    // Retrieve a roughness value from the gloss texture
    float roughnessValue = texture(GlossTexture,FragmentTexCoord*texScale).r;

    // Retrieve the proportion of the reflection will be specular
    float k = texture(SpecularTexture, FragmentTexCoord * texScale).r;

    // Extract the normal from the normal map (rescale to [-1,1]
    vec3 tgt = normalize(texture(NormalTexture, FragmentTexCoord*texScale).rgb * 2.0 - 1.0);

    // The source is just up in the Z-direction
    vec3 src = vec3(0.0, 0.0, 1.0);

    // Perturb the normal according to the target
    vec3 n = rotateVector(src, tgt, nt);   

    // Calculate the light vector
    vec3 s = normalize( vec3(Light.Position) - FragmentPosition );

    // Calculate the view vector
    vec3 v = normalize(vec3(-FragmentPosition));

    // Reflect the light about the surface normal
    vec3 r = reflect( -s, n );

    // Determine our light contribution from the environment map using the roughness as the level
    // The line below originates from
    // http://casual-effects.blogspot.co.uk/2011/08/plausible-environment-lighting-in-two.html
    float EnvironmentMapWidth = 2 << EnvironmentLOD;
    float MIPlevel = log2(EnvironmentMapWidth * sqrt(3)) - 0.5 * log2(roughnessValue + 1);
    //MIPlevel = roughnessValue * 8.0;
    vec4 specLightColour = textureLod(EnvironmentTexture, r, MIPlevel).rgba;
    vec4 diffLightColour = textureLod(EnvironmentTexture, n, MIPlevel).rgba;

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
    // Use the light colour from the environment map here
    vec4 finalValue = diffLightColour * NdotL + specular * specLightColour * k;

    FragColor = texture(ColourTexture,FragmentTexCoord*texScale).rgba * finalValue;
}
