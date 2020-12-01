#version 330

// Uniforms: Transformation Matrices
uniform mat4 matrixProjection;
//uniform mat4 matrixView; not used
uniform mat4 matrixModelView;

uniform float t;	// real time

layout (location = 0) in vec3 aVertex;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;

out vec4 color;
out vec4 position;
out vec3 normal;
out vec2 texCoord0;
out float reflFactor;	// reflection coefficient

float wave(float A, float x, float y, float t)
{
	t *= 2;
	return A * (
			sin(2.0 * (x * 0.2 + y * 0.7) + t * 1.0) +
			sin(2.0 * (x * 0.7 + y * 0.2) + t * 0.8) +
			pow(sin(2.0 * (x * 0.6 + y * 0.5) + t * 1.2), 2) +
			pow(sin(2.0 * (x * 0.8 + y * 0.2) + t * 1.1), 2));
}

void main(void) 
{
	// Calculate the wave
	float a = 0.025;
	float y = wave(a, aVertex.x, aVertex.z, t);

	float d = 0.05;
	float dx = (wave(a, aVertex.x+d, aVertex.z, t) - wave(a, aVertex.x-d, aVertex.z, t)) / 2 / d;
	float dz = (wave(a, aVertex.x, aVertex.z+d, t) - wave(a, aVertex.x, aVertex.z-d, t)) / 2 / d;

	vec3 newVertex = vec3(aVertex.x, y, aVertex.z);
	vec3 newNormal = normalize(vec3(-dx, 1, -dz));

	// calculate position
	position = matrixModelView * vec4(newVertex, 1.0);
	gl_Position = matrixProjection * position;

	// calculate normal - don't forget to include!
	normal = normalize(mat3(matrixModelView) * newNormal);

	// calculate texture coordinate
	texCoord0 = aTexCoord;

	// no light calculation for water
	color = vec4(1, 1, 1, 1);

	// calculate reflection coefficient using Schlick's approximation of Fresnel formula
	float cosTheta = dot(normal, normalize(-position.xyz));
	float R0 = 0.02;
	reflFactor = R0 + (1 - R0) * pow(1.0 - cosTheta, 5);

}
