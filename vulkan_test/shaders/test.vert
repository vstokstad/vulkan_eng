#version 450
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec3 a_Color;
layout(location = 2) in vec2 u_Offset;
layout(location = 3) in float u_Time;


layout(location = 0) out vec3 f_Color;

void main()
{
	vec2 position;
	position.x = a_Position.x * cos(u_Time) - a_Position.y * sin(u_Time);
	position.y = a_Position.x * sin(u_Time) + a_Position.y * cos(u_Time);

	gl_Position = vec4(position + u_Offset, 0.0f, 1.0f);
	f_Color = a_Color;
}
