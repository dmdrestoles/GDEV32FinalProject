#version 330 core
layout(location = 0) in vec3 vertexPos;

out vec3 texCoords;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

void main()
{
    texCoords = vertexPos;
    vec4 newPos = projectionMatrix * viewMatrix * vec4(vertexPos, 1.0);
    gl_Position = newPos.xyzw;
}