#version 420 core
in vec4 FragPosition;   // Input fragment position
in vec3 FragNormal;     // Input fragment normal
in vec3 FragK1;         // Input fragment max curvature
in vec3 FragK2;         // Input fragment min curvature

// Structure for holding light parameters
struct LightInfo {
    vec4 Position;      // Light position in eye coords.
    vec3 La;            // Ambient light intensity
    vec3 Ld;            // Diffuse light intensity
    vec3 Ls;            // Specular light intensity
};

// The alpha parameter (need to figure out what this does)
uniform float alphaX = 1.0;
uniform float alphaY = 1.0;

// We'll have a single light in the scene with some default values
uniform LightInfo Light = LightInfo(
            vec4(1.0, 1.0, 1.0, 1.0),  // position
            vec3(0.2, 0.2, 0.2),        // La
            vec3(1.0, 1.0, 1.0),        // Ld
            vec3(1.0, 1.0, 1.0)         // Ls
            );

// The material properties of our object
struct MaterialInfo {
    vec3 Ka;            // Ambient reflectivity
    vec3 Kd;            // Diffuse reflectivity
    vec3 Ks;            // Specular reflectivity
    float Shininess;    // Specular shininess factor
};

// The object has a material
uniform MaterialInfo Material = MaterialInfo(
            vec3(0.1, 0.1, 0.1),    // Ka
            vec3(1.0, 1.0, 1.0),    // Kd
            vec3(1.0, 1.0, 1.0),    // Ks
            10.0                    // Shininess
            );

out vec4 fragColor;

void main() {   
   vec3 viewDirection = -FragPosition.xyz;   
   vec3 lightDirection = Light.Position.xyz - FragPosition.xyz;
   float attenuation = 1.0 / length(lightDirection);
   lightDirection = normalize(lightDirection);

   vec3 halfwayVector = normalize(lightDirection + viewDirection);

   vec3 normalDirection = normalize(FragNormal);
   vec3 tangentDirection = normalize(FragK1);
   vec3 binormalDirection = normalize(FragK2); // this also works: cross(normalDirection, tangentDirection);

   float dotLN = dot(lightDirection, normalDirection);
   vec3 diffuseReflection = attenuation * Light.Ld * Material.Kd * max(0.0, dotLN);
   float dotHN = dot(halfwayVector, normalDirection);
   float dotVN = dot(viewDirection, normalDirection);
   float dotHTAlphaX = dot(halfwayVector, tangentDirection) / alphaX;
   float dotHBAlphaY = dot(halfwayVector, binormalDirection) / alphaY;

   vec3 specularReflection;
   if (dotLN < 0.0) // light source on the wrong side?
   {
      specularReflection = vec3(0.0);
   } else {
      // light source on the right side
      specularReflection = attenuation * Material.Ks
         * max(0.0,(dotLN/dotVN))
         * exp(-2.0 * (dotHTAlphaX * dotHTAlphaX + dotHBAlphaY * dotHBAlphaY) / (1.0 + dotHN));
   }
   fragColor = vec4(specularReflection + diffuseReflection, 1.0);
   //fragColor = vec4(binormalDirection,1.0);
   
}
