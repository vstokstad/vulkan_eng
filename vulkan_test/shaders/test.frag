#version 450
layout (location=0) in vec3 f_Color;
layout (location=1) out vec4 o_Color;

void main()
{
	o_Color = vec4(f_Color, 1.0f);
}