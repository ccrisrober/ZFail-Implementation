#version 430
layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoord;

out vec3 outPosition;
out vec3 outNormal;
out vec2 outTexCoord;

uniform mat4 modelView;
uniform mat3 normal;
uniform mat4 projection;

void main() {
	outTexCoord = TexCoord;
	outNormal = normalize( normal * Normal);
	outPosition = vec3( modelView * vec4(Position,1.0) );

	gl_Position = projection * modelView * vec4(Position,1.0);
}
