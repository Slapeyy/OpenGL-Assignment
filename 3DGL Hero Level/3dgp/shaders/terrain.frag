#version 330

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;

// Water-related
uniform vec3 waterColor;
uniform vec3 fogColour;

// Input:  Water Related
in float waterDepth;			// water depth (+ for water, - for shore)
in float fogFactor;

// Scene Fog-related
uniform vec3 scenefogColour;
in float scenefogFactor;

// Uniform: The Texture
uniform sampler2D textureBed;
uniform sampler2D textureShore;

// Output Variable (sent down through the Pipeline)
out vec4 outColor;

void main(void) 
{
	outColor = color;

 	// shoreline multitexturing
	float isAboveWater = 1 - clamp(waterDepth, 0, 1); 
	outColor *= mix(texture(textureBed, texCoord0), texture(textureShore, texCoord0), isAboveWater);
	outColor = mix(vec4(fogColour, 1), outColor, fogFactor);
	outColor = mix(vec4(scenefogColour, 1), outColor, scenefogFactor);
}
