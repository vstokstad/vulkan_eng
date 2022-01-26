#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 0 ) out vec4 outColor;

layout(set=0, binding=0) uniform global_ubo {
mat4 projection;
mat4 view;
vec4 ambient_light_color; // w is intensity
vec3 point_light_position;
vec4 point_light_color;
} ubo;

void main(){

	float dis = sqrt(dot(fragOffset,fragOffset));
	if(dis>=1.0){
		discard;
	}

	outColor = vec4(ubo.point_light_color.xyz,1.0);

}