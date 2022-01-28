#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;

struct point_light {
vec4 position; // ignore w
vec4 color; // w is intensity
};

layout(set=0, binding=0) uniform global_ubo {
	mat4 projection;
	mat4 view;
	vec4 ambient_light_color;
	point_light point_lights[10]; // value could be dynamically but is hardcoded for now.
	int num_lights;
} ubo;


layout(push_constant) uniform Push {
mat4 model_matrix; 
mat4 normal_matrix;
} push;


void main(){
 
  vec4 position_world = push.model_matrix * vec4(position,1.0);

  gl_Position = ubo.projection * ubo.view * position_world;

  fragNormalWorld = normalize(mat3(push.normal_matrix) * normal);
  fragPosWorld = position_world.xyz;
  fragColor = color;


}