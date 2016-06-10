#version 430

in vec3 outPosition;
in vec3 outNormal;
in vec2 outTexCoord;

uniform vec3 lightPosition;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 lightIntensity;

uniform vec3 Color;            // Diffuse reflectivity
uniform vec3 Ka;            // Ambient reflectivity
uniform vec3 Ks;            // Specular reflectivity
uniform float Shininess;    // Specular shininess factor

layout( location = 0 ) out vec3 ambColor;       // TODO: Son necesarios los layouts???
layout( location = 1 ) out vec3 diffSpecColor;  // TODO: Son necesarios los layouts???

const float constant = 1.0;
const float linear = 0.045;
const float quadratic = 0.007;
const float levels = 5.0;

float attenuation;
vec3 diffuse( in vec3 colorTex ) {
    vec3 n = normalize(outNormal);
    if(!gl_FrontFacing) n = -n;

	// Diffuse
    vec3 s = normalize(lightPosition - outPosition);
	float diff = max(dot(s, n), 0.0);
	diff = floor(diff * levels) * (1.0 / levels);
    vec3 diffuse = diff * lightColor;

    // Specular
    vec3 viewDir = normalize(viewPos - outPosition);
    vec3 reflectDir = reflect(-s, n);
    vec3 halfwayDir = normalize(s + viewDir);  
    float spec = pow(max(dot(n, halfwayDir), 0.0), 128.0);
    vec3 specular = spec * lightColor;    

    vec3 color = (diffuse + specular) * colorTex * attenuation;

	return color;
}

vec3 ambient( in vec3 colorTex ) {
	return Color * attenuation;
}

void main() {
	float dist = length(lightPosition - outPosition);
	attenuation = 1.0f / (constant + linear * dist + quadratic * (dist * dist));   
    ambColor = ambient(Color);
    diffSpecColor = diffuse(Color);
}
