#version 450


layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragWorldPos;
layout (location = 2) in vec3 fragWorldNormal;


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
    mat4 model_matrix;
    mat4 normal_matrix;
} push;

layout (location = 0) out vec4 outColor;

void main() {

    vec3 diffuseLight = ubo.ambient_light_color.xyz;
    vec3 surfaceNormal = normalize(fragWorldNormal);

//SUN SOURCE LIGHT
 //   vec3 directionToLight = normalize(vec3(1,-20,-1) - fragWorldPos);
//10 is sun source multiplier
   //  diffuseLight += diffuseLight * max(dot(surfaceNormal, normalize(directionToLight)), 0);

    for (int i = 0; i < ubo.num_lights; i++) {
        point_light light = ubo.point_lights[i];
        vec3 directionToLight = light.position.xyz - fragWorldPos;
        float attenuation = 1.0 / dot(directionToLight, directionToLight);// distance squared
        float cosAngIncidence = max(dot(surfaceNormal, normalize(directionToLight)), 0);
        vec3 intensity = light.color.xyz * light.color.w * attenuation;

        diffuseLight += intensity * cosAngIncidence;
    }

    outColor = vec4(diffuseLight * fragColor, 1.0);
}
