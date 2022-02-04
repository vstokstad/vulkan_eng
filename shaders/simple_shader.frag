#version 450


layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location=3)in vec2 fragTexCoord;
layout (location = 0) out vec4 outColor;

layout (binding=1) uniform sampler2D texSampler;

struct point_light {
    vec4 position;// ignore w
    vec4 color;// w is intensity
};


layout(set=0, binding=0) uniform global_ubo {
    mat4 projection;
    mat4 view;
    vec4 ambient_light_color;
    point_light point_lights[10];// value could be dynamically but is hardcoded for now.
    int num_lights;
} ubo;


layout(push_constant) uniform Push {
    mat4 model_matrix;
    mat4 normal_matrix;
} push;


void main() {

    vec3 diffuse_light = ubo.ambient_light_color.xyz * ubo.ambient_light_color.w;
    vec3 surface_normal = normalize(fragNormalWorld);


    for (int i = 0; i < ubo.num_lights; i++) {
        point_light light = ubo.point_lights[i];

        vec3 direction_to_light = (light.position.xyz - fragPosWorld);

        float attenuation = 1.0 / dot(direction_to_light, direction_to_light);// distance squared.

        float cos_ang_incidence = max(dot(surface_normal, normalize(direction_to_light)), 0);

        vec3 intensity = light.color.xyz * light.color.w * attenuation;

        //spec light


        diffuse_light += intensity*cos_ang_incidence;

    };


    outColor = vec4(diffuse_light * fragColor * texture(texSampler, fragTexCoord).rgb, 1.0);

}
