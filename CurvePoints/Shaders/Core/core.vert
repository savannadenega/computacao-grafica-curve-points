#version 410

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec3 vertColor;

out vec3 color;

void main () {
	color = vertColor;
	gl_Position = vec4(vertPosition, 1.0);
}