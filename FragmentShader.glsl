#version 410

const vec3 diffuseColor = vec3(0.64, 0.64, 0.64);
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const float shininess = 8.0f;
const float borderWidth = 0.05 / 32;

in vec3 gsViewNormal;
in vec3 gsToEye;
in vec3 gsColor;
in vec3 gsBaryCoords;

uniform bool showBorders;
uniform vec3 lightDirection;
uniform float customOuterTessFactor;
uniform float customInnerTessFactor;

out vec4 result;

#define saturate(x) clamp(x, 0, 1)

bool isNearBorder(vec3 bary)
{
	float borderWidthScaled = borderWidth * max(customOuterTessFactor, customInnerTessFactor);

	return bary.x < borderWidthScaled
		|| bary.y < borderWidthScaled
		|| bary.z < borderWidthScaled;
}

void main()
{
	result = vec4(0, 0, 0, 1);

	vec3 diffuse = diffuseColor;
	diffuse *= gsColor;

	vec3 specularColor = lightColor;

	vec3 normal = gsViewNormal;

	float diffuseFactor = saturate(dot(lightDirection, normal));

	vec3 reflectedRay = reflect(-lightDirection, normal);
	float specularFactor = pow(saturate(dot(reflectedRay, gsToEye)), shininess);

	if (showBorders && isNearBorder(gsBaryCoords))
		diffuseFactor = 0;

	result.rgb += diffuseFactor * diffuse;

	if (diffuseFactor > 0)
		result.rgb += specularFactor * specularColor;
	
	result = saturate(result);
}