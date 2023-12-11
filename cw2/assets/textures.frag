#version 430

//input attributes
//matches vertex shader
in vec3 v2fColor;
in vec3 v2fNormal;
in vec2 v2fTexCoord;

layout (location = 2) uniform vec3 uLightDir;
layout (location = 3) uniform vec3 uLightDiffuse;
layout (location = 4) uniform vec3 uSceneAmbient;
layout (binding = 0) uniform sampler2D uTexture;
//output attributes
layout (location = 0) out vec3 oColor;

void main()
{
	vec3 normal = normalize(v2fNormal);

	float nDotL = max( 0.0, dot(normal, uLightDir));
	oColor = texture( uTexture, v2fTexCoord ).rgb * (uSceneAmbient + nDotL * uLightDiffuse) * v2fColor;
}
