#version 460

layout (location = 0) out vec4 FragColor;
layout (binding = 0) uniform samplerCube skyBoxTexture;

in vec3 vertexPositionFrag;

uniform vec3 skyBoxLightAmbient;
uniform vec3 skyBoxMaterialAmbient;

//Calculates ambient, diffuse & specular
vec3 AmbientLight()
{
	//Extraction of colour for each fragment
	//vec3 textureColour = texture(baseTexture, textureCoordinates).rgb;

	vec3 ambient = skyBoxLightAmbient * skyBoxMaterialAmbient;// * textureColour; //Ambience

	return ambient; //Only ambient
}

void main() {
    vec3 colour = vec3(0.0);
    for (int i = 0; i < 3; i++)
    {
        colour += AmbientLight();
    }

	vec3 skyBoxTextureColour = texture(skyBoxTexture, normalize(vertexPositionFrag)).rgb;
	FragColor = vec4(skyBoxTextureColour, 1.0) + vec4(colour, 1);
}