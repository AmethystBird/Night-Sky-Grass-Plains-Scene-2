#version 460

layout (location = 0) in vec3 VertexInitialVelocity;
layout (location = 1) in float VertexBirthTime;

out float transparency;
out vec2 textureCoordinate;

uniform float time;
uniform vec3 gravity = vec3(0.0, -0.05, 0.0);
uniform float particleLifetime;
uniform float particleSize = 1.0;
uniform vec3 emitterPosition;

uniform mat4 modelViewMatrix;
uniform mat4 projection;

const vec3 offsets[] = vec3[](vec3(-0.5, -0.5, 0), vec3(0.5, -0.5, 0), vec3(0.5, 0.5, 0), vec3(-0.5, -0.5, 0), vec3(0.5, 0.5, 0), vec3(-0.5, 0.5, 0));
const vec2 textureCoordinates[] = vec2[](vec2(0,0), vec2(1,0), vec2(1,1), vec2(0,0), vec2(1,1), vec2(0,1));

void main()
{
	vec3 cameraPosition;
	float timeChange = time - VertexBirthTime;
	if (timeChange >= 0 && timeChange < particleLifetime)
	{
		vec3 position = emitterPosition + VertexInitialVelocity * timeChange + gravity * timeChange * timeChange;
		cameraPosition = (modelViewMatrix * vec4(position, 1)).xyz + (offsets[gl_VertexID] * particleSize);
		transparency = mix(1, 0, timeChange / particleLifetime);
	}
	else
	{
		cameraPosition = vec3(0);
		transparency = 0.0;
	}

	textureCoordinate = textureCoordinates[gl_VertexID];
	gl_Position = projection * vec4(cameraPosition, 1);
}