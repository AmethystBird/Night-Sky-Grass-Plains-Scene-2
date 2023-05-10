#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;

//texture differentiation
//uniform int textureIndex;
//flat out int textureIndexFrag;

//Vertex lighting
uniform vec4 VertexLightPosition;
uniform vec3 Kd;
uniform vec3 Ld;
out vec3 LightIntensity;

out vec4 position;
out vec3 normal;
out vec2 textureCoordinates;
out vec3 vertexPositionFrag;
out vec4 projectedTextureCoordinates;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 ProjectionMatrix;

//For projected texture
uniform mat4 ProjectorMatrix;
uniform mat4 ModelMatrix;

uniform mat4 MVP;

//Conversion of vertex normal to eye coordinates
//Inference: Transforms object into correct position in relation to camera position
void GetCameraSpaceValues(out vec3 normal, out vec3 position)
{
	normal = normalize(NormalMatrix * VertexNormal);
	position = (ModelViewMatrix * vec4(VertexPosition, 1.0)).xyz;
}

void main()
{
	textureCoordinates = VertexTexCoord;
	GetCameraSpaceValues(normal, position.xyz);

	//Vertex lighting
	vec3 lightPosToVertexPosDirection = normalize(vec3(VertexLightPosition - position));
	LightIntensity = Kd * Ld * max (dot(lightPosToVertexPosDirection, normal), 0.0);

	//Projected texture
	projectedTextureCoordinates = ProjectorMatrix * (ModelMatrix * vec4(VertexPosition, 1.0));

	//textureIndexFrag = textureIndex;

	vertexPositionFrag = VertexPosition;
	gl_Position = MVP * vec4(VertexPosition, 1.0);// * RotationMatrix; //multiply by rotation offset
}