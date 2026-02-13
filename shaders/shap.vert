#version 460 core

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout: require

struct Vertex {
		float x,y,z, face;
};

layout(buffer_reference, scalar) readonly buffer Vertices{
		Vertex in_Vertices[];
};

layout(push_constant) uniform PerFrameData{
		mat4 proj;
		mat4 view;
		mat4 model;
		float tiling;
		uint textId;
		Vertices vertx;
}pc;

layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec3 fragWorldNormal;

vec3 normals[6] = vec3[](
				vec3(0.0f, 0.0f, 1.0f),
				vec3(0.0f, 0.0f, -1.0f),
				vec3(-1.0f, 0.0f, 0.0f),
				vec3(1.0f, 0.0f, 0.0f),
				vec3(0.0f, 1.0f, 0.0f),
				vec3(0.0f, -1.0f, 0.0f)
);

void main () {
  Vertex vtx = pc.vertx.in_Vertices[gl_VertexIndex];
	vec4 worldPos  = pc.model * vec4(vtx.x, vtx.y, vtx.z, 1.0f);
	gl_Position = pc.proj * pc.view * worldPos;

	vec3 norm = normals[int(vtx.face)];


	fragWorldNormal = mat3(transpose(inverse(pc.model))) *norm; 
	fragWorldPos = worldPos.xyz;

}
