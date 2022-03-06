#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

struct point_light {
    vec4 position;// ignore w
    vec4 color;// w is intensity
};


layout(set=0, binding=0) uniform global_ubo {
    mat4 projection;
    mat4 view;
    mat4 inv_view_mat;
    vec3 ambient_light_color;
    int num_lights;
    point_light point_lights[10];// value could be dynamically but is hardcoded for now.
} ubo;

layout(push_constant) uniform Push {
    vec4 position;
    vec4 color;
    float radius;

} push;

void main(){
    float alpha = 1.0 - sqrt(dot(fragOffset, fragOffset));
    if (alpha <= 0.0) {
        discard;
    }
    outColor = vec4(push.color.xyz * push.color.w, alpha);
}