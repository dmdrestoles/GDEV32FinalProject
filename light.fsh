#version 330 core
out vec4 fragColor;

in vec2 outUV;

uniform sampler2D tex;

void main()
{
	vec4 processedLight = vec4(1.0f);
	vec4 sampledColor = texture(tex, outUV);

	fragColor = processedLight * sampledColor;
	//fragColor = processedLight;
}