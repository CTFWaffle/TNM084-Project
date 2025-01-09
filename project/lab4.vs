#version 410 core

// Mostly pass-through but with many attributes
in vec3 in_Position;
in vec2 in_TexCoord;
in vec3 in_Normal;
out vec4 vPosition;
out vec2 vTexCoord;
out vec3 vNormal;

void main()
{
// We will not do transformations here since we want the
// tesselation and geometry shaders to work in model coordinates.
    vPosition = vec4(in_Position, 1.0);
    vTexCoord = in_TexCoord;
    vNormal = in_Normal;
}
