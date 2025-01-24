#version 410 core

out vec4 out_Color;
in vec2 gsTexCoord;
in vec3 gsNormal;
in vec3 modelPosition; // Interpolated position in world space
flat in int isBillboard;

uniform vec3 cameraPosWS;
uniform float u_time;
uniform float waterLevel;
uniform sampler2D treeTexture;

#define FADE(t) ( t * t * t * ( t * ( t * 6.0 - 15.0 ) + 10.0 ) )
#define LERP(t, a, b) ((a) + (t) * ((b) - (a)))

// Permutation table
const int perm[256] = int[](
    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
    140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247,
    120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177,
    33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165,
    71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211,
    133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25,
    63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
    135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217,
    226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206,
    59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248,
    152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22,
    39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218,
    246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,
    81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157,
    184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93,
    222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
);

// Gradient function for 1D Perlin noise
// Gradient function for 3D Perlin noise
float grad3(int hash, float x, float y, float z) {
    int h = hash & 15;        // Take the lower 4 bits of the hash
    float u = h < 8 ? x : y;  // Select x or y based on the lower 3 bits of h
    float v = h < 4 ? y : (h == 12 || h == 14 ? x : z); // Select y or z
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

// 3D noise function
float pnoise3(float x, float y, float z) {
    // Compute integer coordinates of the grid
    int ix0 = int(floor(x));
    int iy0 = int(floor(y));
    int iz0 = int(floor(z));

    // Compute fractional coordinates within the grid
    float fx0 = x - float(ix0);
    float fy0 = y - float(iy0);
    float fz0 = z - float(iz0);
    float fx1 = fx0 - 1.0;
    float fy1 = fy0 - 1.0;
    float fz1 = fz0 - 1.0;

    // Wrap indices to the permutation table range
    int ix1 = (ix0 + 1) & 0xff;
    int iy1 = (iy0 + 1) & 0xff;
    int iz1 = (iz0 + 1) & 0xff;
    ix0 = ix0 & 0xff;
    iy0 = iy0 & 0xff;
    iz0 = iz0 & 0xff;

    // Apply fade curves to fractional parts
    float r = FADE(fz0);
    float t = FADE(fy0);
    float s = FADE(fx0);

    // Hash coordinates of the 8 cube corners
    float nxy0, nxy1, nx0, nx1, n0, n1;
    nxy0 = grad3(perm[ix0 + perm[iy0 + perm[iz0]]], fx0, fy0, fz0);
    nxy1 = grad3(perm[ix0 + perm[iy0 + perm[iz1]]], fx0, fy0, fz1);
    nx0 = LERP(r, nxy0, nxy1);

    nxy0 = grad3(perm[ix0 + perm[iy1 + perm[iz0]]], fx0, fy1, fz0);
    nxy1 = grad3(perm[ix0 + perm[iy1 + perm[iz1]]], fx0, fy1, fz1);
    nx1 = LERP(r, nxy0, nxy1);

    n0 = LERP(t, nx0, nx1);

    nxy0 = grad3(perm[ix1 + perm[iy0 + perm[iz0]]], fx1, fy0, fz0);
    nxy1 = grad3(perm[ix1 + perm[iy0 + perm[iz1]]], fx1, fy0, fz1);
    nx0 = LERP(r, nxy0, nxy1);

    nxy0 = grad3(perm[ix1 + perm[iy1 + perm[iz0]]], fx1, fy1, fz0);
    nxy1 = grad3(perm[ix1 + perm[iy1 + perm[iz1]]], fx1, fy1, fz1);
    nx1 = LERP(r, nxy0, nxy1);

    n1 = LERP(t, nx0, nx1);

    // Scale the result to the expected range [-1, 1]
    return 0.936 * LERP(s, n0, n1);
}

vec3 random3(vec3 st)
{
    st = vec3(
        dot(st, vec3(127.1, 311.7,  74.7)),
        dot(st, vec3(269.5, 183.3, 246.1)),
        dot(st, vec3(113.5, 271.9, 124.6))
    );
    
    return -1.0 + 2.0 * fract(sin(st) * 43758.5453123);
}

vec3 computeCurl(vec3 p)
{
    float eps = 0.001;

    // Offsets for partials
    vec3 px = vec3(eps, 0.0, 0.0);
    vec3 py = vec3(0.0, eps, 0.0);
    vec3 pz = vec3(0.0, 0.0, eps);

    // Forward/backward samples in x, y, and z directions
    vec3 Fx1 = p * pnoise3(p.x + eps, p.y, p.z);
    vec3 Fx2 = p * pnoise3(p.x - eps, p.y, p.z);
    vec3 Fy1 = p * pnoise3(p.x, p.y + eps, p.z);
    vec3 Fy2 = p * pnoise3(p.x, p.y - eps, p.z);
    vec3 Fz1 = p * pnoise3(p.x, p.y, p.z + eps);
    vec3 Fz2 = p * pnoise3(p.x, p.y, p.z - eps);

    // Finite differences of each component
    vec3 dFx = (Fx1 - Fx2) / (2.0 * eps);
    vec3 dFy = (Fy1 - Fy2) / (2.0 * eps);
    vec3 dFz = (Fz1 - Fz2) / (2.0 * eps);

    // Curl F = ( dFz.y - dFy.z, dFx.z - dFz.x, dFy.x - dFx.y )
    float curlX = dFz.y - dFy.z;
    float curlY = dFx.z - dFz.x;
    float curlZ = dFy.x - dFx.y;

    return vec3(curlX, curlY, curlZ);
}

vec3 rot3(vec3 v, float r)
{
    // Rotate around z-axis by angle r using a 3x3 matrix
    mat3 R = mat3(
         cos(r),  sin(r),  0.0,
        -sin(r),  cos(r),  0.0,
         0.0,     0.0,     1.0
    );
    return R * v;
}

float noiser3(vec3 st, float r)
{
    // Integer cell coords and fractional part
    vec3 i = floor(st);
    vec3 f = fract(st);

    // Perlin "fade" function
    vec3 u = f * f * (3.0 - 2.0 * f);

    // Gradients (rotated) at the 8 corners
    // and their dot with the position offset (f - corner).
    float c000 = dot(rot3(random3(i + vec3(0,0,0)), r), f - vec3(0,0,0));
    float c100 = dot(rot3(random3(i + vec3(1,0,0)), r), f - vec3(1,0,0));
    float c010 = dot(rot3(random3(i + vec3(0,1,0)), r), f - vec3(0,1,0));
    float c110 = dot(rot3(random3(i + vec3(1,1,0)), r), f - vec3(1,1,0));
    float c001 = dot(rot3(random3(i + vec3(0,0,1)), r), f - vec3(0,0,1));
    float c101 = dot(rot3(random3(i + vec3(1,0,1)), r), f - vec3(1,0,1));
    float c011 = dot(rot3(random3(i + vec3(0,1,1)), r), f - vec3(0,1,1));
    float c111 = dot(rot3(random3(i + vec3(1,1,1)), r), f - vec3(1,1,1));

    // Now blend along x, then y, then z
    float c00 = mix(c000, c100, u.x);
    float c01 = mix(c001, c101, u.x);
    float c10 = mix(c010, c110, u.x);
    float c11 = mix(c011, c111, u.x);

    float c0  = mix(c00, c10, u.y);
    float c1  = mix(c01, c11, u.y);

    // Final trilinear interpolation
    return mix(c0, c1, u.z);
}

float fbm(vec3 p, float r){
    float value = 0.0;
    float amplitude = 0.3;
    float frequency = 1.5;
    float octave = 51.0;
    for(int i = 0; i < octave; i++){
        value += amplitude * noiser3(vec3(p.x*frequency, p.y*frequency, p.z*frequency), r);
        frequency *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

void main(void)
{

    if (isBillboard == 1)
    {
        // RENDER TREE BILLBOARD
        vec4 treeColor = texture(treeTexture, gsTexCoord);

        // Discard pixels that are fully transparent
        if (treeColor.a < 0.1) discard;

        out_Color = treeColor;
    }
    else
    {

        float minHeight = waterLevel; // Min height for terrain mapping
        float maxHeight = 1.5; // Max height for terrain mapping

        float height = length(modelPosition); // Distance from origin (center of the sphere)
        float normalizedHeight = (height - minHeight) / (maxHeight - minHeight);

        
        vec3 waterColor = vec3(0.027,0.090,0.180);
        vec3 sandColor = vec3(0.6, 0.55, 0.5);
        vec3 grassColor = vec3(0.18, 0.24, 0.07);
        vec3 grassColor2 = vec3(0.18, 0.25, 0.08);
        vec3 mountainColor = vec3(0.29, 0.29, 0.28);
        vec3 snowColor = vec3(0.9, 0.9, 0.9);
        vec3 finalColor;

        

        if (height < waterLevel) {
            float flowIntensity = fbm(modelPosition * 10.0, u_time*4);

            finalColor = waterColor + vec3(0.5, 0.5, 0.7) * flowIntensity * 0.5;
            out_Color = vec4(finalColor,1.0);
        } else {
            float t = smoothstep(0.6, 1.0, normalizedHeight);

            vec3 curl = computeCurl(modelPosition * 50.0);
            float curlIntensity = length(curl);
            vec3 curlColor = vec3(0.2, 0.2, 0.2) * curlIntensity * 0.005;


            // Adjust thresholds
            float waterToGrass = smoothstep(0, 0.01, normalizedHeight);
            float grassToMountain = smoothstep(0.2, 0.5, normalizedHeight);
            float mountainToSnow = smoothstep(0.8, 1.0, normalizedHeight);

            // vec3 baseColor = mix(waterColor, grassColor, waterToGrass);
            vec3 baseColor = mix(sandColor, grassColor, waterToGrass);
            // vec3 baseColor = grassColor;
            baseColor = mix(baseColor, mountainColor, grassToMountain);
            baseColor = mix(baseColor, snowColor, mountainToSnow);


            // Add curl noise for ruggedness
            finalColor = baseColor - mix(curlColor, baseColor, 0.3) * 0.3;

            out_Color = vec4(finalColor, 1.0);
        }

    }
}


/*
green colors:
vec3(0.18, 0.24, 0.07)
vec3(0.17, 0.22, 0.07)
vec3(0.18, 0.23, 0.06)
vec3(0.16, 0.22, 0.07)
vec3(0.18, 0.25, 0.08)
trunk colors:
vec3(0.25, 0.25, 0.25)
vec3(0.29, 0.29, 0.29)
vec3(0.29, 0.29, 0.28)
vec3(0.27, 0.27, 0.27)
vec3(0.31, 0.31, 0.31)
*/