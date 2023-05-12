#version 460

const float pi = 3.14159265359;

in vec4 position;
in vec3 normal;
in vec2 textureCoordinates;
in vec3 vertexPositionFrag;
in vec4 projectedTextureCoordinates;

//Vertex lighting
in vec3 LightIntensity;

//texture differentation
//flat in int textureIndexFrag;

layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform sampler2D baseTexture;
layout (binding = 1) uniform sampler2D overlayTexture;
//layout (binding = 1) uniform sampler2D alphaTexture;
layout (binding = 2) uniform samplerCube skyBoxTexture;
layout (binding = 3) uniform sampler2D projectedTexture;
//layout (binding = 3) uniform sampler2D rockTexture;

uniform struct lightInfoPhong
{
	vec4 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
} lightsPhong;

uniform struct MaterialInfoPhong
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shinyness;
} materialPhong;

uniform struct lightInfo
{
	vec4 position;
	vec3 intensity;
} lights[3];

uniform struct MaterialInfo
{
	float roughness;
	bool metalicness;
	vec3 colour;
} material;

uniform struct FogInfo
{
    float MaxDistance;
    float MinDistance;
    vec3 Colour;
} fog;

float GGXDistribution(float nDotH)
{
	float alpha2 = material.roughness * material.roughness * material.roughness * material.roughness;
	float denominator = (nDotH * nDotH) * (alpha2 - 1) + 1;
	return alpha2 / (pi * denominator * denominator);
}

float GeomSmith(float dotProduct)
{
	float k = (material.roughness + 1.0) * (material.roughness + 1.0) / 8.0;
	float denominator = dotProduct * (1 - k) + k;
	return 1.0 / denominator;
}

vec3 SchlickFresnel(float lDotH)
{
	vec3 f0 = vec3(0.04);
	if (material.metalicness)
	{
		f0 = material.colour;
	}

	return f0 + (1 - f0) * pow(1.0 - lDotH, 5);
}

vec3 MicroFacetModel(int lightIDX, vec3 normalised, vec3 position)
{
	vec3 diffuseBrdf = vec3(0.0);
	if (!material.metalicness)
	{
		diffuseBrdf = material.colour;
	}

	vec3 l = vec3(0.0), lightI = lights[lightIDX].intensity; //this line may be problematic due to 'lightI'
	if (lights[lightIDX].position.w == 0.0)
	{
		l = normalize(lights[lightIDX].position.xyz);
	}
	else
	{
		l = lights[lightIDX].position.xyz - position;
		float distance = length(l);
		l = normalize(l);
		lightI /= (distance * distance);
	}

	vec3 v = normalize(-position);
	vec3 h = normalize(v + 1);
	float nDotH = dot(normalised, h);
	float lDotH = dot(l, h);
	float nDotL = max(dot(normalised, l), 0.0);
	float nDotV = dot(normalised, v);
	vec3 specularBrdf = 0.25 * GGXDistribution(nDotH) * SchlickFresnel(lDotH) * GeomSmith(nDotL) * GeomSmith(nDotV);

	return (diffuseBrdf + pi * specularBrdf) * lightI * nDotL;
}

vec4 Fog()
{
    float distance = abs(position.z); //Calculation of distance | Inference: (on the z plane, so presumably away from viewer)

    float fogFactor = (fog.MaxDistance - distance) / (fog.MaxDistance - fog.MinDistance); //calculation of fog intensity based on distance / position of object
    fogFactor = clamp(fogFactor, 0.0, 1.0); //clamping values (investigate what this means)

	vec3 shadeColour;
	for (int i = 0; i < 3; i++)
	{
    	shadeColour += MicroFacetModel(i, normal, position.xyz);
	}

	vec3 colour = mix(fog.Colour, shadeColour, fogFactor);
	return vec4(colour, 1.0);
}

void main() {
	/*vec3 colour = vec3(0);
	vec3 normalised = normalize(normal);
	for (int i = 0; i < 3; i++)
	{
		colour += MicroFacetModel(i, normalised, position);
	}*/

	//vec4 alphaMap = texture(alphaTexture, textureCoordinates);
	vec3 skyBoxTextureColour = texture(skyBoxTexture, normalize(vertexPositionFrag)).rgb;

	//Temporarily removed
	vec3 projectedTextureColour = vec3(0.0);
	if (projectedTextureCoordinates.z > 0.0) //if value is positive, means in view of projector, so render
	{
		projectedTextureColour = textureProj(projectedTexture, projectedTextureCoordinates).rgb;
	}

	vec3 colour = vec3(0.0); //create colour directly in fragment shader since not passed by vertex shader anymore
	vec3 normalised = normalize(normal); //apparently needs this, but probs doesn't; appears redundant

	for (int i = 0; i < 3; i++)
	{
		colour += MicroFacetModel(i, normalised, position.xyz);
	}

	/*if (textureIndexFrag == 0)
	{
		for (int i = 0; i < 3; i++)
		{
			colour += PhongMultiTexture(i, normal, position);
		}
	}
	else if (textureIndexFrag == 1)
	{
		for (int i = 0; i < 3; i++)
		{
			colour += Phong(i, normal, position);
		}
	}*/

	vec4 fogColour = Fog();
	FragColor = fogColour + vec4(skyBoxTextureColour, 1.0) + vec4(LightIntensity, 1.0) + vec4(colour + projectedTextureColour, 1);
}