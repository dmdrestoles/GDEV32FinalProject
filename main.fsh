#version 330

// Take the 'outPos' output from the vertex shader as input of our fragment shader
in vec3 outPos;

// Take the 'outColor' output from the vertex shader as input of our fragment shader
in vec3 outColor;

// Take the 'outNormals' output from the vertex shader as input of our fragment shader
in vec3 outNormal;

// Take the 'outUV' output from the vertex shader as input of our fragment shader
in vec2 outUV;

// Final color of the fragment, which we are required to output
out vec4 fragColor;

// Uniform variable that will hold the texture unit of the texture that we want to use
uniform sampler2D tex;

struct PointLight
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 position;
	vec3 objectSpecular;
	int shininess;
	vec3 attenuation;
};

struct Ambience
{
	vec3 color;
	float strength;
	vec3 position;
	vec3 fragPos;
	vec3 attenuation;
	vec3 ambience;
};

struct Diffuse
{
	vec3 color;
	vec3 normal;
	vec3 direction;
	vec3 fragPos; 
	vec3 lightPos;
	vec3 attenuation;
	vec3 diffuse;
};

uniform vec3 eye;
uniform PointLight ptLight;

float ComputeAttenuation(vec3 position, vec3 fragPos, vec3 attenuation)
{
	// Attenuation is vec3 = { quadratic, linear, constant }
	float distance = length(position - fragPos);
	float attenuationFactor = 1.0 / (attenuation[2] + (attenuation[1] * distance) + (attenuation[0] * (distance * distance)) );

	return attenuationFactor;
}

vec3 ComputeAmbience(Ambience ambience)
{
	vec3 compAmb = ambience.strength * ambience.color;
	return compAmb;
}

vec3 ComputeDiffuse(Diffuse diffuse)
{
	vec3 norm = normalize(diffuse.normal);
	vec3 lightDir = normalize(diffuse.lightPos - diffuse.fragPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 compDiff = diff * diffuse.color;
	
	return compDiff;
}
Ambience plAmbience;
Diffuse plDiffuse;

void main()
{
	/**/
	// Point light
	plAmbience.color = ptLight.ambient;
	plAmbience.position = ptLight.position;
	plAmbience.fragPos = outPos;
	plAmbience.strength = 0.1f;
	plAmbience.attenuation = ptLight.attenuation;

	plDiffuse.color = ptLight.diffuse;
	plDiffuse.normal = outNormal;
	plDiffuse.fragPos = outPos;
	plDiffuse.lightPos = ptLight.position;
	plDiffuse.attenuation = ptLight.attenuation;

	float ptLightAttenuationFactor = ComputeAttenuation(ptLight.position, outPos, ptLight.attenuation);

	plAmbience.ambience = ptLightAttenuationFactor * ComputeAmbience(plAmbience);
	plDiffuse.diffuse = ptLightAttenuationFactor * ComputeDiffuse(plDiffuse);

	vec3 finalLightColor = (plAmbience.ambience + plDiffuse.diffuse) * outColor;
	vec4 processedLight = vec4(finalLightColor, 1.0f);
	vec4 sampledColor = texture(tex, outUV);

	fragColor = processedLight * sampledColor;
}