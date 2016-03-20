#version 330 core

uniform mat4 u_projection;
uniform vec2 u_positionOffset;

in vec2 inPosition;
in vec4 inColor;
in vec2 inUV;

out vec4 passColor;
out vec2 passPos;
out vec2 passUV;

void main( void )
{
	vec4 pos = vec4(inPosition + u_positionOffset, 0.0f, 1.0f);
	pos = u_projection * pos; 
	
	passColor = inColor;
	passUV = inUV;
   	passPos = pos.xy;

	gl_Position = pos;
}