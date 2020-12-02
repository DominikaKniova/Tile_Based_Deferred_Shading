//------------------------------------------------------------------------------
//  [PGR2] Tile-base Deffered Shading
//  12/02/2020
//-----------------------------------------------------------------------------
// CONTROLS:
//     [M / m]   ... inc / dec number of cube models\n\
//     [L / l]   ... inc / dec number of lights\n\
//     [R / r]   ... inc / dec maximal radius of lights\n\
//     [S / s]   ... inc / dec side size of a super - pixel\n\
//     [Z / z]   ... move scene along z - axis\n\
//     [d]     ... enable / disable tile based deferred shading\n\
//     [t]     ... enable / disable rendering light low res super - pixels\n\
//     [c]     ... compile shaders\n\
//     [mouse] ... scene rotation(left button)\n\
//-----------------------------------------------------------------------------

//#undef USE_ANTTWEAKBAR
#include "./shared/common.h"
#include "./shared/models/cube.h"
#include "./shared/models/sphere.h"
#include "lights.h"

// GLOBAL VARIABLES____________________________________________________________

// Enable/disable modes (from menu)
bool modeDefferedShading = false;
bool modeLightSuperPixelsOnly = false;

// Shader program ids
GLuint naiveRenderingProgram = 0;
GLuint geometryPassProgram = 0;
GLuint g_LightPassProgram = 0;
GLuint g_LightSphereProgram = 0;
GLuint lightIDSPixelProgram = 0;
GLuint lowResRenderingProgram = 0;
GLuint finalRenderingProgram = 0;

// Number of scene primiteves controls
// number of lights and cubes in the scene chosen by the user (from menu)
int sceneNumLights = 400;
int sceneNumCubes = 5; //number of cubes in a row (actual number of cubes is sceneNumCubes^3)
// number of lights and cubes in previous frame (current number)
int currNumLights = 0;
int currNumCubes = 0;
// side size of super-pixel chosen by the user (from menu) and the size from previous frame
int superPixelSideSize = 16;
int currSPixelSideSize = 0;
// number of super-pixels in both dimensions
unsigned int numSuperPixels_x = 0;
unsigned int numSuperPixels_y = 0;

unsigned int MAX_LIGHTS = 512;
float cubeSize = 2.5f;
float maxLightRadius = 5.0f;
float currMaxLightRadius = 0.0f;

unsigned int currWindowSize_x = Variables::WindowSize.x;
unsigned int currWindowSize_y = Variables::WindowSize.y;

// Model VAOs and VBOs
GLuint cubeModelVAO = 0; 
GLuint cubeModelVBO = 0; 
GLuint sphereModelVAO = 0; 
GLuint sphereModelVBO = 0; 
GLuint quadModelVAO = 0;
GLuint quadModelVBO = 0;

// Scene primitives related variables
// randomly generated scene point lights
std::vector<Light> sceneLights;
// specifies correct translation of cubes to form a 3D matrix
std::vector<glm::vec3> cubeTranslations;
int numSphereVertices = 0;
// variable for animating moving lights
float deltaAngle = 0.0f;

// Scene primitive's attribute buffer objects
GLuint cubeTranslationsBuffer = 0;
// uniform buffer for lights
GLuint lightsUBO = 0;

// Geometry pass variables 
// framebuffer and textures storing closest positions, normals and depth
GLuint geometryPassFBO = 0;
GLuint geometryPassTextures[3] = { 0 };
// render buffer for storing depth of closest positions
GLuint geometryPassRBO = 0;

// Storing light ids for super-pixels phase variables
// shader storage buffer for storing array of light ids for each super-pixel
// size is numSuperPixels_x * numSuperPixels_y * MAX_LIGHTS * sizeof(unsigned int)
GLuint superPixelLightsBuffer = 0;
// texture for storing number of lights that affect a super-pixel
// size is numSuperPixels_x * numSuperPixels_y texels
GLuint superPixelCountTexture = 0;
// array of zeros of size numSuperPixels_x * numSuperPixels_y for clearing superPixelCountTexture
std::vector<unsigned int> zeros;
// framebuffer and texture for rendering lights in super-pixels
// framebuffer resolution is downsized to numSuperPixels_x * numSuperPixels_y
GLuint lowResFB0 = 0;
GLuint lowResTexture = 0;
GLuint lowResRBO = 0;

// IMPLEMENTATION______________________________________________________________

// create VAO for rendering a cube
void createCubeVAO() {
	glGenVertexArrays(1, &cubeModelVAO);
	glBindVertexArray(cubeModelVAO);

	glGenBuffers(1, &cubeModelVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeModelVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Tools::Mesh::CUBE_MAP_VERTEX_ARRAY), Tools::Mesh::CUBE_MAP_VERTEX_ARRAY, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * 3 * sizeof(GLfloat), (void*)(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * 3 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// VBO does not need to be stored anymore -> delete
	glDeleteBuffers(1, &cubeModelVBO);
}

// create VAO for rendering a sphere
void createSphereVAO() {
	// create sphere mesh
	int * num_vertices = &numSphereVertices;
	glm::vec3 * sphere_mesh = Tools::Mesh::CreateSphereMesh(1.0f, 20, 20, num_vertices);

	glGenVertexArrays(1, &sphereModelVAO);
	glBindVertexArray(sphereModelVAO);

	glGenBuffers(1, &sphereModelVBO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereModelVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * numSphereVertices, sphere_mesh, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void *)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// VBO does not need to be stored anymore -> delete
	glDeleteBuffers(1, &sphereModelVBO);
}

// create VAO for rendering a quad
void createQuadVAO() {
	float quad_pos_tex[] = {
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};
	glGenVertexArrays(1, &quadModelVAO);
	glBindVertexArray(quadModelVAO);

	glGenBuffers(1, &quadModelVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadModelVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_pos_tex), &quad_pos_tex, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// VBO does not need to be stored anymore -> delete
	glDeleteBuffers(1, &quadModelVBO);
}

// compute number of super-pixels in both dimensions
void computeNumSuperPixels() {
	numSuperPixels_x = Variables::WindowSize.x / superPixelSideSize;
	numSuperPixels_y = Variables::WindowSize.y / superPixelSideSize;

	// if the screen resolutions are not multiples of the size of super-pixel, then 
	// just increment number of super-pixels in the dimension.
	// last super-pixel in a row (column) will not be filled completely
	if (Variables::WindowSize.x % superPixelSideSize != 0) {
		numSuperPixels_x++;
	}
	if (Variables::WindowSize.y % superPixelSideSize != 0) {
		numSuperPixels_y++;
	}
}

// prepare shader storage for super-pixel -- light ids phase
void createSuperPixelShaderStorage() {
	glDeleteBuffers(1, &superPixelLightsBuffer);
	glGenBuffers(1, &superPixelLightsBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, superPixelLightsBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numSuperPixels_x * numSuperPixels_y * MAX_LIGHTS * sizeof(GL_UNSIGNED_INT), nullptr, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, superPixelLightsBuffer);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

// prepare texture for super-pixel -- light ids phase
void createSuperPixelCountTexture() {
	glDeleteTextures(1, &superPixelCountTexture);
	glGenTextures(1, &superPixelCountTexture);
	glBindTexture(GL_TEXTURE_2D, superPixelCountTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, numSuperPixels_x, numSuperPixels_y);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glBindTexture(GL_TEXTURE_2D, 0);
}

// update light and cube arrays if the number of instances is changed. Also update number of super-pixels if it's side size or window's size is changed
// called every frame
void updateUserData() {

	// do nothing when nothing was changed
	if (currNumLights == sceneNumLights && currNumCubes == sceneNumCubes && currSPixelSideSize == superPixelSideSize &&
		currMaxLightRadius == maxLightRadius && currWindowSize_x == Variables::WindowSize.x && currWindowSize_y == Variables::WindowSize.y) {
		return;
	}

	// number of lights was changed -> generate new
	if (currNumLights != sceneNumLights || currNumCubes != sceneNumCubes) {
		int lights_pos_range = sceneNumCubes * cubeSize;
		generate_lights(sceneNumLights, sceneLights, -lights_pos_range, lights_pos_range, maxLightRadius);
		currNumLights = sceneNumLights;

		// store to uniform buffer object
		glDeleteBuffers(1, &lightsUBO);
		glGenBuffers(1, &lightsUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, lightsUBO);
		glBufferData(GL_UNIFORM_BUFFER, sceneLights.size() * (sizeof(glm::vec4) * 4), NULL, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sceneLights.size() * (sizeof(glm::vec4) * 4), &sceneLights[0]);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	// max light radius changed -> just generate new radius and attenuation factor 
	if (currMaxLightRadius != maxLightRadius) {
		for (int i = 0; i < sceneLights.size(); i++) {
			sceneLights[i].radius.x = glm::compRand1(1.0f, maxLightRadius);
			sceneLights[i].color_attenuation.w = 1.0 / (sceneLights[i].radius.x*sceneLights[i].radius.x * 0.1f);
		}
		currMaxLightRadius = maxLightRadius;
	}

	// number of cubes was changed -> generate new translations
	if (currNumCubes != sceneNumCubes) {
		cubeTranslations.resize(sceneNumCubes * sceneNumCubes * sceneNumCubes);
		int idx = 0;
		for (int x = -sceneNumCubes; x < sceneNumCubes; x += 2) {
			for (int y = -sceneNumCubes; y < sceneNumCubes; y += 2) {
				for (int z = -sceneNumCubes; z < sceneNumCubes; z += 2) {
					glm::vec3 translation = glm::vec3(x * cubeSize, y * cubeSize, z * cubeSize);
					cubeTranslations[idx] = translation;
					idx++;
				}
			}

		}
		currNumCubes = sceneNumCubes;

		// store attributes in buffer
		glDeleteBuffers(1, &cubeTranslationsBuffer);

		glGenBuffers(1, &cubeTranslationsBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, cubeTranslationsBuffer);
		glBufferData(GL_ARRAY_BUFFER, cubeTranslations.size() * sizeof(glm::vec3), &cubeTranslations[0].x, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// side size of a super-pixel was changed -> recompute number of super-pixels and create new shader storages
	if (currSPixelSideSize != superPixelSideSize || 
		currWindowSize_x != Variables::WindowSize.x || currWindowSize_y != Variables::WindowSize.y) {
		computeNumSuperPixels();
		createSuperPixelCountTexture();
		createSuperPixelShaderStorage();
		currSPixelSideSize = superPixelSideSize;
		zeros.resize(numSuperPixels_x*numSuperPixels_y);
	}
}

// update positions of scene lights. Called every frame
void updateLightPositions() {
	deltaAngle += 1.0f;
	if (deltaAngle >= 360.0f) {
		deltaAngle = 0.0f;
	}
	float cos_t = glm::cos(glm::radians(deltaAngle));
	float sin_t = glm::sin(glm::radians(deltaAngle));

	for (int i = 0; i < sceneLights.size(); i++) {
		if (i % 2) {
			sceneLights[i].curr_pos_id.x = sceneLights[i].original_pos_radius.w * (cos_t * sceneLights[i].original_pos_radius.x - sin_t * sceneLights[i].original_pos_radius.y);
			sceneLights[i].curr_pos_id.y = sceneLights[i].original_pos_radius.w * (sin_t * sceneLights[i].original_pos_radius.x + cos_t * sceneLights[i].original_pos_radius.y);
		}
		else {
			sceneLights[i].curr_pos_id.x = sceneLights[i].original_pos_radius.w * (cos_t * sceneLights[i].original_pos_radius.x - sin_t * sceneLights[i].original_pos_radius.z);
			sceneLights[i].curr_pos_id.z = sceneLights[i].original_pos_radius.w * (sin_t * sceneLights[i].original_pos_radius.x + cos_t * sceneLights[i].original_pos_radius.z);
		}
	}
	// update uniform buffer
	glBindBuffer(GL_UNIFORM_BUFFER, lightsUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sceneLights.size() * (sizeof(glm::vec4) * 3 + sizeof(glm::vec4)), &sceneLights[0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

// render scene with standard shading (naive approach)
void standardNaiveRender() {
	// Clear frame buffer and set OpenGL states
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Update shader program
	glUseProgram(naiveRenderingProgram);

	glUniform1i(glGetUniformLocation(naiveRenderingProgram, "u_NumModels"), sceneNumCubes);
	glUniform1i(glGetUniformLocation(naiveRenderingProgram, "num_Lights"), sceneNumLights);

	// Specify model geometry and send it to GPU
	glBindVertexArray(cubeModelVAO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeTranslationsBuffer);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glVertexAttribDivisor(3, 1);
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightsUBO);

	// draw using instanced rendering
	glDrawArraysInstanced(GL_TRIANGLES, 0, sizeof(Tools::Mesh::CUBE_MAP_VERTEX_ARRAY) / (6 * sizeof(GLfloat)), sceneNumCubes * sceneNumCubes * sceneNumCubes);

	glBindVertexArray(0);
	glUseProgram(0);
}

// prepare framebuffer and textures for geometry pass phase
void initGeometryPass() {
	glDeleteFramebuffers(1, &geometryPassFBO);
	glDeleteTextures(1, &geometryPassTextures[0]);
	glDeleteTextures(1, &geometryPassTextures[1]);
	glDeleteRenderbuffers(1, &geometryPassRBO);

	glGenFramebuffers(1, &geometryPassFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, geometryPassFBO);

	glGenTextures(1, &geometryPassTextures[0]);
	glBindTexture(GL_TEXTURE_2D, geometryPassTextures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, Variables::WindowSize.x, Variables::WindowSize.y, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, geometryPassTextures[0], 0);

	glGenTextures(1, &geometryPassTextures[1]);
	glBindTexture(GL_TEXTURE_2D, geometryPassTextures[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, Variables::WindowSize.x, Variables::WindowSize.y, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, geometryPassTextures[1], 0);

	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	glGenRenderbuffers(1, &geometryPassRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, geometryPassRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Variables::WindowSize.x, Variables::WindowSize.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, geometryPassRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// geometry pass - store closest positions, normals and other information in framebuffer textures
void geometryPassRender() {
	glUseProgram(geometryPassProgram);

	glBindFramebuffer(GL_FRAMEBUFFER, geometryPassFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(cubeModelVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeTranslationsBuffer);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glVertexAttribDivisor(3, 1);
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightsUBO);

	glDrawArraysInstanced(GL_TRIANGLES, 0, sizeof(Tools::Mesh::CUBE_MAP_VERTEX_ARRAY) / (6 * sizeof(GLfloat)), sceneNumCubes * sceneNumCubes * sceneNumCubes);

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
}

// initialize rendering in lower resolution that for each super-pixel finds lights that affect it
// and stores information in SSBO and texture
void initSuperPixelLightPhase() {
	glDeleteFramebuffers(1, &lowResFB0);
	glDeleteTextures(1, &lowResTexture);

	glGenFramebuffers(1, &lowResFB0);
	glBindFramebuffer(GL_FRAMEBUFFER, lowResFB0);

	glGenTextures(1, &lowResTexture);
	glBindTexture(GL_TEXTURE_2D, lowResTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, numSuperPixels_x, numSuperPixels_y, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lowResTexture, 0);

	unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	glGenRenderbuffers(1, &lowResRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, lowResRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, numSuperPixels_x, numSuperPixels_y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, lowResRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//for each super-pixel find lights that affect it and store information in SSBO and texture
void superPixelLightPhase() {
	// working with super-pixels now -> downsize resolution
	glViewport(0, 0, numSuperPixels_x, numSuperPixels_y);

	glUseProgram(lightIDSPixelProgram);

	// copy depth buffer from geometry pass framebuffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, geometryPassFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lowResFB0); 
	glBlitFramebuffer(0, 0, Variables::WindowSize.x, Variables::WindowSize.y, 0, 0, numSuperPixels_x, numSuperPixels_y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT);																		  

	glBindVertexArray(sphereModelVAO);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightsUBO);
	glBindImageTexture(0, superPixelCountTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);

	glUniform1i(glGetUniformLocation(lightIDSPixelProgram, "numSuperPixels_x"), numSuperPixels_x);
	glUniform1i(glGetUniformLocation(lightIDSPixelProgram, "numLights"), sceneNumLights);
	glUniform1i(glGetUniformLocation(lightIDSPixelProgram, "maxNumLights"), MAX_LIGHTS);

	for (int i = 0; i < sceneLights.size(); i++){
		glUniform1i(glGetUniformLocation(lightIDSPixelProgram, "lightID"), i);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, numSphereVertices);
	}

	//glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, num_sphere_verts, point_lights.size());
	glBindVertexArray(0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glUseProgram(0);

	// change resolution back to original
	glViewport(0, 0, Variables::WindowSize.x, Variables::WindowSize.y);
}

// render low resolution super-pixels with lights
void renderLightSuperPixelsOnly() {
	glUseProgram(lowResRenderingProgram);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, lowResTexture);

	glUniform1i(glGetUniformLocation(lowResRenderingProgram, "pos_texture"), 0);

	glBindVertexArray(quadModelVAO);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightsUBO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	glUseProgram(0);
}

// render scene using information from geometry pass and light ids in super-pixels
void finalTileBasedRendering(){
	glDisable(GL_DEPTH_TEST);
	glUseProgram(finalRenderingProgram);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, geometryPassTextures[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, geometryPassTextures[1]);

	glBindImageTexture(0, superPixelCountTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);

	glUniform1i(glGetUniformLocation(finalRenderingProgram, "posTexture"), 0);
	glUniform1i(glGetUniformLocation(finalRenderingProgram, "normTexture"), 1);
	glUniform1i(glGetUniformLocation(finalRenderingProgram, "maxNumLights"), MAX_LIGHTS);
	glUniform1i(glGetUniformLocation(finalRenderingProgram, "numSuperPixels_x"), numSuperPixels_x);
	glUniform1i(glGetUniformLocation(finalRenderingProgram, "superPixelSideSize"), superPixelSideSize);

	glBindVertexArray(quadModelVAO);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightsUBO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);

	glUseProgram(0);
	glEnable(GL_DEPTH_TEST);
}

//-----------------------------------------------------------------------------
// Name: display()
// Desc: 
//-----------------------------------------------------------------------------
void display() {
    // update user data if number of models changed and update light positions
    updateUserData();
	updateLightPositions();

	if (!modeDefferedShading) {
		// render in standard way
		standardNaiveRender();
	}
	else {
		// 1. geometry pass phase
		initGeometryPass();
		geometryPassRender();

		// clear texture for storing number of lights that affect a super-pixel
		glBindTexture(GL_TEXTURE_2D, superPixelCountTexture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, numSuperPixels_x, numSuperPixels_y, GL_RED_INTEGER, GL_UNSIGNED_INT, &zeros[0]);
		glBindTexture(GL_TEXTURE_2D, 0);

		// 2. gathering affecting lights for each super-pixel phase
		initSuperPixelLightPhase();
		superPixelLightPhase();
		
		// 3. final rendering
		if (modeLightSuperPixelsOnly) {
			renderLightSuperPixelsOnly();
		}
		else {
			finalTileBasedRendering();
		}
	}
	
	
}

//-----------------------------------------------------------------------------
// Name: initGL()
// Desc: 
//-----------------------------------------------------------------------------
void initGL() {
    // set default camera's distance from the scene
    Variables::Shader::SceneZOffset = 26.0f;

    // set OpenGL state variables
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

    // load shader program
    compileShaders();

	// initialize primitives for rendering
	createCubeVAO();
	createSphereVAO();
	createQuadVAO();
}


// Include GUI and control stuff
#include "controls.hpp"
