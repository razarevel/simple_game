layout(push_constant) uniform PushConstants {
	mat4 view;
	mat4 proj;
	vec4 cameraPos;
	uint texCube;
}pc;

struct PerVertex {
	vec2 uv;
	vec3 worldNormal;
	vec3 worldPos;
};
