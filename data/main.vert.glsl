#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 vert_color;
flat out vec3 frag_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
	frag_color = vert_color;
}