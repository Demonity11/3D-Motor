#version 330 core

out vec4 fragColor;

in vec4 color;

void main()
{
	fragColor = vec4(color);
}