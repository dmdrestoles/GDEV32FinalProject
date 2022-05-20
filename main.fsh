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

// If we want to simultaneously use another texture at a different texture unit,
// we can create another uniform for it.
// Example:
// uniform sampler2D tex1;
// We can then set the value of this uniform to 1 (for texture unit 1) in our Main.cpp

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

struct DirectionalLight
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 direction;
	vec3 objectSpecular;
	int shininess;
};

// Spotlight struct
struct Spotlight
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 position;
	vec3 direction;
	float cutoff;
	float outerCutoff;
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

struct Specular
{
	vec3 color;
	vec3 normal; 
	vec3 fragPos; 
	vec3 lightPos;
	vec3 direction;
	int shininess;
	vec3 viewPos;
	vec3 objectSpecular;
	vec3 attenuation;
	vec3 specular;
};

uniform vec3 eye;
uniform PointLight ptLight;
uniform DirectionalLight dirLight;
uniform Spotlight spotlight;

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

vec3 ComputeDirAmbience(Ambience ambience)
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

vec3 ComputeDirDiffuse(Diffuse diffuse)
{
	vec3 norm = normalize(diffuse.normal);
	float diff = max(dot(norm, normalize(diffuse.direction)), 0.0f);
	vec3 compDiff = diff * diffuse.color;
	
	return compDiff;
}

vec3 ComputeSpecular(Specular specular)
{
	vec3 viewDir = normalize(specular.viewPos - specular.fragPos);
	vec3 lightDir = normalize(specular.lightPos - specular.fragPos);
	vec3 reflectDir = reflect(-lightDir, specular.normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), specular.shininess);
	vec3 compSpec = spec * specular.objectSpecular * specular.color;

	return compSpec;
}

vec3 ComputeDirSpecular(Specular specular)
{
	vec3 viewDir = normalize(specular.viewPos - specular.fragPos);
	vec3 reflectDir = reflect(-normalize(specular.direction), specular.normal);
	float dirSpec = pow(max(dot(viewDir, reflectDir), 0.0f), specular.shininess);
	vec3 compSpec = dirSpec * specular.objectSpecular * specular.color;

	return compSpec;
}

float ComputeSpotlightClamp(Spotlight spotlight, float theta)
{
	float epsilon = spotlight.cutoff - spotlight.outerCutoff;
	float intensity = clamp( (theta - spotlight.outerCutoff) / epsilon, 0.0, 1.0 );

	return intensity;
}
Ambience plAmbience, dlAmbience, spAmbience;
Diffuse plDiffuse, dlDiffuse, spDiffuse;
Specular plSpecular, dlSpecular, spSpecular;

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

	plSpecular.color = ptLight.specular;
	plSpecular.shininess = ptLight.shininess;
	plSpecular.normal = outNormal; 
	plSpecular.fragPos = outPos; 
	plSpecular.lightPos = ptLight.position;
	plSpecular.viewPos = eye;
	plSpecular.objectSpecular = ptLight.objectSpecular;
	plSpecular.attenuation = ptLight.attenuation;
	
	float ptLightAttenuationFactor = ComputeAttenuation(ptLight.position, outPos, ptLight.attenuation);

	plAmbience.ambience = ptLightAttenuationFactor * ComputeAmbience(plAmbience);
	plDiffuse.diffuse = ptLightAttenuationFactor * ComputeDiffuse(plDiffuse);
	plSpecular.specular = ptLightAttenuationFactor * ComputeSpecular(plSpecular);
	
	// Directional light
	dlAmbience.color = dirLight.ambient;
	dlAmbience.strength = 0.9f;

	dlDiffuse.color = dirLight.diffuse;
	dlDiffuse.normal = outNormal;
	dlDiffuse.fragPos = outPos;
	dlDiffuse.direction = dirLight.direction;
	
	dlSpecular.color = dirLight.specular;
	dlSpecular.shininess = dirLight.shininess;
	dlSpecular.normal = outNormal;
	dlSpecular.fragPos = outPos;
	dlSpecular.direction = dirLight.direction;
	dlSpecular.viewPos = eye;
	dlSpecular.objectSpecular = dirLight.objectSpecular;
	
	float dirLightAttenuationFactor = 1.0f;

	dlAmbience.ambience = ComputeDirAmbience(dlAmbience);
	dlDiffuse.diffuse = ComputeDirDiffuse(dlDiffuse);
	dlSpecular.specular = ComputeDirSpecular(dlSpecular);
	
	// Spotlight
	spAmbience.color = spotlight.ambient;
	spAmbience.strength = 0.9f;

	spDiffuse.color = spotlight.diffuse;
	spDiffuse.normal = outNormal;
	spDiffuse.fragPos = outPos;
	spDiffuse.lightPos = eye;
	spDiffuse.attenuation = spotlight.attenuation;

	spSpecular.color = spotlight.specular;
	spSpecular.shininess = spotlight.shininess;
	spSpecular.normal = outNormal; 
	spSpecular.fragPos = outPos; 
	spSpecular.lightPos = eye;
	spSpecular.viewPos = eye;
	spSpecular.objectSpecular = spotlight.objectSpecular;
	
	vec3 lightDir = normalize(spotlight.position - outPos);
	float theta = dot(lightDir, normalize(-spotlight.direction));
	float intensity = ComputeSpotlightClamp(spotlight, theta);

	float spotLightAttenuationFactor = ComputeAttenuation(spotlight.position, outPos, spotlight.attenuation);

	spAmbience.ambience = spotLightAttenuationFactor * ComputeAmbience(spAmbience);
	spDiffuse.diffuse = vec3(0.0f, 0.0f, 0.0f);
	spSpecular.specular = vec3(0.0f, 0.0f, 0.0f);

	if ( theta > spotlight.cutoff )
	{
		spDiffuse.diffuse = spotLightAttenuationFactor * ComputeDiffuse(spDiffuse);
		spSpecular.specular = spotLightAttenuationFactor * ComputeSpecular(spSpecular);
	}

	spDiffuse.diffuse *= intensity;
	spSpecular.specular *= intensity;

	vec3 averageAmbience = (plAmbience.ambience + dlAmbience.ambience + spAmbience.ambience) / 3.0f;

	vec3 finalLightColor = (averageAmbience + ((plDiffuse.diffuse + dlDiffuse.diffuse + spDiffuse.diffuse) + (plSpecular.specular + dlSpecular.specular + spSpecular.specular))) * outColor;
	//vec3 finalLightColor = (plAmbience.ambience + plDiffuse.diffuse + plSpecular.specular) * outColor;
	vec4 processedLight = vec4(finalLightColor, 1.0f);
	vec4 sampledColor = texture(tex, outUV);

	fragColor = processedLight * sampledColor;
}