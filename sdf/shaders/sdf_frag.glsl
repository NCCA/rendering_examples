#version 430

uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iTime;                 // shader playback time (in seconds)

uniform vec3 eyepos = vec3(0.0, 0.0, 1.0);  // Eye position
uniform vec3 target = vec3(0.0, 0.0, 0.0);// Target

out vec4 fragColor;                      // Output colour value from this shader

// Ray marching parameters
uniform int marchIter = 64;             // The maximum number of iterations for the ray marching algorithm
uniform int shadowIter = 32;             // The maximum number of shadow iterations permitted
uniform float marchDist = 40.0;          // The maximum distance to ray march
uniform float epsilon = 0.001;          // The error tolerance - lower means slower by better

// Ambient occlusion parameters
uniform int aoIter = 8;                  // The number of iterations for ambient occlusion. Higher is better quality.
uniform float aoDist = 1.0;              // The size of the ambient occlusion kernel
uniform float aoPower = 16.0;             // The exponent for the ao kernel - a larger power increases the fall-off

// Colouring mode (1=black&white, 2=AO visualisation, 3=depth visualisation, 4=lambert shading visualisation, 5=shadow visualisation)
uniform int colourMode = 1;

// Modeling parameters
uniform int isBlending = 1;
uniform float blendRadius = 0.5f;        // The radius for the polynomial min blend function - higher means more blend
const float tau = 6.283185;              // A modeling constant

// Structure for holding light parameters
struct LightInfo {
    vec4 Position; // Light position in eye coords.
    vec3 La; // Ambient light intensity
    vec3 Ld; // Diffuse light intensity
    vec3 Ls; // Specular light intensity
};

// We'll have a single light in the scene with some default values
uniform LightInfo Light = LightInfo(
            vec4(0.0, 10.0, 10.0, 1.0),   // position
            vec3(0.2, 0.2, 0.2),        // La
            vec3(1.0, 1.0, 1.0),        // Ld
            vec3(1.0, 1.0, 1.0)         // Ls
            );

// set important material values
uniform float roughnessValue = 0.1; // 0 : smooth, 1: rough
uniform float F0 = 0.5; // fresnel reflectance at normal incidence
uniform float k = 0.5;  // fraction of diffuse reflection (specular reflection = 1 - k)

// Cosine based palette from http://iquilezles.org/www/articles/palettes/palettes.htm
uniform vec3 ColorPalette[4] = vec3[](vec3(0.5, 0.5, 0.5), vec3(0.5, 0.5, 0.5), vec3(1.0, 1.0, 1.0), vec3(0.00, 0.10, 0.20));
vec3 Palette(in float t) {
    return ColorPalette[0] + ColorPalette[1]*cos( 6.28318*(ColorPalette[2]*t+ColorPalette[3]) );
}

// This determines and stores the shape positions and directions
const int shapeCount = 5;                // The number of shapes to use (needs to be const)
vec3 shapePos[shapeCount];               // Array of shape positions
mat3 shapeDir[shapeCount];               // Array of shape directions

// Allows the user to set the shape type to be rendered. Currently 6 are supported.
uniform int shapeType = 0;             

// The following fixed shape parameters are needed as we support lots of shapes. 
// These could be input parameters if needed
uniform vec3 shape_b = vec3(1.0);         // Box corner
uniform float shape_r = 0.01;             // Rounded box corner radius
uniform vec2 shape_t = vec2(1.0, 0.3);    // torus wide and narrow radii respectively
uniform vec3 shape_c = vec3(1.0);         // cylinder/cone cap radius
uniform vec2 shape_h = vec2(1.0);         // hexagon/triangular prism radius

// A basic function to return the signed distance from the ground plane
float ground(vec3 p) { 
    return p.y; 
}

/** A function to return a shape to render in the scene at point p.
  */
float shape(vec3 p) {
    vec2 q;
    vec3 d;
    switch(shapeType) {
        case 1: // signed exact box
            d = abs(p) - shape_b;
            return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
        case 2: // hexagon
            d = abs(p);
            return max(d.z-shape_h.y,max((d.x*0.866025+d.y*0.5),d.y)-shape_h.x);
        case 3: // torus
            q = vec2(length(p.xz)-shape_t.x,p.y);
            return length(q)-shape_t.y; 
        case 4: // cylinder
            q = abs(vec2(length(p.xz),p.y)) - shape_h;
            return min(max(q.x,q.y),0.0) + length(max(q,0.0));
        case 5: // A fancy trimmed cube (shading artefacts here!)
            return max(length(max(abs(p) - vec3(1.0), 0.0)), length(p) - 1.35);
        default: // A sphere
            return length(p)-1.0;        
    }
}

// polynomial smooth min (k = 0.1) from http://iquilezles.org/www/articles/smin/smin.htm
// the bigger the k, the bigger the region of smoothing
float smin( float a, float b, float k ) {
    float h = clamp( 0.5*(1.0+(b-a)/k), 0.0, 1.0 );
    return mix( b, a, h ) - k*h*(1.0-h);
}

/** The rotation of the shapes was taken from here https://www.shadertoy.com/view/XlXyD4 and forms the basis of
  * the application.
  */
void setShape(float index, out vec3 shapePos, out mat3 shapeDir) {
    float t = tau * mod(index * 0.2 + 0.02 * iTime + 0.12, 1.0);
    float a = 2.0 * t;
    float b = 3.0 * t;
    float c = 7.0 * t;
    shapePos = vec3(1.8 * cos(b),  1.0 + sin(a), 1.8 * cos(c));
    shapeDir = mat3(cos(a), -sin(a), 0.0, sin(a), cos(a), 0.0, 0.0, 0.0, 1.0);
    shapeDir *= mat3(cos(b), 0.0, -sin(b), 0.0, 1.0, 0.0, sin(b), 0.0, cos(b));
    shapeDir *= mat3(cos(c), -sin(c), 0.0, sin(c), cos(c), 0.0, 0.0, 0.0, 1.0);
}

/** This function initialises each shape in the scene. It is modified from https://www.shadertoy.com/view/XlXyD4 
  * to support an arbitrary number of shapes
  */
void setScene() {
    float shapeRatio = 5.0 / float(shapeCount);
    for (int i = 0; i < shapeCount; ++i) {
        setShape(float(i) * shapeRatio, shapePos[i], shapeDir[i]);
    }
}

/** This function returns the distance from a 3D point p to the surface. It is used a lot in
  * the ray marching algorithm, and needs to be as fast as humanly possible.
  */
float scene(vec3 p) {
    float s = ground(p);
    for (int i = 0; i < shapeCount; ++i) {
        if (isBlending == 1) {
            s = smin(s, shape(shapeDir[i] * (p - shapePos[i])), blendRadius);
        } else {
            s = min(s, shape(shapeDir[i] * (p - shapePos[i])));
        }
    }    
    return s;
}

/** I need another version of ths scene function to visualise the distance field on the ground plane.
  */
float sceneWithoutGround(vec3 p) {
    float s = marchDist; // this needs to be bigger than the predicted distance to the scene
    for (int i = 0; i < shapeCount; ++i) {
        if (isBlending == 1) {
            s = smin(s, shape(shapeDir[i] * (p - shapePos[i])), blendRadius);
        } else {
            s = min(s, shape(shapeDir[i] * (p - shapePos[i])));
        }
    }    
    return s;
}

/** This is where the magic happens. The algorithm was taken from https://www.shadertoy.com/view/XlXyD4  but it is 
  * quite generic and was originally taken from (and explained rather well) here:
  * http://jamie-wong.com/2016/07/15/ray-marching-signed-distance-functions/
  */
float march(vec3 eye, vec3 dir) {
    float depth = 0.0;
    for (int i = 0; i < marchIter; ++i) {
        float dist = scene(eye + depth * dir);
        depth += dist;
        if (dist < epsilon || depth >= marchDist)
			break;
    }
    return depth;
}

/** This is an implementation of ambient occlusion method for SDF scenes, 
  * described here: http://iquilezles.org/www/material/nvscene2008/rwwtt.pdf .
  * Note this is considerably faster than the method implemented here: https://www.shadertoy.com/view/XlXyD4
  * but also demonstrates better quality for most shapes tested. Not sure why this other method is used.
  */
float ao(vec3 p, vec3 n) {
    float sum = 0.0;
    float factor = 1.0;
    float depthInc = aoDist / (aoIter+1);
    float depth = depthInc;
    float constK = 0.5; // Not sure what this needs to be, but 0.5 seems to work

    for (int i = 0; i < aoIter; ++i) {
        sum += factor * (depth - scene(p+n*depth)) / depth;
        factor *= 0.5;
        depth += depthInc;
    }
    return 1.0 -  constK * max(sum, 0.0);
}

/** An approximation of a local surface normal at point p based on finite differences. Pretty generic, but this
  * version taken from here https://www.shadertoy.com/view/XlXyD4 .
  * Note this works anywhere in space, not just on the surface.
  */
vec3 normal(vec3 p) {
    return normalize(vec3(
        scene(vec3(p.x + epsilon, p.y, p.z)) - scene(vec3(p.x - epsilon, p.y, p.z)),
        scene(vec3(p.x, p.y + epsilon, p.z)) - scene(vec3(p.x, p.y - epsilon, p.z)),
        scene(vec3(p.x, p.y, p.z + epsilon)) - scene(vec3(p.x, p.y, p.z - epsilon))
    ));
}

/** A generic function to determine the ray to march into the scene based on the fragment coordinates. 
  * Taken from https://www.shadertoy.com/view/XlXyD4 
  */
vec3 ray(float fieldOfView, vec2 size, vec2 fragCoord) {
    vec2 xy = fragCoord - size * 0.5;
    float z = fieldOfView * size.y;
    return normalize(vec3(xy, -z));
}

/** A generic function to determine a view matrix.
  * Taken from https://www.shadertoy.com/view/XlXyD4 
  */
mat3 viewMatrix(vec3 dir, vec3 up) {
    vec3 f = normalize(dir);
    vec3 s = normalize(cross(f, up));
    vec3 u = cross(s, f);
    return mat3(s, u, -f);
}

/** A generic function to determine the rotation matrix necessary to align the view direction with the default ray.
  * Taken from https://www.shadertoy.com/view/XlXyD4 
  
mat3 alignMatrix(vec3 dir) {
    vec3 f = normalize(dir);
    vec3 s = normalize(cross(f, vec3(0.48, 0.6, 0.64)));
    vec3 u = cross(s, f);
    return mat3(u, s, f);
}*/

// From http://www.iquilezles.org/www/articles/rmshadows/rmshadows.htm
float hardShadow( in vec3 p, in vec3 dir, in float maxt) {
    int iter = 0;
    float t=epsilon;
    for(; (t < maxt) && (iter < shadowIter); ++iter) {
        float dist = scene(p + dir*t);
        if( dist < epsilon )
            return 0.0;
        t += dist;
    }
    return 1.0;
}
float softShadow( in vec3 p, in vec3 dir, float maxt) {
    float res = 1.0;
    float t = epsilon;
    float k = 8.0; // This determines the size of the penumbra (bigger is softer)
    int iter = 0;
    for( ; (t < maxt) && (iter < shadowIter); ++iter )
    {
        float h = scene(p + dir*t);
        if( h < epsilon )
            return 0.0;

        res = min( res, (k*h)/t );
        t += h;
    }
    return res;
}

void main() {
    // Determine where the viewer is looking based on the provided eye position and scene target
    vec3 dir = ray(2.5, iResolution.xy, gl_FragCoord.xy);
    mat3 mat = viewMatrix(target - eyepos, vec3(0.0, 1.0, 0.0));
    vec3 eye = eyepos;
    dir = mat * dir;

    // Initialise the scene based on the current elapsed time
    setScene();
    
    // March until it hits the object. The depth indicates how far you have to travel down dir to get to the object.
    float depth = march(eye, dir);
    if (depth >= marchDist - epsilon) {
        fragColor = vec4(depth);
		return;
    }

    // Calculate the intersection point and the associated normal
    vec3 p = eye + depth * dir;
    vec3 n = normal(p);

    // Shadow pass - just shoot a ray to the light
    vec3 lightPos = Light.Position.xyz; // eye
    vec3 pointToLight = lightPos - p;
    float distToLight = length(lightPos - p);
    vec3 pointToEye = eye - p;
    
    // Calculate the light and view vectors s and v respectively, along with the reflection vector
    vec3 s = normalize(pointToLight);
    vec3 v = normalize(pointToEye);
    vec3 r = reflect( -s, n );

    // Precompute the dot products
    vec3 h = normalize(s + v);
    float NdotL = max(dot(n, s), 0.0);
    float NdotV = max(dot(n, v), 0.0);
    float NdotH = max(dot(n, h), 0.0);
    float VdotH = max(dot(v, h), 0.0);
    
    float specular = 0.0;
    if (NdotL > 0.0) {
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

        specular = max(0.0, (fresnel * geoAtt * roughness) / (NdotV * NdotL * 3.14));
    }
    

        // Determine if in shadow
    float shadowFactor = softShadow(p + n*epsilon, s, distToLight);    

    // The result of this function will be a value between 0 and 1 describing how much darker this fragment should be
    float aoFactor = ao(p, n);    

    switch(colourMode) {        
    case 2:
        fragColor = vec4(Palette(aoFactor), 1.0);
        break;
    case 3:
        fragColor = vec4(Palette(sceneWithoutGround(p)),1.0);
        break;
    case 4:
        fragColor = vec4(Palette(NdotL), 1.0);
        break;
    case 5:
        fragColor = vec4(Palette(shadowFactor), 1.0);
        break;    
    default:    
        fragColor = vec4(aoFactor * shadowFactor * (Light.Ld * NdotL + Light.Ls * specular), 1.0);
        break;
    }
}
