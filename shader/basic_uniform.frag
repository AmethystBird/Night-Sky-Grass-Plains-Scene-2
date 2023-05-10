#version 460

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

uniform struct lightInfo
{
	vec4 lightPosition;
	vec3 lightAmbient;
	vec3 lightDiffuse;
	vec3 lightSpecular;
} lights[3];

uniform struct MaterialInfo
{
	vec3 materialAmbient;
	vec3 materialDiffuse;
	vec3 materialSpecular;
	float shinyness;
} material;

uniform struct FogInfo
{
    float MaxDistance;
    float MinDistance;
    vec3 Colour;
} fog;

//Calculates ambient, diffuse & specular
vec3 PhongMultiTexture(int lightIndex, vec3 cameraNormalization, vec4 cameraPosition)
{
	//Extraction of colour for each fragment
	vec3 primaryTextureColour = texture(baseTexture, textureCoordinates).rgb;
	vec3 secondaryTextureColour = texture(overlayTexture, textureCoordinates).rgb;

	vec3 textureColour = mix(primaryTextureColour.rgb, secondaryTextureColour.rgb, 1.0);

	vec3 ambient = lights[lightIndex].lightAmbient * material.materialAmbient * textureColour; //Ambience

	//Diffusion
	vec3 lightPosToVertexPosDirection = normalize(vec3(lights[lightIndex].lightPosition - (cameraPosition * lights[lightIndex].lightPosition)));
	float sDotN = max(dot(lightPosToVertexPosDirection, cameraNormalization), 0.0);
	vec3 diffuse = lights[lightIndex].lightDiffuse * material.materialDiffuse * sDotN * textureColour;

	vec3 specular = vec3(0.0); //Specular

	//If dot product is above 0, reflection can take place
	if (sDotN > 0.0)
	{
		vec3 v = normalize(-cameraPosition.xyz);
		vec3 reflection = reflect(-lightPosToVertexPosDirection, cameraNormalization);
		specular = lights[lightIndex].lightSpecular * material.materialSpecular * pow(max(dot(reflection, v), 0.0), material.shinyness);
	}

	return ambient + diffuse + specular; //Composition of all light components
}

//Calculates ambient, diffuse & specular
vec3 Phong(int lightIndex, vec3 cameraNormalization, vec4 cameraPosition)
{
	//Extraction of colour for each fragment
	vec3 textureColour = texture(baseTexture, textureCoordinates).rgb;

	/*vec3 textureColour = vec3(0.0);
	if (textureIndexFrag == 0)
	{
		textureColour = texture(baseTexture, textureCoordinates).rgb;
	}
	else if (textureIndexFrag == 1)
	{
		textureColour = texture(rockTexture, textureCoordinates).rgb;
	}*/

	vec3 ambient = lights[lightIndex].lightAmbient * material.materialAmbient * textureColour; //Ambience

	//Diffusion
	vec3 lightPosToVertexPosDirection = normalize(vec3(lights[lightIndex].lightPosition - (cameraPosition * lights[lightIndex].lightPosition)));
	float sDotN = max(dot(lightPosToVertexPosDirection, cameraNormalization), 0.0);
	vec3 diffuse = lights[lightIndex].lightDiffuse * material.materialDiffuse * sDotN * textureColour;

	vec3 specular = vec3(0.0); //Specular

	//If dot product is above 0, reflection can take place
	if (sDotN > 0.0)
	{
		vec3 v = normalize(-cameraPosition.xyz);
		vec3 reflection = reflect(-lightPosToVertexPosDirection, cameraNormalization);
		specular = lights[lightIndex].lightSpecular * material.materialSpecular * pow(max(dot(reflection, v), 0.0), material.shinyness);
	}

	return ambient + diffuse + specular; //Composition of all light components
}

vec4 Fog()
{
    float distance = abs(position.z); //Calculation of distance | Inference: (on the z plane, so presumably away from viewer)

    float fogFactor = (fog.MaxDistance - distance) / (fog.MaxDistance - fog.MinDistance); //calculation of fog intensity based on distance / position of object
    fogFactor = clamp(fogFactor, 0.0, 1.0); //clamping values (investigate what this means)

	vec3 shadeColour;
	for (int i = 0; i < 3; i++)
	{
    	shadeColour += Phong(i, normal, position);
	}

	vec3 colour = mix(fog.Colour, shadeColour, fogFactor);
	return vec4(colour, 1.0);
}

void main() {
	//vec4 alphaMap = texture(alphaTexture, textureCoordinates);
	vec3 skyBoxTextureColour = texture(skyBoxTexture, normalize(vertexPositionFrag)).rgb;

	//Temporarily removed
	vec3 projectedTextureColour = vec3(0.0);
	if (projectedTextureCoordinates.z > 0.0) //if value is positive, means in view of projector, so render
	{
		projectedTextureColour = textureProj(projectedTexture, projectedTextureCoordinates).rgb;
	}

	vec3 colour = vec3(0.0); //create colour directly in fragment shader since not passed by vertex shader anymore

	for (int i = 0; i < 3; i++)
	{
		colour += PhongMultiTexture(i, normal, position);
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