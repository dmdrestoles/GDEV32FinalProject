#version 330

// Vertex attributes as inputs
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormals;
layout(location = 2) in vec3 vertexColor;
layout(location = 3) in vec2 vertexUV;

// Output position
out vec3 outPos;

// Output color
out vec3 outColor;

// Output normals
out vec3 outNormal;

// Output UV-coordinates
out vec2 outUV;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
// 4x4 matrix uniform variable to contain the transformation to be applied to of our vertex position
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;

mat2 uvRotation;
float angle;

void main()
{
	// Transform our vertex position to homogeneous coordinates.
	// Remember that w = 1.0 means that the vector is a position.
	vec4 finalPosition = vec4(vertexPosition, 1.0);
	vec4 shaderPos = modelMatrix * finalPosition;
	outPos = vec3(shaderPos);

	// Apply the transformation to our vertex position by multiplying
	// our transformation matrix
	finalPosition = projectionMatrix * viewMatrix * modelMatrix * finalPosition;

	// gl_Position is a built-in shader variable that we need to set
	gl_Position = finalPosition;

	// We pass the color of the current vertex to our output variable
	outColor = vertexColor;

	float s = sin(angle);
	float c = cos(angle);

	uvRotation = mat2(0.f, -1.f, 1.f, 0.f);
	// We pass the UV-coordinates of the current vertex to our output variable
	outUV = vertexUV;

	// We pass the normals of the current vertex to our output variable
	vec3 finalNormals = mat3(transpose(inverse(modelMatrix))) * vertexNormals;
	outNormal = finalNormals;
}
