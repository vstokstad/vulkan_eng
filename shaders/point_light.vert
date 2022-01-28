#version 450


const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

layout (location = 0) out vec2 fragOffset;

struct point_light {
	vec4 position; // ignore w
	vec4 color; // w is intensity
};


layout(set=0, binding=0) uniform global_ubo {
	mat4 projection;
	mat4 view;
	vec4 ambient_light_color;
	vec4 camera_position; // ignore w
	point_light point_lights[10]; // value could be dynamically but is hardcoded for now.
	int num_lights;
} ubo;

layout(push_constant) uniform Push {
	vec4 position;
	vec4 color;
	float radius;

} push;



void main() 
{

	fragOffset = OFFSETS[gl_VertexIndex];
	vec3 cameraRightWorld = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
	vec3 cameraUpWorld = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};

	vec3 position_world = push.position.xyz
	+ push.radius * fragOffset.x * cameraRightWorld
	+ push.radius * fragOffset.y * cameraUpWorld;

	gl_Position = ubo.projection * ubo.view * vec4(position_world, 1.0);

}