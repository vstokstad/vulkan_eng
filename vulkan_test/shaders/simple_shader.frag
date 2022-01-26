#version 450



layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColor;


layout(set=0, binding=0) uniform global_ubo {
mat4 projection_view_matrix;
vec4 ambient_light_color;
vec3 point_light_position;
vec4 point_light_color;
} ubo;

layout(push_constant) uniform Push {
mat4 model_matrix; 
mat4 normal_matrix;
} push;


void main() {


  vec3 direction_to_light = ubo.point_light_position - fragPosWorld;

  float attenuation = 1.0 / dot(direction_to_light, direction_to_light); // distance squared.

  vec3 light_color = ubo.point_light_color.xyz * ubo.point_light_color.w * attenuation;
  vec3 ambient_light = ubo.ambient_light_color.xyz * ubo.ambient_light_color.w;
  vec3 diffuse_light = light_color*max(dot(normalize(fragNormalWorld), normalize(direction_to_light)),0);

  outColor = vec4((diffuse_light+ambient_light) * fragColor, 1.0);
   
}
