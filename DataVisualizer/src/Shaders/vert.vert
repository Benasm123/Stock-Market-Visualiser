#version 450 core

layout (binding = 0) uniform uniform_buffer_object {
	mat4 mvp;
} ubo;

layout (push_constant) uniform constants {
	vec4 color;
	mat4 mvp;
} pc;

layout (location = 0) in vec2 i_position;

layout (location = 0) out vec4 fragColor;

void main(void) 
{
	gl_PointSize = 1.5f;

	gl_Position = pc.mvp * vec4(i_position, -1.0f, 1.0f);
	
	fragColor = vec4(pc.color);
}