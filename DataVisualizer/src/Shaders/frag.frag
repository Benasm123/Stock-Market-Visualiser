#version 450 core

layout(location = 0) in vec4 fragColor;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = fragColor;
	if (mod(gl_FragCoord.y, 10.0f) < 5.0f) {
		if (mod(gl_FragCoord.x, 10.0f) < 5.0f) {
			outColor = fragColor;
		}
	} else {
	if (mod(gl_FragCoord.x, 10.0f) > 5.0f) {
			outColor = fragColor;
		}
	}
}