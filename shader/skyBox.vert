#version 460

layout (location = 0) in vec3 VertexPosition;

out vec3 vertexPositionFrag;

uniform mat4 MVP;

void main()
{
    vertexPositionFrag = VertexPosition;
	gl_Position = MVP * vec4(VertexPosition, 1.0);// * RotationMatrix; //multiply by rotation offset
}