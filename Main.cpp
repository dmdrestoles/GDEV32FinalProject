/**
 * This is a program that renders a 3D scene with static and dynamic objects.
 * @author Dan Mark D. Restoles (184116)
 * @version 2022-03-19 v2
 */

 /*
 I have not discussed the C++ language code in my program with anyone other than my instructor or the teaching assistants assigned to this course.

 I have not used C++ language code obtained from another student, or any other unauthorized source, either modified or unmodified.

 If any C++ language code or documentation used in my program was obtained from another source, such as a textbook or course notes, that has been clearly noted with a proper citation in the comments of my program.
 */

 // Quick note: GLAD needs to be included first before GLFW.
 // Otherwise, GLAD will complain about gl.h being already included.
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm>

// Include stb_image for loading images
// Remember to define STB_IMAGE_IMPLEMENTATION first before including
// to avoid linker errors
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// We include glm to give us access to vectors (glm::vec3) and matrices (glm::mat4)
#include <glm/glm.hpp>
// This gives us access to convenience functions for constructing transformation matrices
#include <glm/gtc/matrix_transform.hpp>
// This gives us access to the glm::value_ptr() function, which converts a vector/matrix to a pointer that OpenGL accepts
#include <glm/gtc/type_ptr.hpp>

// ---------------
// Function declarations
// ---------------

/**
 * @brief Creates a shader program based on the provided file paths for the vertex and fragment shaders.
 * @param[in] vertexShaderFilePath Vertex shader file path
 * @param[in] fragmentShaderFilePath Fragment shader file path
 * @return OpenGL handle to the created shader program
 */
GLuint CreateShaderProgram(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath);

/**
 * @brief Creates a shader based on the provided shader type and the path to the file containing the shader source.
 * @param[in] shaderType Shader type
 * @param[in] shaderFilePath Path to the file containing the shader source
 * @return OpenGL handle to the created shader
 */
GLuint CreateShaderFromFile(const GLuint& shaderType, const std::string& shaderFilePath);

/**
 * @brief Creates a shader based on the provided shader type and the string containing the shader source.
 * @param[in] shaderType Shader type
 * @param[in] shaderSource Shader source string
 * @return OpenGL handle to the created shader
 */
GLuint CreateShaderFromSource(const GLuint& shaderType, const std::string& shaderSource);

/**
 * @brief Function for handling the event when the size of the framebuffer changed.
 * @param[in] window Reference to the window
 * @param[in] width New width
 * @param[in] height New height
 */
void FramebufferSizeChangedCallback(GLFWwindow* window, int width, int height);

void CrossProduct (float cross[3], float origin[3], float start[3], float end[3])
{
	float a[3] = { start[0] - origin[0], start[1] - origin[1], start[2] - origin[2] };
	float b[3] = { end[0] - origin[0], end[1] - origin[1], end[2] - origin[2] };

	cross[0] = (a[1] * b[2]) - (a[2] * b[1]);
	cross[1] = (a[0] * b[2]) - (a[2] * b[0]);
	cross[2] = (a[0] * b[1]) - (a[1] * b[0]);
}
/**
 * Struct containing data about a vertex
 */
struct Vertex
{
	GLfloat x, y, z;	// Position
	GLfloat nx, ny, nz; // Normals
	GLubyte r, g, b;	// Color
	GLfloat u, v;		// UV-coordinates

	Vertex()
	{

	}

	Vertex(GLfloat cx, GLfloat cy, GLfloat cz, GLfloat cnx, GLfloat cny, GLfloat cnz, GLubyte cr, GLubyte cg, GLfloat cb, GLfloat cu, GLfloat cv)
	{
		x = cx;
		y = cy;
		z = cz;
		nx = cnx;
		ny = cny;
		nz = cnz;
		r = cr;
		g = cg;
		b = cb;
		u = cu;
		v = cv;
	}

	Vertex(GLfloat vertex[3], GLfloat normals[3], GLubyte cr, GLubyte cg, GLfloat cb, GLfloat cu, GLfloat cv)
	{
		x = vertex[0];
		y = vertex[1];
		z = vertex[2];
		nx = normals[0];
		ny = normals[1];
		nz = normals[2];
		r = cr;
		g = cg;
		b = cb;
		u = cu;
		v = cv;
	}
};

struct Planet {
	GLfloat radius, majorAxis, minorAxis, angle, speed, eccentricity;
	GLfloat cx, cy, cz;
	std::string textureMap;
	GLuint texture;
	
	Planet(){
		radius = 1.0f;
		majorAxis = 1.0f;
		minorAxis = 1.0f;
		angle = 0.f;
		cx = 0.f;
		cy = 0.f;
		cz = 0.f;
		speed = 90.0f;
	}

	Planet(float r, float m1, float m2, float a, float s, float x, float y, float z) {
		radius = r;
		majorAxis = m1;
		minorAxis = m2;
		angle = a;
		speed = s;
		cx = x;
		cy = y;
		cz = z;
	}

	void ComputeMinorAxis() {
		minorAxis = majorAxis * sqrt(1 - (pow(eccentricity, 2)));
	}

	void LoadTexture() {
		glGenTextures(1, &texture);
		stbi_set_flip_vertically_on_load(true);

		int imageWidth, imageHeight, numChannels;

		unsigned char* imageData = stbi_load(textureMap.c_str(), &imageWidth, &imageHeight, &numChannels, 0);

		// Make sure that we actually loaded the image before uploading the data to the GPU
		if (imageData != nullptr)
		{
			// Our texture is 2D, so we bind our texture to the GL_TEXTURE_2D target
			glBindTexture(GL_TEXTURE_2D, texture);

			// Set the filtering methods for magnification and minification
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			// Set the wrapping method for the s-axis (x-axis) and t-axis (y-axis)
			// Try experimenting with different wrapping methods introduced in the textures slide set
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			// Upload the image data to GPU memory
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);

			// If we set minification to use mipmaps, we can tell OpenGL to generate the mipmaps for us
			//glGenerateMipmap(GL_TEXTURE_2D);

			// Once we have copied the data over to the GPU, we can delete
			// the data on the CPU side, since we won't be using it anymore
			stbi_image_free(imageData);
			imageData = nullptr;
		}
		else
		{
			std::cerr << "Failed to load white.jpg" << std::endl;
		}
	}
};

float moveConstant = 50.0f;
const float PI = acos(-1);

void ProcessMovement(GLFWwindow* window, glm::vec3& eye, glm::vec3& target, glm::vec3& up, float moveSpeed)
{
	int wKey = glfwGetKey(window, GLFW_KEY_W);
	int sKey = glfwGetKey(window, GLFW_KEY_S);
	int aKey = glfwGetKey(window, GLFW_KEY_A);
	int dKey = glfwGetKey(window, GLFW_KEY_D);
	int shiftKey = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);

	if (sKey == GLFW_PRESS)
	{
		eye -= moveSpeed * target;
	}

	if (wKey == GLFW_PRESS)
	{
		eye += moveSpeed * target;
	}

	if (dKey == GLFW_PRESS)
	{
		eye += glm::normalize(glm::cross(target, up)) * moveSpeed;
	}

	if (aKey == GLFW_PRESS)
	{
		eye -= glm::normalize(glm::cross(target, up)) * moveSpeed;
	}

	if (shiftKey == GLFW_PRESS)
	{
		moveConstant = 100.0f;
	}

	if (shiftKey == GLFW_RELEASE)
	{
		moveConstant = 50.0f;
	}
}

bool initialMouseInput = true;

double xMousePos, yMousePos, yaw, pitch;
float sensitivity = 0.1f;
glm::vec3 target;

void ProcessMouse(GLFWwindow* window, double xpos, double ypos)
{

	if (initialMouseInput)
	{
		xMousePos = xpos;
		yMousePos = ypos;
		initialMouseInput = false;
	}

	float dx = xpos - xMousePos;
	float dy = yMousePos - ypos;
	xMousePos = xpos;
	yMousePos = ypos;

	dx *= sensitivity;
	dy *= sensitivity;

	yaw += dx;
	pitch += dy;

	if (pitch > 89.0f)
	{
		pitch = 89.0f;
	}
	if (pitch < -89.0f)
	{
		pitch = -89.0f;
	}

	glm::vec3 lookField;
	lookField.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	lookField.y = sin(glm::radians(pitch));
	lookField.z = sin(glm::radians(yaw))* cos(glm::radians(pitch));

	target = glm::normalize(lookField);
}

float revolutionSpeed = 1.f;
const float distScale = 1.f;
void ProcessRevolutionSpeed(GLFWwindow* window, float& revolutionSpeed) {
	int upKey = glfwGetKey(window, GLFW_KEY_UP);
	int downKey = glfwGetKey(window, GLFW_KEY_DOWN);
	int rKey = glfwGetKey(window, GLFW_KEY_R);

	if (upKey == GLFW_PRESS)
	{
		revolutionSpeed += 1.0f;
	}
	if (downKey == GLFW_PRESS) {
		revolutionSpeed -= 1.0f;
	}
	if (rKey == GLFW_PRESS) {
		revolutionSpeed = 1.0f;
	}

	revolutionSpeed = fmax(revolutionSpeed, 1.0f);
}

void GenerateSphereVertices(std::vector<Vertex>& vertices, std::vector<int>& indices, float radius, int sectorCount, int stackCount, float color[3])
{
	// xyz, rgb, uv
	// clear memory of prev arrays
	Vertex v;

	float xy;
	float lengthInv = 1.0f / radius;

	float sectorStep = 2 * PI / sectorCount;
	float stackStep = PI / stackCount;
	float sectorAngle, stackAngle;

	for (int i = 0; i <= stackCount; ++i)
	{
		stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
		xy = radius * cosf(stackAngle);             // r * cos(u)
		v.z = radius * sinf(stackAngle);              // r * sin(u)

		// add (sectorCount+1) vertices per stack
		// the first and last vertices have same position and normal, but different tex coords
		for (int j = 0; j <= sectorCount; ++j)
		{
			sectorAngle = j * sectorStep;           // starting from 0 to 2pi

			// vertex position (x, y, z)
			v.x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
			v.y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)

			v.nx = v.x * lengthInv;
			v.ny = v.y * lengthInv;
			v.nz = v.z * lengthInv;

			// vertex color (r, g, b)
			
			v.r = color[0];
			v.g = color[1];
			v.b = color[2];

			// vertex tex coord (s, t) range between [0, 1]
			v.u = (float)j / sectorCount;
			v.v = (float)i / stackCount;

			vertices.push_back(v);
		}
	}
	;
	int k1, k2;
	for (int i = 0; i < stackCount; ++i)
	{
		k1 = i * (sectorCount + 1);     // beginning of current stack
		k2 = k1 + sectorCount + 1;      // beginning of next stack

		for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
		{
			// 2 triangles per sector excluding first and last stacks
			// k1 => k2 => k1+1
			if (i != 0)
			{
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			// k1+1 => k2 => k2+1
			if (i != (stackCount - 1))
			{
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}
		}
	}
}

unsigned int LoadCubeMap(std::vector<std::string> faces) {
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

std::vector<Planet> planets;
void SetPlanetInfo() {

	Planet mercury;
	mercury.radius = 0.244f;
	mercury.majorAxis = 5.7f * distScale;
	mercury.eccentricity = 0.205f;
	mercury.ComputeMinorAxis();
	mercury.speed = 4.15f ;
	mercury.textureMap = "mercury.jpg";
	mercury.LoadTexture();
	mercury.cx = 0.f;
	mercury.cy = 0.f;
	mercury.cz = 0.f;
	mercury.angle = 0.0f;

	Planet venus;
	venus.radius = 0.6502f;
	venus.majorAxis = 10.8f * distScale;
	venus.eccentricity = 0.007;
	venus.ComputeMinorAxis();
	venus.speed = 1.62;
	venus.textureMap = "venus.jpg";
	venus.LoadTexture();
	venus.cx = 0.f;
	venus.cy = 0.f;
	venus.cz = 0.f;
	venus.angle = 0.f;
	
	Planet earth;
	earth.radius = 0.6371f;
	earth.majorAxis = 14.9f * distScale;
	earth.eccentricity = 0.017;
	earth.ComputeMinorAxis();
	earth.speed = 1;
	earth.textureMap = "earth.jpg";
	earth.LoadTexture();
	earth.cx = 0.f;
	earth.cy = 0.f;
	earth.cz = 0.f;
	earth.angle = 0.f;

	Planet mars;
	mars.radius = 0.339f;
	mars.majorAxis = 22.8f * distScale;
	mars.eccentricity = 0.093;
	mars.ComputeMinorAxis();
	mars.speed = 0.53f;
	mars.textureMap = "mars.jpg";
	mars.LoadTexture();
	mars.cx = 0.f;
	mars.cy = 0.f;
	mars.cz = 0.f;
	mars.angle = 0.f;

	Planet jupiter;
	jupiter.radius = 6.991f;
	jupiter.majorAxis = 89.f * distScale;
	jupiter.eccentricity = 0.084;
	jupiter.ComputeMinorAxis();
	jupiter.speed = 0.08f;
	jupiter.textureMap = "jupiter.jpg";
	jupiter.LoadTexture();
	jupiter.cx = 0.f;
	jupiter.cy = 0.f;
	jupiter.cz = 0.f;
	jupiter.angle = 0.f;

	Planet saturn;
	saturn.radius = 5.8232f;
	saturn.majorAxis = 143.7f * distScale;
	saturn.eccentricity = 0.054f;
	saturn.ComputeMinorAxis();
	saturn.speed = 0.03f;
	saturn.textureMap = "saturn.jpg";
	saturn.LoadTexture();
	saturn.cx = 0.f;
	saturn.cy = 0.f;
	saturn.cz = 0.f;
	saturn.angle = 0.f;

	Planet uranus;
	uranus.radius = 2.5362f;
	uranus.majorAxis = 287.1f * distScale;
	uranus.eccentricity = 0.047f;
	uranus.ComputeMinorAxis();
	uranus.speed = 0.0119f;
	uranus.textureMap = "uranus.jpg";
	uranus.LoadTexture();
	uranus.cx = 0.f;
	uranus.cy = 0.f;
	uranus.cz = 0.f;
	uranus.angle = 0.f;

	Planet neptune;
	neptune.radius = 2.4622f;
	neptune.majorAxis = 453.0f * distScale;
	neptune.eccentricity = 0.008f;
	neptune.ComputeMinorAxis();
	neptune.speed = 0.0061f;
	neptune.textureMap = "neptune.jpg";
	neptune.LoadTexture();
	neptune.cx = 0.f;
	neptune.cy = 0.f;
	neptune.cz = 0.f;
	neptune.angle = 0.f;

	planets.push_back(mercury);
	planets.push_back(venus);
	planets.push_back(earth);
	planets.push_back(mars);
	planets.push_back(jupiter);
	planets.push_back(saturn);
	planets.push_back(uranus);
	planets.push_back(neptune);
}
/**
 * @brief Main function
 * @return An integer indicating whether the program ended successfully or not.
 * A value of 0 indicates the program ended succesfully, while a non-zero value indicates
 * something wrong happened during execution.
 */
int main()
{
	// Initialize GLFW
	int glfwInitStatus = glfwInit();
	if (glfwInitStatus == GLFW_FALSE)
	{
		std::cerr << "Failed to initialize GLFW!" << std::endl;
		return 1;
	}

	// Tell GLFW that we prefer to use OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// Tell GLFW that we prefer to use the modern OpenGL
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Tell GLFW to create a window
	int windowWidth = 800;
	int windowHeight = 600;
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Hello Triangle", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cerr << "Failed to create GLFW window!" << std::endl;
		glfwTerminate();
		return 1;
	}

	// Tell GLFW to use the OpenGL context that was assigned to the window that we just created
	glfwMakeContextCurrent(window);

	// Register the callback function that handles when the framebuffer size has changed
	glfwSetFramebufferSizeCallback(window, FramebufferSizeChangedCallback);

	// Tell GLAD to load the OpenGL function pointers
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cerr << "Failed to initialize GLAD!" << std::endl;
		return 1;
	}

	// --- Vertex specification ---
	float v0[3] = { -0.5f, -0.5f, -0.5f };
	float v1[3] = { -0.5f, -0.5f, 0.5f };
	float v2[3] = { -0.5f, 0.5f, -0.5f };
	float v3[3] = { -0.5f, 0.5f, 0.5f };
	float v4[3] = { 0.5f, -0.5f, -0.5f };
	float v5[3] = { 0.5f, -0.5f, 0.5f };
	float v6[3] = { 0.5f, 0.5f, -0.5f };
	float v7[3] = { 0.5f, 0.5f, 0.5f };

	float xP[3] = { 1.0f, 0.0f, 0.0f };
	float xN[3] = { -1.0f, 0.0f, 0.0f };
	float yP[3] = { 0.0f, 1.0f, 0.0f };
	float yN[3] = { 0.0f, -1.0f, 0.0f };
	float zP[3] = { 0.0f, 0.0f, 1.0f };
	float zN[3] = { 0.0f, 0.0f, -1.0f };

	float center0[3] = { 0.f, 0.f, 0.5f };
	float center1[3] = { -0.5f, 0.f, 0.f };
	float center2[3] = { 0.f, 0.f, -0.5f };
	float center3[3] = { 0.5f, 0.f, 0.f };
	float center4[3] = { 0.f, 0.5f, 0.f };
	float center5[3] = { 0.f, -0.5f, 0.f };

	// Set up the data for each vertex of the quad
	// These vertices are in LOCAL SPACE
	Vertex vertices[36];
					// Position				// Normals				// Color				// UV
	vertices[0] = { center0,		zP,		255,255,255,		0.5f, 0.5f };
	vertices[1] = { v7,				zP,		255,255,255,		1.0f, 1.0f };
	vertices[2] = { v3,				zP,		255,255,255,		0.0f, 1.0f };
	vertices[3] = { v1,				zP,		255,255,255,		0.0f, 0.0f };
	vertices[4] = { v5,				zP,		255,255,255,		1.0f, 0.0f };
	vertices[5] = { v7,				zP,		255,255,255,		1.0f, 1.0f };

	vertices[6] = { center1,		xN,		255,255,255,		0.5f, 0.5f };
	vertices[7] = { v2,				xN,		255,255,255,		1.0f, 1.0f };
	vertices[8] = { v3,				xN,		255,255,255,		0.0f, 1.0f };
	vertices[9] = { v1,				xN,		255,255,255,		0.0f, 0.0f };
	vertices[10] = { v0,			xN,		255,255,255,		1.0f, 0.0f };
	vertices[11] = { v2,			xN,		255,255,255,		1.0f, 1.0f };

	vertices[12] = { center2,		zN,		255,255,255,		0.5f, 0.5f };
	vertices[13] = { v6,			zN,		255,255,255,		1.0f, 1.0f };
	vertices[14] = { v2,			zN,		255,255,255,		0.0f, 1.0f };
	vertices[15] = { v0,			zN,		255,255,255,		0.0f, 0.0f };
	vertices[16] = { v4,			zN,		255,255,255,		1.0f, 0.0f };
	vertices[17] = { v6,			zN,		255,255,255,		1.0f, 1.0f };

	vertices[18] = { center3,		xP,		255,255,255,		0.5f, 0.5f };
	vertices[19] = { v6,			xP,		255,255,255,		1.0f, 1.0f };
	vertices[20] = { v7,			xP,		255,255,255,		0.0f, 1.0f };
	vertices[21] = { v5,			xP,		255,255,255,		0.0f, 0.0f };
	vertices[22] = { v4,			xP,		255,255,255,		1.0f, 0.0f };
	vertices[23] = { v6,			xP,		255,255,255,		1.0f, 1.0f };

	vertices[24] = { center4,		yP,		255,255,255,		0.5f, 0.5f };
	vertices[25] = { v6,			yP,		255,255,255,		1.0f, 1.0f };
	vertices[26] = { v2,			yP,		255,255,255,		0.0f, 1.0f };
	vertices[27] = { v3,			yP,		255,255,255,		0.0f, 0.0f };
	vertices[28] = { v7,			yP,		255,255,255,		1.0f, 0.0f };
	vertices[29] = { v6,			yP,		255,255,255,		1.0f, 1.0f };

	vertices[30] = { center5,		yN,		255,255,255,		0.5f, 0.5f };
	vertices[31] = { v4,			yN,		255,255,255,		1.0f, 1.0f };
	vertices[32] = { v0,			yN,		255,255,255,		0.0f, 1.0f };
	vertices[33] = { v1,			yN,		255,255,255,		0.0f, 0.0f };
	vertices[34] = { v5,			yN,		255,255,255,		1.0f, 0.0f };
	vertices[35] = { v4,			yN,		255,255,255,		1.0f, 1.0f };

	std::vector<Vertex> sphereVertices;
	std::vector<int> sphereIndices;
	float sphereColor[3] = { 255, 255, 255 };

	GenerateSphereVertices(sphereVertices, sphereIndices, 1.0f, 36, 18, sphereColor);

	// Create a vertex buffer object (VBO), and upload our vertices data to the VBO
	GLuint vbo1, vbo2;
	glGenBuffers(1, &vbo1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo2);
	glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(Vertex), sphereVertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(int), sphereIndices.data(), GL_STATIC_DRAW);

	// Create a vertex array object that contains data on how to map vertex attributes
	// (e.g., position, color) to vertex shader properties.
	GLuint vao1, vao2, sunVAO;
	glGenVertexArrays(1, &vao1);
	glBindVertexArray(vao1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo1);

	// Vertex attribute 0 - Position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	// Vertex attribute 1 - Normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, nx)));

	// Vertex attribute 2 - Color
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)(offsetof(Vertex, r)));

	// Vertex attribute 3 - UV-coordinates
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, u)));
	
	glGenVertexArrays(1, &vao2);
	glBindVertexArray(vao2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo2);
	glEnableVertexAttribArray(0);

	// Vertex attribute 0 - Position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	// Vertex attribute 1 - Normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, nx)));

	// Vertex attribute 2 - Color
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)(offsetof(Vertex, r)));

	// Vertex attribute 3 - UV-coordinates
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, u)));
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &sunVAO);
	glBindVertexArray(sunVAO);
	glBindBuffer(GL_ARRAY_BUFFER, vbo2);
	// Vertex attribute 0 - Position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	// Vertex attribute 1 - Normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, nx)));

	// Vertex attribute 2 - Color
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)(offsetof(Vertex, r)));

	// Vertex attribute 3 - UV-coordinates
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, u)));
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	std::vector<std::string> faces{
		"px.png",
		"nx.png",
		"py.png",
		"ny.png",
		"pz.png",
		"nz.png"
	};

	unsigned int cubemapTexture = LoadCubeMap(faces);
	// Create a variable that will contain the ID of our first texture (eye.jpg),
	// and use glGenTextures() to generate the texture itself
	GLuint tex0;
	glGenTextures(1, &tex0);

	// --- Load our pepe.jpg image using stb_image ---

	// Im image-space (pixels), (0, 0) is the upper-left corner of the image
	// However, in u-v coordinates, (0, 0) is the lower-left corner of the image
	// This means that the image will appear upside-down when we use the image data as is
	// This function tells stbi to flip the image vertically so that it is not upside-down when we use it
	stbi_set_flip_vertically_on_load(true);

	// 'imageWidth' and imageHeight will contain the width and height of the loaded image respectively
	int imageWidth, imageHeight, numChannels;

	// Read the image data of our white.jpg image, and store it in an unsigned char array
	unsigned char* imageData = stbi_load("sun.jpg", &imageWidth, &imageHeight, &numChannels, 0);

	// Make sure that we actually loaded the image before uploading the data to the GPU
	if (imageData != nullptr)
	{
		// Our texture is 2D, so we bind our texture to the GL_TEXTURE_2D target
		glBindTexture(GL_TEXTURE_2D, tex0);

		// Set the filtering methods for magnification and minification
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		// Set the wrapping method for the s-axis (x-axis) and t-axis (y-axis)
		// Try experimenting with different wrapping methods introduced in the textures slide set
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Upload the image data to GPU memory
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);

		// If we set minification to use mipmaps, we can tell OpenGL to generate the mipmaps for us
		//glGenerateMipmap(GL_TEXTURE_2D);

		// Once we have copied the data over to the GPU, we can delete
		// the data on the CPU side, since we won't be using it anymore
		stbi_image_free(imageData);
		imageData = nullptr;
	}
	else
	{
		std::cerr << "Failed to load sun.jpg" << std::endl;
	}

	// Create a shader program
	GLuint program = CreateShaderProgram("main.vsh", "main.fsh");
	GLuint skyboxShader = CreateShaderProgram("skybox.vsh", "skybox.fsh");
	GLuint lightShader = CreateShaderProgram("light.vsh", "light.fsh");

	// Tell OpenGL the dimensions of the region where stuff will be drawn.
	// For now, tell OpenGL to use the whole screen
	glViewport(0, 0, windowWidth, windowHeight);

	// We enable depth testing so that we use the z-axis to determine
	// which objects goes in front of which object (when overlapping geometry is drawn)
	glEnable(GL_DEPTH_TEST);

	// Declaration of mouse and keyboard movement
	xMousePos = windowWidth / 2;
	yMousePos = windowHeight / 2;
	glfwSetCursorPos(window, xMousePos, yMousePos);

	// Declaration of camera units
	glm::vec3 cameraPosition = { 0.0f, 200.0f, 0.f };
	target = { 0.0f, 0.0f, 0.0f }; // Target is a specific point that the camera is looking at
	glm::vec3 up = { 0.0f, 1.0f, 0.0f }; // Global up vector (which will be used by the lookAt function to calculate the camera's right and up vectors)

	glm::vec3 eye = cameraPosition;

	// Declaration of time frames
	float deltaTime = 0.0f;	// Time between current frame and last frame
	float lastFrame = 0.0f; // Time of last frame
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glm::mat4 modelMatrix(1.0f);
	SetPlanetInfo();

	// Render loop
	while (!glfwWindowShouldClose(window))
	{
		// Clear the colors and depth values (since we enabled depth testing) in our off-screen framebuffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 viewMatrix(1.0f);
		viewMatrix = glm::translate(viewMatrix, -cameraPosition); // Note the negative translation
		glm::mat4 lookAtMatrix = glm::lookAt(eye, eye + target, up);

		// Construct our view frustrum (projection matrix) using the following parameters
		float fieldOfViewY = glm::radians(60.0f); // Field of view
		float aspectRatio = windowWidth * 1.0f / windowHeight; // Aspect ratio, which is the ratio between width and height
		float nearPlane = 0.1f; // Near plane, minimum distance from the camera where things will be rendered
		float farPlane = 500.0f; // Far plane, maximum distance from the camera where things will be rendered
		glm::mat4 projectionMatrix = glm::perspective(fieldOfViewY, aspectRatio, nearPlane, farPlane);

		// Skybox rendering
		glDepthMask(GL_FALSE);
		//glDepthFunc(GL_LEQUAL);
		glUseProgram(skyboxShader);

		glm::mat4 skyboxView = glm::mat4(glm::mat3(lookAtMatrix));

		GLint skyboxViewMatrixUniform = glGetUniformLocation(skyboxShader, "viewMatrix");
		glUniformMatrix4fv(skyboxViewMatrixUniform, 1, GL_FALSE, glm::value_ptr(skyboxView));

		GLint skyboxProjectionMatrixUniform = glGetUniformLocation(skyboxShader, "projectionMatrix");
		glUniformMatrix4fv(skyboxProjectionMatrixUniform, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		glBindVertexArray(vao1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
		glDrawArrays(GL_TRIANGLE_FAN, 6, 6);
		glDrawArrays(GL_TRIANGLE_FAN, 12, 6);
		glDrawArrays(GL_TRIANGLE_FAN, 18, 6);
		glDrawArrays(GL_TRIANGLE_FAN, 24, 6);
		glDrawArrays(GL_TRIANGLE_FAN, 30, 6);
		glDepthMask(GL_TRUE);
		glBindVertexArray(0);
		//glDepthFunc(GL_LESS);
		// 
		// Use the shader program that we created
		glUseProgram(program);

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		float moveSpeed = moveConstant * deltaTime;
		// Construct our view matrix (for the "camera")
		// Let's say we want to position our camera to be at (2, 1, 4) and looking down at the origin (0, 0, 0).
		// For the position, remember that having our camera at (2, 1, 4) is the same as moving the entire world in the opposite direction (-2, -1, -4)
		// As for the orientation of the camera, we can use the lookAt function, which glm kindly provides us

		viewMatrix = lookAtMatrix * modelMatrix;
		GLint viewMatrixUniform = glGetUniformLocation(program, "viewMatrix");
		glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewMatrix));

		glm::mat4 normalMatrix(1.0f);
		GLint normalMatrixUniform = glGetUniformLocation(program, "normalMatrix");
		glUniformMatrix4fv(normalMatrixUniform, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		
		// START: Lighting
		// Point light
		// Ambient
		glm::vec3 ambientColor = glm::vec3(1.0f, 1.0f, 1.0f);
		GLint ambientColorUniform = glGetUniformLocation(program, "ptLight.ambient");
		glUniform3fv(ambientColorUniform, 1, glm::value_ptr(ambientColor));

		// Diffuse
		glm::vec3 lightPos = glm::vec3(0.0f, 0.0f, 0.0f);
		GLint lightPosUniform = glGetUniformLocation(program, "ptLight.position");
		glUniform3fv(lightPosUniform, 1, glm::value_ptr(lightPos));

		//glm::vec3 diffuseColor = glm::vec3(0.5294f, 0.8078f, 0.9216f);
		glm::vec3 diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
		GLint diffuseColorUniform = glGetUniformLocation(program, "ptLight.diffuse");
		glUniform3fv(diffuseColorUniform, 1, glm::value_ptr(diffuseColor));

		// Specular
		//glm::vec3 specularColor = glm::vec3(0.5294f, 0.8078f, 0.9216f);
		glm::vec3 specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
		GLint specularColorUniform = glGetUniformLocation(program, "ptLight.specular");
		glUniform3fv(specularColorUniform, 1, glm::value_ptr(specularColor));

		GLint eyePositionUniform = glGetUniformLocation(program, "eye");
		glUniform3fv(eyePositionUniform, 1, glm::value_ptr(eye));

		glm::vec3 objectSpecular = glm::vec3(1.0f, 1.0f, 1.0f);
		GLint objectSpecularUniform = glGetUniformLocation(program, "ptLight.objectSpecular");
		glUniform3fv(objectSpecularUniform, 1, glm::value_ptr(objectSpecular));
		
		int objectShininess = 256;
		GLint objectShininessUniform = glGetUniformLocation(program, "ptLight.shininess");
		glUniform1i(objectShininessUniform, objectShininess);

		glm::vec3 pointLightAttenuation = glm::vec3(0.0f, 0.f, 1.0f);
		GLint plAttenuationUniform = glGetUniformLocation(program, "ptLight.attenuation");
		glUniform3fv(plAttenuationUniform, 1, glm::value_ptr(pointLightAttenuation));
		// END: Lighting

		GLint projectionMatrixUniform = glGetUniformLocation(program, "projectionMatrix");
		glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		// We retrieve our 'modelMatrix' uniform variable from the vertex shader,
		GLint modelMatrixUniform = glGetUniformLocation(program, "modelMatrix");

		glBindVertexArray(0);
		glBindVertexArray(vao2);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		
		float x1, z1;
		// Declaration of elliptical constants
		GLint texUniform = glGetUniformLocation(program, "tex");
		glUniform1i(texUniform, 0);

		for (auto currentPlanet : planets) {
			glm::mat4 sphereTransform2 = glm::mat4(1.0f);
			x1 = currentPlanet.majorAxis * glm::cos(glm::radians((float)glfwGetTime() * currentPlanet.speed * revolutionSpeed));
			z1 = currentPlanet.minorAxis * glm::sin(glm::radians((float)glfwGetTime() * currentPlanet.speed * revolutionSpeed));

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, currentPlanet.texture);

			sphereTransform2 = glm::translate(sphereTransform2, glm::vec3(currentPlanet.cx, currentPlanet.cy, currentPlanet.cz));
			sphereTransform2 = glm::translate(sphereTransform2, glm::vec3(x1, 0.f, z1));
			sphereTransform2 = glm::scale(sphereTransform2, glm::vec3(currentPlanet.radius));
			glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(sphereTransform2));
			glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, (void*)0);
		}

		glBindVertexArray(0);

		glUseProgram(lightShader);
		glBindVertexArray(sunVAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

		GLint lightTexUniform = glGetUniformLocation(lightShader, "tex");
		glUniform1i(texUniform, tex0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex0);
		glm::mat4 sphereTransforms(1.0f);

		GLint lightProjectionMatrixUniform = glGetUniformLocation(lightShader, "projectionMatrix");
		glUniformMatrix4fv(lightProjectionMatrixUniform, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
		GLint lightViewMatrixUniform = glGetUniformLocation(lightShader, "viewMatrix");
		glUniformMatrix4fv(lightViewMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		GLint lightModelMatrixUniform = glGetUniformLocation(lightShader, "modelMatrix");

		glUniformMatrix4fv(lightModelMatrixUniform, 1, GL_FALSE, glm::value_ptr(sphereTransforms));
		glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, (void*)0);

		glBindVertexArray(0);
		// Movement
		glfwGetCursorPos(window, &xMousePos, &yMousePos);
		ProcessMovement(window, eye, target, up, moveSpeed);
		ProcessRevolutionSpeed(window, revolutionSpeed);
		//std::cout << "Revolution speed:" << revolutionSpeed << std::endl;
		glfwSetCursorPosCallback(window, ProcessMouse);

		// "Unuse" the vertex array object
		glBindVertexArray(0);

		// Tell GLFW to swap the screen buffer with the offscreen buffer
		glfwSwapBuffers(window);

		// Tell GLFW to process window events (e.g., input events, window closed events, etc.)
		glfwPollEvents();
	}

	// --- Cleanup ---

	// Make sure to delete the shader program
	glDeleteProgram(program);
	glDeleteProgram(skyboxShader);

	// Delete the VBO that contains our vertices
	glDeleteBuffers(1, &vbo2);
	glDeleteBuffers(1, &vbo1);

	// Delete the vertex array object
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteVertexArrays(1, &vbo1);
	glDeleteVertexArrays(1, &vbo2);

	// Delete our textures
	glDeleteTextures(1, &tex0);

	// Remember to tell GLFW to clean itself up before exiting the application
	glfwTerminate();

	return 0;
}

/**
 * @brief Creates a shader program based on the provided file paths for the vertex and fragment shaders.
 * @param[in] vertexShaderFilePath Vertex shader file path
 * @param[in] fragmentShaderFilePath Fragment shader file path
 * @return OpenGL handle to the created shader program
 */
GLuint CreateShaderProgram(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath)
{
	GLuint vertexShader = CreateShaderFromFile(GL_VERTEX_SHADER, vertexShaderFilePath);
	GLuint fragmentShader = CreateShaderFromFile(GL_FRAGMENT_SHADER, fragmentShaderFilePath);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	glLinkProgram(program);

	glDetachShader(program, vertexShader);
	glDeleteShader(vertexShader);
	glDetachShader(program, fragmentShader);
	glDeleteShader(fragmentShader);

	// Check shader program link status
	GLint linkStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != GL_TRUE) {
		char infoLog[512];
		GLsizei infoLogLen = sizeof(infoLog);
		glGetProgramInfoLog(program, infoLogLen, &infoLogLen, infoLog);
		std::cerr << "program link error: " << infoLog << std::endl;
	}

	return program;
}

/**
 * @brief Creates a shader based on the provided shader type and the path to the file containing the shader source.
 * @param[in] shaderType Shader type
 * @param[in] shaderFilePath Path to the file containing the shader source
 * @return OpenGL handle to the created shader
 */
GLuint CreateShaderFromFile(const GLuint& shaderType, const std::string& shaderFilePath)
{
	std::ifstream shaderFile(shaderFilePath);
	if (shaderFile.fail())
	{
		std::cerr << "Unable to open shader file: " << shaderFilePath << std::endl;
		return 0;
	}

	std::string shaderSource;
	std::string temp;
	while (std::getline(shaderFile, temp))
	{
		shaderSource += temp + "\n";
	}
	shaderFile.close();

	return CreateShaderFromSource(shaderType, shaderSource);
}

/**
 * @brief Creates a shader based on the provided shader type and the string containing the shader source.
 * @param[in] shaderType Shader type
 * @param[in] shaderSource Shader source string
 * @return OpenGL handle to the created shader
 */
GLuint CreateShaderFromSource(const GLuint& shaderType, const std::string& shaderSource)
{
	GLuint shader = glCreateShader(shaderType);

	const char* shaderSourceCStr = shaderSource.c_str();
	GLint shaderSourceLen = static_cast<GLint>(shaderSource.length());
	glShaderSource(shader, 1, &shaderSourceCStr, &shaderSourceLen);
	glCompileShader(shader);

	// Check compilation status
	GLint compileStatus;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE)
	{
		char infoLog[512];
		GLsizei infoLogLen = sizeof(infoLog);
		glGetShaderInfoLog(shader, infoLogLen, &infoLogLen, infoLog);
		std::cerr << "shader compilation error: " << infoLog << std::endl;
	}

	return shader;
}

/**
 * @brief Function for handling the event when the size of the framebuffer changed.
 * @param[in] window Reference to the window
 * @param[in] width New width
 * @param[in] height New height
 */
void FramebufferSizeChangedCallback(GLFWwindow* window, int width, int height)
{
	// Whenever the size of the framebuffer changed (due to window resizing, etc.),
	// update the dimensions of the region to the new size
	glViewport(0, 0, width, height);
}