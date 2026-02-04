#version 460 core

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

struct Vertex {
	float x, y;
	float u, v;
	float r,g,b;
};

layout(buffer_reference, scalar) readonly buffer Vertices {
	Vertex in_Vertices[];
};

layout(location = 0) out vec2 uvs;
layout(location = 1) out vec3 colors;

layout(push_constant) uniform PerFrameData {
    mat4 proj;
    uint textureId;
		Vertices vtx;
}pc;

vec3 getPosition(int i) {
	return vec3(pc.vtx.in_Vertices[i].x, pc.vtx.in_Vertices[i].y, 0.0f);
}

void main() {
		int i = gl_VertexIndex;
		gl_Position = pc.proj * vec4(getPosition(i), 1.0f);
		uvs = vec2(pc.vtx.in_Vertices[i].u, pc.vtx.in_Vertices[i].v);
		colors = vec3(pc.vtx.in_Vertices[i].r, pc.vtx.in_Vertices[i].g, pc.vtx.in_Vertices[i].b);
}
