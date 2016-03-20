#version 330 core

uniform mat4 u_projection;
uniform mat4 u_view;

in vec3 inPosition;
in vec4 inColor;

out vec4 passColor;
out vec3 passPos;

void main( void )
{
	vec4 pos = vec4(inPosition, 1.0f);
	pos = u_projection * u_view * pos; 
	
	passColor = inColor;
   	passPos = pos.xyz;

	gl_Position = pos;
}