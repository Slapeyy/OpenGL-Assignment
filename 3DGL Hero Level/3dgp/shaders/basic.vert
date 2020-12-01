#version 330

// Uniforms: Transformation Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;

// Uniforms: Material Colours
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

// Uniforms: Fog Density
uniform float fogDensity;

layout (location = 0) in vec3 aVertex;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;
layout (location = 4) in vec3 aTangent;
layout (location = 5) in vec3 aBiTangent;

// Output Variables
out vec4 color;
out vec4 position;
out vec3 normal;
out vec2 texCoord0;
out vec3 texCoordCubeMap;
out vec4 normal2;
out float fogFactor;
out mat3 matrixTangent;

// Light declarations
struct AMBIENT
{	
	int on;
	vec3 color;
};
uniform AMBIENT lightAmbient, lightEmissive;

struct DIRECTIONAL
{	
	int on;
	vec3 direction;
	vec3 diffuse;
};
uniform DIRECTIONAL lightDir;

vec4 AmbientLight(AMBIENT light)
{
	// Calculate Ambient Light
	return vec4(materialAmbient * light.color, 1);
}

vec4 DirectionalLight(DIRECTIONAL light)
{
	// Calculate Directional Light
	vec4 color = vec4(0, 0, 0, 0);
	vec3 L = normalize(mat3(matrixView) * light.direction);
	float NdotL = dot(normal, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;
	return color;
}

void main(void) 
{
	// calculate position
	position = matrixModelView * vec4(aVertex, 1.0);
	gl_Position = matrixProjection * position;

	// calculate normal
	normal = normalize(mat3(matrixModelView) * aNormal);
	normal2 = vec4(normalize(mat3(matrixModelView) * aNormal), 1);

	// calculate tangent local system transformation
	vec3 tangent = normalize(mat3(matrixModelView) * aTangent);
	tangent = normalize(tangent - dot(tangent, normal) * normal);	// Gramm-Schmidt process
	vec3 biTangent = cross(normal, tangent);
	matrixTangent = mat3(tangent, biTangent, normal);

	// calculate texture coordinate
	texCoord0 = aTexCoord;
	//texCoordCubeMap = -inverse(mat3(matrixView)) * reflect(position.xyz, normal2.xyz);

	// Adding a "-" before inverse will cause reflections to flip and appear on top of the UFO
	texCoordCubeMap = inverse(mat3(matrixView)) * mix(reflect(position.xyz, normal2.xyz), normal2.xyz, 0.2);
	
	// calculate fog
	fogFactor = exp2(-fogDensity * length(position));

	// calculate light
	color = vec4(1, 1, 1, 1);
	if (lightAmbient.on == 1) 
		color += AmbientLight(lightAmbient);
	if (lightDir.on == 1) 
		color += DirectionalLight(lightDir);
}
