#version 450 core

layout(location = 0) in vec4 fragColor;
layout(location = 1) in float valid;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = vec4(0.9f, 0.94f, 0.93f, 1.0f);
	if (valid == 1.0f) {
		outColor = fragColor;
	}
}