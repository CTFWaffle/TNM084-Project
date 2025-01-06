#version 410 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
in vec2 teTexCoord[3];
in vec3 teNormal[3];
out vec2 gsTexCoord;
out vec3 gsNormal;
uniform sampler2D tex;

uniform mat4 projMatrix;
uniform mat4 mdlMatrix;
uniform mat4 camMatrix;

uniform float disp;
uniform int texon;

vec2 random2(vec2 st)
{
    st = vec2( dot(st,vec2(127.1,311.7)),
              dot(st,vec2(269.5,183.3)) );
    return -1.0 + 2.0*fract(sin(st)*43758.5453123);
}

// Gradient Noise by Inigo Quilez - iq/2013
// https://www.shadertoy.com/view/XdXGW8
float noise(vec2 st)
{
    vec2 i = floor(st);
    vec2 f = fract(st);

    vec2 u = f*f*(3.0-2.0*f);

    return mix( mix( dot( random2(i + vec2(0.0,0.0) ), f - vec2(0.0,0.0) ),
                     dot( random2(i + vec2(1.0,0.0) ), f - vec2(1.0,0.0) ), u.x),
                mix( dot( random2(i + vec2(0.0,1.0) ), f - vec2(0.0,1.0) ),
                     dot( random2(i + vec2(1.0,1.0) ), f - vec2(1.0,1.0) ), u.x), u.y);
}

vec3 random3(vec3 st)
{
    st = vec3( dot(st,vec3(127.1,311.7, 543.21)),
              dot(st,vec3(269.5,183.3, 355.23)),
              dot(st,vec3(846.34,364.45, 123.65)) ); // Haphazard additional numbers by IR
    return -1.0 + 2.0*fract(sin(st)*43758.5453123);
}

// Gradient Noise by Inigo Quilez - iq/2013
// https://www.shadertoy.com/view/XdXGW8
// Trivially extended to 3D by Ingemar
float noise(vec3 st)
{
    vec3 i = floor(st);
    vec3 f = fract(st);

    vec3 u = f*f*(3.0-2.0*f);

    return mix(
    			mix( mix( dot( random3(i + vec3(0.0,0.0,0.0) ), f - vec3(0.0,0.0,0.0) ),
                     dot( random3(i + vec3(1.0,0.0,0.0) ), f - vec3(1.0,0.0,0.0) ), u.x),
                mix( dot( random3(i + vec3(0.0,1.0,0.0) ), f - vec3(0.0,1.0,0.0) ),
                     dot( random3(i + vec3(1.0,1.0,0.0) ), f - vec3(1.0,1.0,0.0) ), u.x), u.y),

    			mix( mix( dot( random3(i + vec3(0.0,0.0,1.0) ), f - vec3(0.0,0.0,1.0) ),
                     dot( random3(i + vec3(1.0,0.0,1.0) ), f - vec3(1.0,0.0,1.0) ), u.x),
                mix( dot( random3(i + vec3(0.0,1.0,1.0) ), f - vec3(0.0,1.0,1.0) ),
                     dot( random3(i + vec3(1.0,1.0,1.0) ), f - vec3(1.0,1.0,1.0) ), u.x), u.y), u.z

          	);
}

vec3 fbm(vec3 p){
    float value = 0.0;
    float amplitude = 0.3;
    float frequency = 1.5;
    float octave = 51.0;
    for(int i = 0; i < octave; i++){
        value += amplitude * noise(p*frequency);
        frequency *= 2.0;
        amplitude *= 0.5;
    }
    return p*value;
}

void computeVertex(int nr)
{
    // Get the vertex position in object space
    vec3 p = vec3(gl_in[nr].gl_Position);

    // Define two tangent directions for the plane
    vec3 tangent1 = normalize(cross(p, vec3(0.0, 1.0, 0.0))); // Arbitrary direction 1
    vec3 tangent2 = normalize(cross(p,tangent1)); // Arbitrary direction 2

    // Sample points close to `p` along tangent directions
    float epsilon = 0.01; // Small step size for finite difference
    vec3 p1 = p + epsilon * tangent1;
    vec3 p2 = p + epsilon * tangent2;
    vec3 p3 = p - epsilon * tangent1 - epsilon * tangent2;

    // Apply displacement (or deformation) to these points
    p += fbm(p);      // Displacement for the current vertex
    p1 += fbm(p1);    // Displacement for the first tangent point
    p2 += fbm(p2);    // Displacement for the second tangent point
    p3 += fbm(p3);

    // Compute the normal using the cross product of the displaced tangent vectors
    vec3 edge1 = p1 - p3;
    vec3 edge2 = p2 - p3;
    vec3 normal = normalize(cross(edge1, edge2));

    // Transform the vertex position
    gl_Position = projMatrix * camMatrix * mdlMatrix * vec4(p, 1.0);

    // Pass texture coordinates (interpolated)
    gsTexCoord = teTexCoord[nr];

    // Transform and output the normal
    gsNormal = normalize(mat3(camMatrix * mdlMatrix) * normal);

    EmitVertex();
}

void main()
{
    computeVertex(0);
    computeVertex(1);
    computeVertex(2);
    EndPrimitive();
}

