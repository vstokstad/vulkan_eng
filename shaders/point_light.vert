#version 450


const vec3 OFFSETS[6] = vec3[](
vec3(-0.5, -0.5, 0.0),
vec3(-0.5, 0.5, 0.0),
vec3(0.5, -0.5, 0.0),
vec3(0.5, -0.5, 0.0),
vec3(-0.5, 0.5, 0.0),
vec3(0.5, 0.5, 0.0)
);

layout (location = 0) out vec2 fragOffset;

struct point_light {
    vec4 position;// ignore w
    vec4 color;// w is intensity
};

layout(set=0, binding=0) uniform global_ubo {
    mat4 projection;
    mat4 view;
    mat4 inv_view_mat;
    vec4 ambient_light_color;
    int num_lights;
    point_light point_lights[10];// value could be dynamically but is hardcoded for now.
} ubo;

layout(push_constant) uniform Push {
    vec4 position;
    vec4 color;
    float radius;

} push;



void main()
{
    vec3 offset =OFFSETS[gl_VertexIndex];
    fragOffset = 2.0*offset.xy;
    vec3 cameraRightWorld = { ubo.view[0][0], ubo.view[1][0], ubo.view[2][0] };
    vec3 cameraUpWorld = { ubo.view[0][1], ubo.view[1][1], ubo.view[2][1] };

    vec3 position_world = push.position.xyz
    + offset.x * 2.0 * push.radius * cameraRightWorld
    + offset.y * 2.0 * push.radius * cameraUpWorld;

    gl_Position = ubo.projection * ubo.view * vec4(position_world, 1.0);

}