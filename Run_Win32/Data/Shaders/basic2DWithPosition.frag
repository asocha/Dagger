#version 330 core

uniform sampler2D texture2D;

in vec4 passColor;
in vec2 passPos;
in vec2 passUV;

out vec4 outColor;
out vec2 outPos;
out vec2 outUV;

void main( void )
{
	vec4 texColor = texture(texture2D, passUV);
	outColor = passColor * texColor;

	outUV = passUV;
	outPos = passPos;
}