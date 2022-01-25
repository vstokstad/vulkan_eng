#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push {
mat4 transform; // projection*view*model;
mat4 normal_matrix; //model
} push;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0,-3.0,-1.0));
const float AMBIENT = 0.02;

void main(){
 
    gl_Position = push.transform*vec4(position,1.0);

    //temp: ths is only correct in certain sitations.
    //Only works in uniform scaling.
   // vec3 normalWorldSpace = normalize(mat3(push.model_matrix) * normal);
    
    //correct version but more calculations.
 // vec3 normalWorldSpace = normalize(mat3((push.modelMatrix * vec4(normal,0.0).xyz)
  
  //calculating the inverse in a shader can be expensive and should be avoided//
 // mat3 normal_matrix = transpose(inverse(mat3(push.model_matrix)));
 // vec3 normalWorldSpace = normalize(normal_matrix*normal);

  vec3 normalWorldSpace = normalize(mat3(push.normal_matrix)*normal);

  float lightIntensity = AMBIENT + max(dot(normalWorldSpace, DIRECTION_TO_LIGHT),0);

  fragColor = lightIntensity*color;
}