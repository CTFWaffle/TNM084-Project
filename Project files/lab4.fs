#version 410 core

out vec4 out_Color;
in vec2 gsTexCoord;
in vec3 gsNormal;

void main(void)
{
	float shade = normalize(gsNormal).z; // Fake light
//	out_Color = vec4(gsTexCoord.s, gsTexCoord.t, 0.0, 1.0);
	out_Color = vec4(gsNormal.x, gsNormal.y, gsNormal.z, 1.0);
	//out_Color = vec4(shade, shade, shade, 1.0);
}

