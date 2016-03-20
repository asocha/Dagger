#version 330 core

uniform mat4 projection;

in vec2 inPosition;
in vec4 inColor;

out vec4 passColor;
out vec2 passPos;

void main( void )
{
	vec4 pos = vec4(inPosition, 0.0f, 1.0f);
	pos = projection * pos; 
	
	passColor = inColor;
   	passPos = pos.xy;

	gl_Position = pos;
}