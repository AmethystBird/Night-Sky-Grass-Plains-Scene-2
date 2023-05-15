#version 460

layout (location = 0) in vec3 VertexPosition;

out vec3 vertexPositionFrag;

uniform mat4 MVP;
uniform mat4 modelViewMatrix;
out vec4 position;

void main()
{
    position.xyz = (modelViewMatrix * vec4(VertexPosition, 1.0)).xyz;
    
    vertexPositionFrag = VertexPosition;
	gl_Position = MVP * vec4(VertexPosition, 1.0);
}