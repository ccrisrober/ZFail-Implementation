#version 430
in vec2 outTexCoord;

uniform sampler2D DiffSpecTex;

out vec4 fragColor;

void main() {
	fragColor = vec4(texture(DiffSpecTex, outTexCoord).rgb, 1.0);
}