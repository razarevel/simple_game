#version 460 core

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout: require

struct Vertex {
		float x,y,z;
		float u,v;
};

layout(buffer_reference, scalar) readonly buffer Vertices{
		Vertex in_Vertices[];
};

layout(push_constant) uniform PerFrameData{
		mat4 proj;
		mat4 view;
		mat4 model;
		uint textId;
		Vertices vertx;
}pc;

layout(location = 0) out vec2 uv;

void main () {
  Vertex vtx = pc.vertx.in_Vertices[gl_VertexIndex];
	gl_Position = pc.proj * pc.view * pc.model * vec4(vtx.x, vtx.y, vtx.z, 1.0f);

	uv = vec2(vtx.u, vtx.v);
}
