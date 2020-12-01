#include <iostream>
#include "GL/glew.h"
#include "GL/3dgl.h"
#include "GL/glut.h"
#include "GL/freeglut_ext.h"

// Include GLM core features
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;

// 3D Models
C3dglTerrain terrain, water;
C3dglSkyBox skybox;
C3dglModel woodCabin;
C3dglModel ufo;
C3dglModel tree;
C3dglModel boat;
C3dglModel stone;
C3dglModel lamp;

// texture ids
GLuint idTexGrass;		// grass texture
GLuint idTexSand;		// sand texture
GLuint idTexWater;		// water texture
GLuint idTexNone;
GLuint idTexCube;		// cube map
GLuint idTexNormal;		// normal map
GLuint idTexStone;		// stone texture

GLuint idTexParticle;
GLuint idBufferVelocity;
GLuint idBufferStartTime;

// GLSL Objects (Shader Program)
C3dglProgram ProgramBasic;
C3dglProgram ProgramWater;
C3dglProgram ProgramTerrain;
C3dglProgram ProgramParticle;

// Water specific variables
float waterLevel = 4.6f;

// Particle specific variables
const float M_PI = 3.14;
const float PERIOD = 0.00075f;
const float LIFETIME = 6;
const int NPARTICLES = (int)(LIFETIME / PERIOD);

// camera position (for first person type camera navigation)
mat4 matrixView;			// The View Matrix
float angleTilt = 15.f;		// Tilt Angle
vec3 cam(0);				// Camera movement values

bool init()
{
	// switch on: transparency/blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(0x8642);    // !!!!
	glEnable(GL_POINT_SPRITE);

	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!

	// create & load textures
	C3dglBitmap bm;
	glActiveTexture(GL_TEXTURE0);

	// Initialise Shaders
	C3dglShader VertexShader;
	C3dglShader FragmentShader;

	// Basic shaders
	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/basic.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/basic.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramBasic.Create()) return false;
	if (!ProgramBasic.Attach(VertexShader)) return false;
	if (!ProgramBasic.Attach(FragmentShader)) return false;
	if (!ProgramBasic.Link()) return false;
	if (!ProgramBasic.Use(true)) return false;

	// Water shaders
	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/water.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/water.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramWater.Create()) return false;
	if (!ProgramWater.Attach(VertexShader)) return false;
	if (!ProgramWater.Attach(FragmentShader)) return false;
	if (!ProgramWater.Link()) return false;
	if (!ProgramWater.Use(true)) return false;

	// Terrain shaders
	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/terrain.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/terrain.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramTerrain.Create()) return false;
	if (!ProgramTerrain.Attach(VertexShader)) return false;
	if (!ProgramTerrain.Attach(FragmentShader)) return false;
	if (!ProgramTerrain.Link()) return false;
	if (!ProgramTerrain.Use(true)) return false;

	// Particle system shaders
	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/particlesystem.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/particlesystem.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramParticle.Create()) return false;
	if (!ProgramParticle.Attach(VertexShader)) return false;
	if (!ProgramParticle.Attach(FragmentShader)) return false;
	if (!ProgramParticle.Link()) return false;
	if (!ProgramParticle.Use(true)) return false;

	// Re-enable basic shader after setting up all additional shaders
	ProgramBasic.Use();

	// Prepare the particle buffers
	std::vector<float> bufferVelocity;
	std::vector<float> bufferStartTime;
	float time = 0;
	for (int i = 0; i < NPARTICLES; i++)
	{
		float theta = (float)M_PI / 12.f * (float)rand() / (float)RAND_MAX;
		float phi = (float)M_PI * 4.f * (float)rand() / (float)RAND_MAX;
		float x = sin(theta) * cos(phi);
		float y = cos(theta);
		float z = sin(theta) * sin(phi);
		float v = 1 + 0.25f * (float)rand() / (float)RAND_MAX;

		bufferVelocity.push_back(x * v);
		bufferVelocity.push_back(y * v);
		bufferVelocity.push_back(z * v);

		bufferStartTime.push_back(time);
		time += PERIOD;
	}
	glGenBuffers(1, &idBufferVelocity);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferVelocity.size(), &bufferVelocity[0],
		GL_STATIC_DRAW);
	glGenBuffers(1, &idBufferStartTime);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferStartTime.size(), &bufferStartTime[0],
		GL_STATIC_DRAW);

	// glut additional setup
	glutSetVertexAttribCoord3(ProgramBasic.GetAttribLocation("aVertex"));
	glutSetVertexAttribNormal(ProgramBasic.GetAttribLocation("aNormal"));

	// load your 3D models here!
	if (!terrain.loadHeightmap("models\\heightmap3.png", 10)) return false;
	if (!water.loadHeightmap("models\\watermap.png", 10)) return false;

	if (!woodCabin.load("models\\WoodenCabinObj\\WoodenCabin.obj")) return false;
	woodCabin.loadMaterials("models\\WoodenCabinObj");

	if (!ufo.load("models\\saucerObj\\ufo-fixed.obj")) return false;
	ufo.loadMaterials("models\\saucerObj");

	if (!tree.load("models\\Spruce_obj\\Spruce.obj")) return false;
	tree.loadMaterials("models\\Spruce_obj");

	if (!boat.load("models\\OldBoat\\OldBoat.obj")) return false;
	boat.loadMaterials("models\\OldBoat");

	if (!stone.load("models\\stone\\stone.obj")) return false;

	if (!lamp.load("models\\StreetLamp\\streetLamp.obj")) return false;

	// load moon skybox
	if (!skybox.load("models\\skybox2\\front.png", "models\\skybox2\\left.png", "models\\skybox2\\back.png", "models\\skybox2\\right.png", "models\\skybox2\\up.png", "models\\skybox2\\down.png")) return false;

	// load Cube Map
	glActiveTexture(GL_TEXTURE3);
	glGenTextures(3, &idTexCube);
	glBindTexture(GL_TEXTURE_CUBE_MAP, idTexCube);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
	// load moon cube map images
	bm.Load("models\\cube2\\left.png", GL_RGBA); glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, bm.GetWidth(), abs(bm.GetHeight()), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());
	bm.Load("models\\cube2\\right.png", GL_RGBA); glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, bm.GetWidth(), abs(bm.GetHeight()), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());
	bm.Load("models\\cube2\\down.png", GL_RGBA); glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, bm.GetWidth(), abs(bm.GetHeight()), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());
	bm.Load("models\\cube2\\up.png", GL_RGBA); glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, bm.GetWidth(), abs(bm.GetHeight()), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());
	bm.Load("models\\cube2\\front.png", GL_RGBA); glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, bm.GetWidth(), abs(bm.GetHeight()), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());
	bm.Load("models\\cube2\\back.png", GL_RGBA); glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, bm.GetWidth(), abs(bm.GetHeight()), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	// Grass texture
	bm.Load("models/grass.png", GL_RGBA);
	glGenTextures(1, &idTexGrass);
	glBindTexture(GL_TEXTURE_2D, idTexGrass);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	// Sand texture
	bm.Load("models/sand.png", GL_RGBA);
	glGenTextures(1, &idTexSand);
	glBindTexture(GL_TEXTURE_2D, idTexSand);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	// Water texture
	bm.Load("models/water.png", GL_RGBA);
	glGenTextures(1, &idTexWater);
	glBindTexture(GL_TEXTURE_2D, idTexWater);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	// Stone texture
	bm.Load("models/stone/stone.png", GL_RGBA);
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexStone);
	glBindTexture(GL_TEXTURE_2D, idTexStone);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	// Setup the Rain Texture
	bm.Load("models/water.bmp", GL_RGBA);
	glGenTextures(1, &idTexParticle);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, idTexParticle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	// Send the texture info to the shaders
	ProgramBasic.SendUniform("texture0", 0);

	// Setup cube map texture to GL_TEXTURE3
	ProgramBasic.SendUniform("textureCubeMap", 3);

	// Setup water texture to be used by particle system
	ProgramParticle.SendUniform("texture0", 5);

	// Setup normal map texturing
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, idTexNormal);
	ProgramBasic.SendUniform("textureNormal", 4);	

	// setup lights (for basic and terrain programs only, water does not use these lights):
	ProgramBasic.SendUniform("lightAmbient.on", 1);
	ProgramBasic.SendUniform("lightAmbient.color", 0.1, 0.1, 0.1);
	ProgramBasic.SendUniform("lightDir.on", 1);
	ProgramBasic.SendUniform("lightDir.direction", -1.0, 0.5, -1.0);
	ProgramBasic.SendUniform("lightDir.diffuse", 1.0, 1.0, 1.0);

	ProgramBasic.SendUniform("lightPoint1.on", 1);
	ProgramBasic.SendUniform("lightPoint1.position", 3.0f, 10.0f, 2.0f);
	ProgramBasic.SendUniform("lightPoint1.diffuse", 0.001, 0.001, 0.001);

	ProgramBasic.SendUniform("spotLight.on", 1);
	ProgramBasic.SendUniform("spotLight.position", 3.0f, 10.0f, 2.0f);
	ProgramBasic.SendUniform("spotLight.diffuse", 0.8, 1.0, 0.4);
	ProgramBasic.SendUniform("spotLight.direction", 0.0, -1.0, 0.0);
	ProgramBasic.SendUniform("spotLight.cutoff", 30.0f);
	ProgramBasic.SendUniform("spotLight.attenuation", 3.0f);
	
	ProgramTerrain.SendUniform("lightAmbient.on", 1);
	ProgramTerrain.SendUniform("lightAmbient.color", 0.1, 0.1, 0.1);
	ProgramTerrain.SendUniform("lightDir.on", 1);
	ProgramTerrain.SendUniform("lightDir.direction", -1.0, 0.5, -1.0);
	ProgramTerrain.SendUniform("lightDir.diffuse", 1.0, 1.0, 1.0);

	// setup materials (for basic and terrain programs only, water does not use these materials):
	ProgramBasic.SendUniform("materialAmbient", 1.0, 1.0, 1.0);		// full power (note: ambient light is extremely dim)
	ProgramBasic.SendUniform("materialDiffuse", 1.0, 1.0, 1.0);
	ProgramTerrain.SendUniform("materialAmbient", 1.0, 1.0, 1.0);		// full power (note: ambient light is extremely dim)
	ProgramTerrain.SendUniform("materialDiffuse", 1.0, 1.0, 1.0);

	// setup the textures
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, idTexSand);
	ProgramTerrain.SendUniform("textureBed", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, idTexGrass);
	ProgramTerrain.SendUniform("textureShore", 2);

	// setup the water colours and level
	ProgramWater.SendUniform("waterColor", 0.10f, 0.10f, 0.44f);		// daytime colours 0.2f, 0.22f, 0.02f, nighttime colours 0.10f, 0.10f, 0.44f
	ProgramWater.SendUniform("skyColor", 0.66f, 0.66f, 0.66f);			// daytime 0.2f, 0.6f, 1.f, nighttime 0.66f, 0.66f, 0.66f
	ProgramTerrain.SendUniform("waterLevel", waterLevel);

	// setup underwater fog colour and density
	ProgramTerrain.SendUniform("fogColour", 0.2f, 0.22f, 0.02f);	
	ProgramTerrain.SendUniform("fogDensity", 0.2f);

	// Setup fog colour and density - basic affects objects, terrain affects terrain. NOTE: "fogColour" refers to the underwater fog & "scenefogColour" refer to overworld fog
	ProgramBasic.SendUniform("fogColour", 0.1f, 0.1f, 0.1f);			// same as sky colour
	ProgramBasic.SendUniform("fogDensity", 0.1f);						// set to 0 to turn off
	ProgramTerrain.SendUniform("scenefogColour", 0.1f, 0.1f, 0.1f);		// same as sky colour
	ProgramTerrain.SendUniform("scenefogDensity", 0.1f);
	
	// Initialise the View Matrix (initial position of the camera)
	matrixView = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));
	matrixView *= lookAt(
		vec3(4.0, 0.4, 30.0),
		vec3(4.0, 0.4, 0.0),
		vec3(0.0, 1.0, 0.0));

	// setup the screen background colour
	glClearColor(0.2f, 0.6f, 1.f, 1.0f);   // blue sky colour

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << endl;

	return true;
}

void renderReflections(mat4 matrixView, float theta, float Y);

void renderObjects(mat4 matrixView, float theta, float Y);

void prepareCubeMap(float x, float y, float z, float theta);

void prepareParticles(mat4 m, float Y);

void render()
{
	// send the animation time to shaders
	ProgramWater.SendUniform("t", glutGet(GLUT_ELAPSED_TIME) / 1000.f);
	ProgramParticle.SendUniform("time", glutGet(GLUT_ELAPSED_TIME) / 1000.f - 2);

	// calculate the Y position of the camera - above the ground
	float Y = -std::max(terrain.getInterpolatedHeight(inverse(matrixView)[3][0], inverse(matrixView)[3][2]), waterLevel);

	// this global variable controls the animation
	float theta = glutGet(GLUT_ELAPSED_TIME) * 0.01f;

	// call prepareCubeMap using coords of reflective object
	prepareCubeMap(3.0f, Y + 15.0f, 2.0f, theta);

	glCullFace(GL_BACK);

	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// setup the View Matrix (camera)
	mat4 m = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));// switch tilt off
	m = translate(m, cam);												// animate camera motion (controlled by WASD keys)
	m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));			// switch tilt on
	matrixView = m * matrixView;

	// setup View Matrix
	ProgramTerrain.SendUniform("matrixView", matrixView);
		
	// Basic Shader is not currently in use...
	ProgramBasic.Use();

	// render skybox
	m = matrixView;
	ProgramBasic.SendUniform("reflectionPower", 0.0);
	skybox.render(m);
	
	// setup the grass texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexGrass);

	// render the terrain
	ProgramTerrain.Use();
	m = translate(matrixView, vec3(0, Y, 0));
	terrain.render(m);

	// setup the water texture
	glBindTexture(GL_TEXTURE_2D, idTexWater);

	// render the water
	ProgramWater.Use();
	m = translate(matrixView, vec3(0, Y, 0));
	m = translate(m, vec3(0, waterLevel, 0));
	m = scale(m, vec3(0.5f, 1.0f, 0.5f));
	ProgramWater.SendUniform("matrixModelView", m);
	water.render(m);

	renderReflections(matrixView, theta, Y);
	renderObjects(matrixView, theta, Y);
	prepareParticles(m, Y);

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
}

void renderObjects(mat4 matrixView, float theta, float Y)
{
	mat4 m;
	ProgramBasic.Use();

	// normal rendering without reflections
	glActiveTexture(GL_TEXTURE0);
	ProgramBasic.SendUniform("reflectionPower", 0.0);

	// Wooden Cabin
	m = matrixView;
	m = translate(m, vec3(3.0f, Y + 5.3f, 2.0f));
	m = scale(m, vec3(0.05f, 0.05f, 0.05f));
	ProgramBasic.SendUniform("matrixModelView", m);
	woodCabin.render(m);
	
	// Trees
	m = matrixView;
	m = translate(m, vec3(-3.0f, Y + 8.6f, -5.0f));
	m = scale(m, vec3(0.5f, 0.5f, 0.5f));
	ProgramBasic.SendUniform("matrixModelView", m);
	tree.render(m);

	// Boat
	m = matrixView;
	m = translate(m, vec3(-10.0f, Y + 4.6f, 15.0f));
	m = scale(m, vec3(0.1f, 0.1f, 0.1f));
	m = rotate(m, radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
	ProgramBasic.SendUniform("matrixModelView", m);
	boat.render(m);

	// UFO non-reflective
	m = matrixView;
	m = translate(m, vec3(15.0f, Y + 20.0f, 2.0f));
	m = scale(m, vec3(0.15f, 0.15f, 0.15f));
	m = rotate(m, radians(30.f) * theta * 0.1f, vec3(0.0f, 1.0f, 0.0f));
	ProgramBasic.SendUniform("matrixModelView", m);
	ufo.render(m);

	// Stones
	glBindTexture(GL_TEXTURE_2D, idTexStone);
	m = matrixView;
	m = translate(m, vec3(15.0f, Y + 10.2f, 2.0f));
	m = scale(m, vec3(0.015f, 0.015f, 0.015f));
	m = rotate(m, radians(90.f) * theta * 0.1f, vec3(1.0f, 1.0f, 1.0f));
	ProgramBasic.SendUniform("matrixModelView", m);
	stone.render(m);

	m = matrixView;
	m = translate(m, vec3(15.0f, Y + 18.0f, 1.0f));
	m = scale(m, vec3(0.015f, 0.015f, 0.015f));
	m = rotate(m, radians(30.f) * theta * 0.1f, vec3(1.0f, 0.0f, 0.5f));
	ProgramBasic.SendUniform("matrixModelView", m);
	stone.render(m);

	m = matrixView;
	m = translate(m, vec3(13.0f, Y + 9.0f, 4.0f));
	m = scale(m, vec3(0.015f, 0.015f, 0.015f));
	m = rotate(m, radians(100.f) * theta * 0.1f, vec3(0.5f, 0.0f, 0.5f));
	ProgramBasic.SendUniform("matrixModelView", m);
	stone.render(m);

	m = matrixView;
	m = translate(m, vec3(15.0f, Y + 14.0f, -2.5f));
	m = scale(m, vec3(0.015f, 0.015f, 0.015f));
	m = rotate(m, radians(55.f) * theta * 0.1f, vec3(0.5f, 1.0f, 0.5f));
	ProgramBasic.SendUniform("matrixModelView", m);
	stone.render(m);

	// Streelamp
	ProgramBasic.SendUniform("materialDiffuse", 1.0f, 0.0f, 0.0f);
	glGenTextures(1, &idTexNone);
	m = matrixView;
	m = translate(m, vec3(0.0f, Y + 5.2f, 3.5f));
	m = scale(m, vec3(0.025f, 0.025f, 0.025f));
	ProgramBasic.SendUniform("matrixModelView", m);
	lamp.render(m);

	// Lamp bulb
	ProgramBasic.SendUniform("materialDiffuse", 1.0f, 1.0f, 1.0f);
	glGenTextures(1, &idTexNone);
	m = matrixView;
	m = translate(m, vec3(0.0f, Y + 8.25f, 3.5f));
	m = scale(m, vec3(0.15f, 0.15f, 0.15f));
	ProgramBasic.SendUniform("matrixModelView", m);
	glutSolidSphere(2, 32, 32);
	ProgramBasic.SendUniform("materialDiffuse", 0.0f, 0.0f, 0.0f);

}

void renderReflections(mat4 matrixView, float theta, float Y)
{
	mat4 m;
	ProgramBasic.Use();

	// Render with reflections
	ProgramBasic.SendUniform("reflectionPower", 1.0);
	//glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, idTexCube);

	// UFO reflective
	m = matrixView;
	m = translate(m, vec3(3.0f, Y + 15.0f, 2.0f));
	m = scale(m, vec3(0.1f, 0.1f, 0.1f));
	m = rotate(m, radians(-120.f) * theta * 0.1f, vec3(0.0f, 1.0f, 0.0f));
	ProgramBasic.SendUniform("matrixModelView", m);
	ufo.render(m);

	//// sphere reflection test
	//ProgramBasic.SendUniform("spotLight.on", 1);
	//ProgramBasic.SendUniform("materialAmbient", 1.0, 1.0, 1.0);
	//m = matrixView;
	//m = translate(m, vec3(3.0f, Y + 10.0f, 15.0f));
	//m = scale(m, vec3(0.4f, 0.4f, 0.4f));
	//ProgramBasic.SendUniform("matrixModelView", m);
	//glutSolidSphere(2, 32, 32);
}

void prepareCubeMap(float x, float y, float z, float theta)
{
	// Store the current viewport in a safe place
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	int w = viewport[2];
	int h = viewport[3];

	// setup the viewport to 256x256, 90 degrees FoV (Field of View)
	glViewport(0, 0, 256, 256);
	ProgramBasic.SendUniform("matrixProjection", perspective(radians(90.f), 1.0f, 0.02f, 1000.0f));

	// render environment 6 times
	ProgramBasic.SendUniform("reflectionPower", 0.0);
	for (int i = 0; i < 6; ++i)
	{
		// clear background
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// setup the camera
		const GLfloat ROTATION[6][6] =
		{	// at              up
			{ 1.0, 0.0, 0.0,   0.0, -1.0, 0.0 },  // pos x
			{ -1.0, 0.0, 0.0,  0.0, -1.0, 0.0 },  // neg x
			{ 0.0, 1.0, 0.0,   0.0, 0.0, 1.0 },   // pos y
			{ 0.0, -1.0, 0.0,  0.0, 0.0, -1.0 },  // neg y
			{ 0.0, 0.0, 1.0,   0.0, -1.0, 0.0 },  // poz z
			{ 0.0, 0.0, -1.0,  0.0, -1.0, 0.0 }   // neg z
		};
		mat4 matrixView2 = lookAt(
			vec3(x, y, z),
			vec3(x + ROTATION[i][0], y + ROTATION[i][1], z + ROTATION[i][2]),
			vec3(ROTATION[i][3], ROTATION[i][4], ROTATION[i][5]));

		// send the View Matrix
		ProgramBasic.SendUniform("matrixView", matrixView);

		// render scene objects - all but the reflective one
		glActiveTexture(GL_TEXTURE0);
		float Y = -std::max(terrain.getInterpolatedHeight(inverse(matrixView2)[3][0], inverse(matrixView2)[3][2]), waterLevel);
		renderObjects(matrixView2, theta, Y);

		// send the image to the cube texture
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_CUBE_MAP, idTexCube);
		glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8, 0, 0, 256, 256, 0);
	}

	// restore the matrixView, viewport and projection
	void reshape(int w, int h);
	reshape(w, h);
}

void prepareParticles(mat4 m, float Y)
{
	// Setup the particle system
	ProgramParticle.SendUniform("initialPos", -10.0f, Y + 4.6f, 15.0f);
	ProgramParticle.SendUniform("gravity", 0.0, -0.2, 0.0);
	ProgramParticle.SendUniform("particleLifetime", LIFETIME);

	// RENDER THE PARTICLE SYSTEM
	// setup the point size
	glEnable(GL_POINT_SPRITE);
	glPointSize(2);

	// particles
	glDepthMask(GL_FALSE);                // disable depth buffer updates
	glActiveTexture(GL_TEXTURE0);            // choose the active texture
	glBindTexture(GL_TEXTURE_2D, idTexParticle);    // bind the texture

	ProgramParticle.Use();

	m = matrixView;
	ProgramParticle.SendUniform("matrixModelView", m);

	// render the buffer
	glEnableVertexAttribArray(0);    // velocity
	glEnableVertexAttribArray(1);    // start time
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_POINTS, 0, NPARTICLES);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glDepthMask(GL_TRUE);        // don't forget to switch the depth test updates back on
}

// called before window opened or resized - to setup the Projection Matrix
void reshape(int w, int h)
{
	float ratio = w * 1.0f / h;      // we hope that h is not zero
	glViewport(0, 0, w, h);
	mat4 m = perspective(radians(60.f), ratio, 0.02f, 1000.f);
	ProgramBasic.SendUniform("matrixProjection", m);
	ProgramTerrain.SendUniform("matrixProjection", m);
	ProgramWater.SendUniform("matrixProjection", m);
	ProgramParticle.SendUniform("matrixProjection", m);
}

// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': cam.z = std::max(cam.z * 1.05f, 0.01f); break;
	case 's': cam.z = std::min(cam.z * 1.05f, -0.01f); break;
	case 'a': cam.x = std::max(cam.x * 1.05f, 0.01f); break;
	case 'd': cam.x = std::min(cam.x * 1.05f, -0.01f); break;
	case 'e': cam.y = std::max(cam.y * 1.05f, 0.01f); break;
	case 'q': cam.y = std::min(cam.y * 1.05f, -0.01f); break;
	case '1': ProgramBasic.SendUniform("lightPoint1.on", 0); break;
	}
	// speed limit
	cam.x = std::max(-0.15f, std::min(0.15f, cam.x));
	cam.y = std::max(-0.15f, std::min(0.15f, cam.y));
	cam.z = std::max(-0.15f, std::min(0.15f, cam.z));
}

// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': cam.z = 0; break;
	case 'a':
	case 'd': cam.x = 0; break;
	case 'q':
	case 'e': cam.y = 0; break;
	case '1': ProgramBasic.SendUniform("lightPoint1.on", 1); break;
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
bool bJustClicked = false;
void onMouse(int button, int state, int x, int y)
{
	bJustClicked = (state == GLUT_DOWN);
	glutSetCursor(bJustClicked ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_INHERIT);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
}

// handle mouse move
void onMotion(int x, int y)
{
	if (bJustClicked)
		bJustClicked = false;
	else
	{
		glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);

		// find delta (change to) pan & tilt
		float deltaPan = 0.25f * (x - glutGet(GLUT_WINDOW_WIDTH) / 2);
		float deltaTilt = 0.25f * (y - glutGet(GLUT_WINDOW_HEIGHT) / 2);

		// View = Tilt * DeltaPan * Tilt^-1 * DeltaTilt * View;
		angleTilt += deltaTilt;
		mat4 m = mat4(1.f);
		m = rotate(m, radians(angleTilt), vec3(1.f, 0.f, 0.f));
		m = rotate(m, radians(deltaPan), vec3(0.f, 1.f, 0.f));
		m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));
		m = rotate(m, radians(deltaTilt), vec3(1.f, 0.f, 0.f));
		matrixView = m * matrixView;
	}
}

int main(int argc, char **argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("CI5520 3D Graphics Programming");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cerr << "GLEW Error: " << glewGetErrorString(err) << endl;
		return 0;
	}
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;

	// register callbacks
	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);

	cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Version: " << glGetString(GL_VERSION) << endl;

	// init light and everything – not a GLUT or callback function!
	if (!init())
	{
		cerr << "Application failed to initialise" << endl;
		return 0;
	}

	// enter GLUT event processing cycle
	glutMainLoop();


	return 1;
}

