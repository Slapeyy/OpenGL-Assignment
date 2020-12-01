#version 330

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;
in vec3 texCoordCubeMap;
in float fogFactor;
in mat3 matrixTangent;

vec3 normalNew;

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;
uniform sampler2D textureNormal;

// View Matrix
uniform mat4 matrixView;

// Texture switch: 0 = skybox; 1 = reflective surface
uniform float reflectionPower;

// Uniform: The Texture
uniform sampler2D texture0;
uniform samplerCube textureCubeMap;

// Fog Colour
uniform vec3 fogColour;

// Output Variable (sent down through the Pipeline)
out vec4 outColor;

struct POINT
{
	int on;
	vec3 position;
	vec3 diffuse;
	vec3 specular;
};
uniform POINT lightPoint1;

struct SPOT
{
	int on;
	vec3 position;
	vec3 diffuse;
	vec3 specular;
	vec3 direction;
	float cutoff;
	float attenuation;
};
uniform SPOT spotLight;

vec4 PointLight(POINT light)
{
	// Calculate Diffuse Light
	vec4 color = vec4(0, 0, 0, 0);
	float dist = length(matrixView * vec4(light.position, 1) - position);
	float att = 1 / (0.0000055 * dist * dist);
	vec3 L = normalize((matrixView * vec4(light.position, 1)) - position).xyz;
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normalNew);
	float NdotL = dot(normalNew, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;
	return color * att;
}

vec4 SpotLight(SPOT light)
{
    // HERE GOES THE CODE COPIED FROM THE POINT LIGHT FUNCTION
    // Calculate Spot Light
    vec4 color = vec4(0, 0, 0, 0);
	float dist = length(matrixView * vec4(light.position, 1) - position);
	float att = 1 / (0.0000055 * dist * dist);
    vec3 L = normalize(matrixView * vec4(light.position, 1) - position).xyz;
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normalNew);
    float NdotL = dot(normalNew, L);
    if (NdotL > 0)
            color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;
	

    // HERE GOES THE NEW CODE TO DETERMINE THE SPOT FACTOR
    //vec3 D = normalize(matrixView * vec4(light.direction, 1)- position).xyz;
    vec3 D = normalize(mat3(matrixView) * light.direction);
    float LdotD = dot(-L, D);
    float A = acos(LdotD);
    float Cut = clamp(radians(light.cutoff), 0.0f, 90.0f);
    float spotFactor;
    if (A <= Cut) spotFactor = pow(LdotD, light.attenuation);
    else if (A > Cut) spotFactor = 0;

    // assuming that the Point Light value is stored as color and we have calculated spotFactor:
    return spotFactor * color;
}

void main(void) 
{
	outColor = color;

	normalNew = 2.0 * texture(textureNormal, texCoord0).xyz - vec3(1.0, 1.0, 1.0);
	normalNew = normalize(matrixTangent * normalNew);

	if (lightPoint1.on == 1)
		outColor += PointLight(lightPoint1);

	if (spotLight.on == 1)
		outColor += SpotLight(spotLight);

	// outColor order is as follows: normal mapping, environment mapping w/ reflections, fog
	outColor *= texture(texture0, texCoord0);
	outColor *= mix(texture(texture0, texCoord0.st), texture(textureCubeMap, texCoordCubeMap), reflectionPower);
	outColor = mix(vec4(fogColour, 1), outColor, fogFactor);
}
