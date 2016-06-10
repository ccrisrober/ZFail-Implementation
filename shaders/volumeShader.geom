#version 430
layout(triangles_adjacency) in;
layout(triangle_strip, max_vertices = 18) out;

in vec3 outPosition[];
in vec3 outNormal[];

layout(location=45) uniform vec3 lightPosition;
uniform mat4 projection;     // projection matrix

bool faceInLight(vec3 a, vec3 b, vec3 c) {
	vec3 n = cross(b-a, c-a);
	vec3 la = lightPosition.xyz - a;
	vec3 lb = lightPosition.xyz - b;
	vec3 lc = lightPosition.xyz - c;

	return 
		dot(n, la) > 0 ||
		dot(n, lb) > 0 ||
		dot(n, lc) > 0;
}

void createEdgeQuad(vec3 a, vec3 b) {
	gl_Position = projection * vec4(a, 1.0);
	EmitVertex();
	gl_Position = projection * vec4(a - lightPosition.xyz, 0.0);
	EmitVertex();
	gl_Position = projection * vec4(b, 1.0);
	EmitVertex();
	gl_Position = projection * vec4(b - lightPosition.xyz, 0.0);
	EmitVertex();
	EndPrimitive();
}

void main() {
	// Check if triangle is iluminated
	if(faceInLight(outPosition[0], outPosition[2], outPosition[4])) {
		// If adjacent triangle isnt´t iluminated, generate silhouette with edge quad
		if(!faceInLight(outPosition[0], outPosition[1], outPosition[2])) {
			createEdgeQuad(outPosition[0], outPosition[2]);
		}
		if(!faceInLight(outPosition[2], outPosition[3], outPosition[4])) {
			createEdgeQuad(outPosition[2], outPosition[4]);
		}
		if(!faceInLight(outPosition[4], outPosition[5], outPosition[0])) {
			createEdgeQuad(outPosition[4], outPosition[0]);
		}
	}
}