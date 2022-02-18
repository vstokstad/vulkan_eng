#version 450


layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragWorldPos;
layout (location = 2) in vec3 fragNormal;
layout (location = 3) in vec2 fragTexCoord;



layout (binding=1) uniform sampler2D texSampler;

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
    mat4 model_matrix;
    mat4 normal_matrix;
} push;

layout (location = 0) out vec4 outColor;

void main() {

    vec3 diffuseLighting = ubo.ambient_light_color.xyz*ubo.ambient_light_color.w;
    vec3 specularLighting = vec3(0.0);
    vec3 surfaceNormal = normalize(fragNormal);

    vec3 camWorldPos = ubo.inv_view_mat[3].xyz;

    for (int i = 0; i < ubo.num_lights; i++) {
        point_light light = ubo.point_lights[i];
        vec3 directionToLight = light.position.xyz - fragWorldPos;
        float attenuation = light.color.w / dot(directionToLight, directionToLight);
        float cosAngIncidence = dot(surfaceNormal, directionToLight);
        cosAngIncidence = clamp(cosAngIncidence, 0, 1);

        // diffuse lighting
        diffuseLighting += light.color.xyz * attenuation * cosAngIncidence;

        // specular lighting
        vec3 viewDirection = normalize(camWorldPos - fragWorldPos);
        vec3 halfAngle = normalize(directionToLight + viewDirection);
        float blinnTerm = dot(surfaceNormal, halfAngle);
        blinnTerm = clamp(blinnTerm, 0, 1);
        blinnTerm = cosAngIncidence != 0.0 ? blinnTerm : 0.0;
        blinnTerm = pow(blinnTerm, 16.0);
        specularLighting += light.color.xyz * attenuation * blinnTerm;
    }
    outColor = vec4((specularLighting + diffuseLighting+ fragColor)*texture(texSampler,fragTexCoord).rgb, 1.0);
}
