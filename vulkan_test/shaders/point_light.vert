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

layout(set=0, binding=0) uniform global_ubo {
mat4 projection;
mat4 view;
vec4 ambient_light_color;
vec3 point_light_position;
vec4 point_light_color;
} ubo;

const float LIGHT_RADIUS = 0.1;

void main() 
{

	fragOffset = OFFSETS[gl_VertexIndex];

	vec3 cameraRightWorld = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
	
	vec3 cameraUpWorld = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};

	vec3 positionWorld = ubo.point_light_position.xyz
	+ LIGHT_RADIUS * fragOffset.x * cameraRightWorld
	+ LIGHT_RADIUS * fragOffset.y * cameraUpWorld;

	gl_Position = ubo.projection * ubo.view * uvec4(positionWorld, 1.0);


}