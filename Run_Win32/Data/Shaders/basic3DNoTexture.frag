#version 330 core

in vec4 passColor;
in vec3 passPos;

out vec4 outColor;
out vec3 outPos;

void main( void )
{
	outColor = passColor;
	outPos = passPos;
}