#version 330 core

in vec4 passColor;
in vec2 passPos;

out vec4 outColor;
out vec2 outPos;

void main( void )
{
	outColor = passColor;
	outPos = passPos;
}