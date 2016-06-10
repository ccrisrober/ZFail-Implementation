#version 430
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;

out vec3 outPosition;
out vec3 outNormal;

uniform mat4 modelView;
uniform mat3 normal;
uniform mat4 projection;

void main() { 
    outNormal = normal * Normal;
    outPosition = (modelView * vec4(Position,1.0)).xyz;
    gl_Position = projection * modelView * vec4(Position,1.0);
}
