#version 460 core

#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 0) uniform texture2D kTextures2D[];
layout(set = 0, binding = 1) uniform sampler kSamplers[];

layout(push_constant) uniform PerFrameData{
		mat4 proj;
		mat4 view;
		mat4 model;
		float tiling;
		uint textId;
}pc;

layout(location = 0) out vec4 out_FragColor;

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragWorldNormal;

vec4 textureBindless2D(uint textureid, uint samplerid, vec2 uv) {
    return texture(nonuniformEXT(sampler2D(kTextures2D[textureid], kSamplers[samplerid])), uv);
}

void main () {
		vec3 normal = normalize(fragWorldNormal);

		// get blending weight from normal
		vec3 blend = abs(normal);
		blend = normalize(max(blend, 0.00001));
		blend /= (blend.x + blend.y + blend.z);


		vec2 uvX = fragWorldPos.yz * pc.tiling; // project along x
		vec2 uvY = fragWorldPos.xz * pc.tiling; // project along x
		vec2 uvZ = fragWorldPos.xy * pc.tiling; // project along x

		vec4 texX = textureBindless2D(pc.textId, 0, uvX);
		vec4 texY = textureBindless2D(pc.textId, 0, uvY);
		vec4 texZ = textureBindless2D(pc.textId, 0, uvZ);

		vec4 finalColor = texX * blend.x 
				+ texY * blend.y + texZ * blend.z;

		out_FragColor = finalColor;
}
