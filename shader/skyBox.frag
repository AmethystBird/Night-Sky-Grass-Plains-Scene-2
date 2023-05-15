#version 460

layout (location = 0) out vec4 FragColor;
layout (binding = 0) uniform samplerCube skyBoxTexture;

in vec3 vertexPositionFrag;
in vec4 position;

uniform struct FogInfo
{
    float MaxDistance;
    float MinDistance;
    vec3 Colour;
} fog;

uniform vec3 skyBoxLightAmbient;
uniform vec3 skyBoxMaterialAmbient;

//Calculates ambient light only; no diffuse & specular lighting
vec3 AmbientLight()
{
	vec3 ambient = skyBoxLightAmbient * skyBoxMaterialAmbient;

	return ambient;
}

vec4 Fog()
{
    float distance = abs(position.z); //Calculation of distance | Inference: (on the z plane, so presumably away from viewer)

    float fogFactor = (fog.MaxDistance - distance) / (fog.MaxDistance - fog.MinDistance); //calculation of fog intensity based on distance / position of object
    fogFactor = clamp(fogFactor, 0.0, 1.0); //clamping values (investigate what this means)

	vec3 shadeColour;
	for (int i = 0; i < 3; i++)
	{
    	shadeColour += AmbientLight();
	}

	vec3 colour = mix(fog.Colour, shadeColour, fogFactor);
	return vec4(colour, 1.0);
}

void main() {
    vec3 colour = vec3(0.0);
    for (int i = 0; i < 3; i++)
    {
        colour += AmbientLight();
    }

	vec3 skyBoxTextureColour = texture(skyBoxTexture, normalize(vertexPositionFrag)).rgb;
	vec4 fogColour = Fog();

	FragColor = vec4(skyBoxTextureColour, 1.0) + vec4(colour, 1) + fogColour;
}