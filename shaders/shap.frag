#version 460 core

#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 0) uniform texture2D kTextures2D[];
layout(set = 0, binding = 1) uniform sampler kSamplers[];

layout(push_constant) uniform PerFrameData{
		mat4 proj;
		mat4 view;
		mat4 model;
		uint textId;
}pc;

layout(location = 0) out vec4 out_FragColor;

layout(location = 0) in vec2 uvs;

vec4 textureBindless2D(uint textureid, uint samplerid, vec2 uv) {
    return texture(nonuniformEXT(sampler2D(kTextures2D[textureid], kSamplers[samplerid])), uv);
}

void main () {
		out_FragColor = textureBindless2D(pc.textId, 0, uvs);
}
