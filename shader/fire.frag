#version 460

in float transparency;
in vec2 textureCoordinate;
uniform sampler2D particleTexture;

layout (location = 0) out vec4 FragColor;

void main()
{
	FragColor = texture(particleTexture, textureCoordinate);
	FragColor.a *= transparency;
}