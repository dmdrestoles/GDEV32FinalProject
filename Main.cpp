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
};

float moveConstant = 2.0f;
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
		moveConstant = 4.0f;
	}

	if (shiftKey == GLFW_RELEASE)
	{
		moveConstant = 2.0f;
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

	std::vector<Vertex> sphereVertices;
	std::vector<int> sphereIndices;
	float sphereColor[3] = { 255, 255, 255 };

	std::vector<Vertex>sphereLightVertices;
	std::vector<int> sphereLightIndices;
	float sphereLightColor[3] = { 255, 255, 255 };

	GenerateSphereVertices(sphereVertices, sphereIndices, 1.0f, 36, 18, sphereColor);

	GenerateSphereVertices(sphereLightVertices, sphereLightIndices, 1.0f, 36, 18, sphereLightColor);

	// Create a vertex buffer object (VBO), and upload our vertices data to the VBO
	GLuint vbo2;

	glGenBuffers(1, &vbo2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo2);
	glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(Vertex), sphereVertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint lightVBO;
	glGenBuffers(1, &lightVBO);
	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, sphereLightVertices.size() * sizeof(Vertex), sphereLightVertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint ibo, lightIbo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(int), sphereIndices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &lightIbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightIbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereLightIndices.size() * sizeof(int), sphereLightIndices.data(), GL_STATIC_DRAW);

	GLuint lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
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

	// Create a vertex array object that contains data on how to map vertex attributes
	// (e.g., position, color) to vertex shader properties.
	GLuint vao2;
	
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

	// Read the image data of our eye.jpg image, and store it in an unsigned char array
	unsigned char* imageData = stbi_load("eye.jpg", &imageWidth, &imageHeight, &numChannels, 0);

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
		std::cerr << "Failed to load eye.jpg" << std::endl;
	}

	// Create our other texture (wings.jpg)
	GLuint tex1;
	glGenTextures(1, &tex1);

	// Read the image data of our bioshock.jpg image
	imageData = stbi_load("wings.jpg", &imageWidth, &imageHeight, &numChannels, 0);

	// Make sure that we actually loaded the image before uploading the data to the GPU
	if (imageData != nullptr)
	{
		// Our texture is 2D, so we bind our texture to the GL_TEXTURE_2D target
		glBindTexture(GL_TEXTURE_2D, tex1);

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
		std::cerr << "Failed to load wings.jpg" << std::endl;
	}

	// Create a shader program
	GLuint program = CreateShaderProgram("main.vsh", "main.fsh");

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
	glm::vec3 cameraPosition = { 0.0f, 0.0f, 3.0f };
	target = { 0.0f, 0.0f, -1.0f }; // Target is a specific point that the camera is looking at
	glm::vec3 up = { 0.0f, 1.0f, 0.0f }; // Global up vector (which will be used by the lookAt function to calculate the camera's right and up vectors)

	glm::vec3 eye = cameraPosition;

	// Declaration of time frames
	float deltaTime = 0.0f;	// Time between current frame and last frame
	float lastFrame = 0.0f; // Time of last frame
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glm::mat4 modelMatrix(1.0f);

	// Render loop
	while (!glfwWindowShouldClose(window))
	{
		// Clear the colors and depth values (since we enabled depth testing) in our off-screen framebuffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

		glm::mat4 viewMatrix(1.0f);
		viewMatrix = glm::translate(viewMatrix, -cameraPosition); // Note the negative translation
		glm::mat4 lookAtMatrix = glm::lookAt(eye, eye + target, up);

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
		glm::vec3 lightPos = glm::vec3(0.0f, 5.0f, 5.0f);
		GLint lightPosUniform = glGetUniformLocation(program, "ptLight.position");
		glUniform3fv(lightPosUniform, 1, glm::value_ptr(lightPos));

		glm::vec3 diffuseColor = glm::vec3(0.5294f, 0.8078f, 0.9216f);
		//glm::vec3 diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
		GLint diffuseColorUniform = glGetUniformLocation(program, "ptLight.diffuse");
		glUniform3fv(diffuseColorUniform, 1, glm::value_ptr(diffuseColor));

		// Specular
		glm::vec3 specularColor = glm::vec3(0.5294f, 0.8078f, 0.9216f);
		// glm::vec3 specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
		GLint specularColorUniform = glGetUniformLocation(program, "ptLight.specular");
		glUniform3fv(specularColorUniform, 1, glm::value_ptr(specularColor));

		GLint eyePositionUniform = glGetUniformLocation(program, "eye");
		glUniform3fv(eyePositionUniform, 1, glm::value_ptr(eye));

		glm::vec3 objectSpecular = glm::vec3(0.5294f, 0.8078f, 0.9216f);
		GLint objectSpecularUniform = glGetUniformLocation(program, "ptLight.objectSpecular");
		glUniform3fv(objectSpecularUniform, 1, glm::value_ptr(objectSpecular));
		
		int objectShininess = 256;
		GLint objectShininessUniform = glGetUniformLocation(program, "ptLight.shininess");
		glUniform1i(objectShininessUniform, objectShininess);

		glm::vec3 pointLightAttenuation = glm::vec3(0.017, 0.07f, 1.0f);
		GLint plAttenuationUniform = glGetUniformLocation(program, "ptLight.attenuation");
		glUniform3fv(plAttenuationUniform, 1, glm::value_ptr(pointLightAttenuation));
		
		// Directional light
		glm::vec3 directionalLightVector = glm::vec3(0.0f, 0.0f, -20.0f);
		GLint directionalLightVectorUniform = glGetUniformLocation(program, "dirLight.direction");
		glUniform3fv(directionalLightVectorUniform, 1, glm::value_ptr(directionalLightVector));

		glm::vec3 directionalAmbientColor = glm::vec3(0.82f, 0.65f, 0.13f);
		//glm::vec3 directionalAmbientColor = glm::vec3(1.0f, 1.0f, 1.0f);
		GLint directionalAmbientColorUniform = glGetUniformLocation(program, "dirLight.ambient");
		glUniform3fv(directionalAmbientColorUniform, 1, glm::value_ptr(directionalAmbientColor));

		//glm::vec3 directionalDiffuseColor = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 directionalDiffuseColor = glm::vec3(0.82f, 0.65f, 0.13f);
		//glm::vec3 directionalDiffuseColor = glm::vec3(1.0f, 0.0f, 0.0f);
		GLint directionalDiffuseColorUniform = glGetUniformLocation(program, "dirLight.diffuse");
		glUniform3fv(directionalDiffuseColorUniform, 1, glm::value_ptr(directionalDiffuseColor));

		glm::vec3 directionalSpecularColor = glm::vec3(0.82f, 0.65f, 0.13f);
		//glm::vec3 directionalSpecularColor = glm::vec3(0.0f, 0.0f, 0.0f);
		GLint directionalSpecularColorUniform = glGetUniformLocation(program, "dirLight.specular");
		glUniform3fv(directionalSpecularColorUniform, 1, glm::value_ptr(directionalSpecularColor));

		glm::vec3 directionalObjectSpecularColor = glm::vec3(1.0f, 1.0f, 1.0f);
		GLint dirLightObjectSpecularUniform = glGetUniformLocation(program, "dirLight.objectSpecular");
		glUniform3fv(dirLightObjectSpecularUniform, 1, glm::value_ptr(directionalObjectSpecularColor));

		GLint dirLightObjectShininessUniform = glGetUniformLocation(program, "dirLight.shininess");
		glUniform1i(dirLightObjectShininessUniform, objectShininess);
		
		glm::vec3 spotlightColor = glm::vec3(1.0f, 1.0f, 1.0f);
		GLint spotlightColorUniform = glGetUniformLocation(program, "spotlight.ambient");
		glUniform3fv(spotlightColorUniform, 1, glm::value_ptr(spotlightColor));
		spotlightColorUniform = glGetUniformLocation(program, "spotlight.diffuse");
		glUniform3fv(spotlightColorUniform, 1, glm::value_ptr(spotlightColor));
		spotlightColorUniform = glGetUniformLocation(program, "spotlight.specular");
		glUniform3fv(spotlightColorUniform, 1, glm::value_ptr(spotlightColor));
		spotlightColorUniform = glGetUniformLocation(program, "spotlight.objectSpecular");
		glUniform3fv(spotlightColorUniform, 1, glm::value_ptr(spotlightColor));

		GLint spotlightUniform = glGetUniformLocation(program, "spotlight.position");
		glUniform3fv(spotlightUniform, 1, glm::value_ptr(eye));

		spotlightUniform = glGetUniformLocation(program, "spotlight.direction");
		glUniform3fv(spotlightUniform, 1, glm::value_ptr(target));

		float spotlightCutoff = glm::cos(glm::radians(15.0f));
		spotlightUniform = glGetUniformLocation(program, "spotlight.cutoff");
		glUniform1f(spotlightUniform, spotlightCutoff);
		
		float spotlightOutercutoff = glm::cos(glm::radians(17.5f));
		spotlightUniform = glGetUniformLocation(program, "spotlight.outerCutoff");
		glUniform1f(spotlightUniform, spotlightOutercutoff);

		GLint spotlightShininessUniform = glGetUniformLocation(program, "spotlight.shininess");
		glUniform1i(spotlightShininessUniform, objectShininess);
		
		plAttenuationUniform = glGetUniformLocation(program, "spotlight.attenuation");
		glUniform3fv(plAttenuationUniform, 1, glm::value_ptr(pointLightAttenuation));

		// END: Lighting
		
		// Construct our view frustrum (projection matrix) using the following parameters
		float fieldOfViewY = glm::radians(60.0f); // Field of view
		float aspectRatio = windowWidth * 1.0f / windowHeight; // Aspect ratio, which is the ratio between width and height
		float nearPlane = 0.1f; // Near plane, minimum distance from the camera where things will be rendered
		float farPlane = 100.0f; // Far plane, maximum distance from the camera where things will be rendered
		glm::mat4 projectionMatrix = glm::perspective(fieldOfViewY, aspectRatio, nearPlane, farPlane);

		GLint projectionMatrixUniform = glGetUniformLocation(program, "projectionMatrix");
		glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		// We retrieve our 'modelMatrix' uniform variable from the vertex shader,
		GLint modelMatrixUniform = glGetUniformLocation(program, "modelMatrix");

		glBindVertexArray(0);
		glBindVertexArray(vao2);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex1);

		glm::mat4 sphereTransforms(1.0f);
		glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(sphereTransforms));
		glDrawElements(GL_TRIANGLES, sphereLightIndices.size(), GL_UNSIGNED_INT, (void*)0);
		
		float x1, z1;
		// Declaration of elliptical constants
		Planet currentPlanet;
		const float distScale = 10.f;
		glm::mat4 sphereTransform2 = glm::mat4(1.0f);

		Planet mercury;
		mercury.radius = 0.1f;
		mercury.majorAxis = 1.38f * distScale;
		mercury.eccentricity = 0.205f;
		mercury.ComputeMinorAxis();
		mercury.speed = 375.0f;
		mercury.cx = 0.f;
		mercury.cy = 0.f;
		mercury.cz = 0.f;
		mercury.angle = 0.0f;

		currentPlanet = mercury;

		x1 = currentPlanet.majorAxis * glm::cos( glm::radians((float)glfwGetTime() * currentPlanet.speed) );
		z1 = currentPlanet.minorAxis * glm::sin( glm::radians((float)glfwGetTime() * currentPlanet.speed) );

		sphereTransform2 = glm::translate(sphereTransform2, glm::vec3(currentPlanet.cx, currentPlanet.cy, currentPlanet.cz));
		sphereTransform2 = glm::translate(sphereTransform2, glm::vec3(x1, 0.f, z1));
		sphereTransform2 = glm::scale(sphereTransform2, glm::vec3(currentPlanet.radius));
		glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(sphereTransform2));
		glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, (void*)0);

		Planet venus;
		venus.radius = 1.02f;
		venus.majorAxis = 1.72f * distScale;
		venus.eccentricity = 0.007;
		venus.ComputeMinorAxis();
		venus.speed = 145.161f;
		venus.cx = 0.f;
		venus.cy = 0.f;
		venus.cz = 0.f;
		venus.angle = 0.f;

		currentPlanet = venus;
		x1 = currentPlanet.majorAxis * glm::cos(glm::radians((float)glfwGetTime() * currentPlanet.speed));
		z1 = currentPlanet.minorAxis * glm::sin(glm::radians((float)glfwGetTime() * currentPlanet.speed));

		sphereTransform2 = glm::mat4(1.0f);
		sphereTransform2 = glm::translate(sphereTransform2, glm::vec3(currentPlanet.cx, currentPlanet.cy, currentPlanet.cz));
		sphereTransform2 = glm::translate(sphereTransform2, glm::vec3(x1, 0.f, z1));
		sphereTransform2 = glm::scale(sphereTransform2, glm::vec3(currentPlanet.radius));
		glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(sphereTransform2));
		glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, (void*)0);

		Planet earth;
		earth.radius = 1.0f;
		earth.majorAxis = 2.0f * distScale;
		earth.eccentricity = 0.017;
		earth.ComputeMinorAxis();
		earth.speed = 90.f;
		earth.cx = 0.f;
		earth.cy = 0.f;
		earth.cz = 0.f;
		earth.angle = 0.f;

		currentPlanet = earth;
		x1 = currentPlanet.majorAxis * glm::cos(glm::radians((float)glfwGetTime() * currentPlanet.speed));
		z1 = currentPlanet.minorAxis * glm::sin(glm::radians((float)glfwGetTime() * currentPlanet.speed));

		sphereTransform2 = glm::mat4(1.0f);
		sphereTransform2 = glm::translate(sphereTransform2, glm::vec3(currentPlanet.cx, currentPlanet.cy, currentPlanet.cz));
		sphereTransform2 = glm::translate(sphereTransform2, glm::vec3(x1, 0.f, z1));
		sphereTransform2 = glm::scale(sphereTransform2, glm::vec3(currentPlanet.radius));
		glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(sphereTransform2));
		glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, (void*)0);

		Planet mars;
		mars.radius = 0.53f;
		mars.majorAxis = 2.52f * distScale;
		mars.eccentricity = 0.093;
		mars.ComputeMinorAxis();
		mars.speed = 47.872;
		mars.cx = 0.f;
		mars.cy = 0.f;
		mars.cz = 0.f;
		mars.angle = 0.f;

		currentPlanet = mars;
		x1 = currentPlanet.majorAxis * glm::cos(glm::radians((float)glfwGetTime() * currentPlanet.speed));
		z1 = currentPlanet.minorAxis * glm::sin(glm::radians((float)glfwGetTime() * currentPlanet.speed));

		sphereTransform2 = glm::mat4(1.0f);
		sphereTransform2 = glm::translate(sphereTransform2, glm::vec3(currentPlanet.cx, currentPlanet.cy, currentPlanet.cz));
		sphereTransform2 = glm::translate(sphereTransform2, glm::vec3(x1, 0.f, z1));
		sphereTransform2 = glm::scale(sphereTransform2, glm::vec3(currentPlanet.radius));
		glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(sphereTransform2));
		glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, (void*)0);

		// Movement
		glfwGetCursorPos(window, &xMousePos, &yMousePos);
		ProcessMovement(window, eye, target, up, moveSpeed);
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

	// Delete the VBO that contains our vertices
	glDeleteBuffers(1, &vbo2);

	// Delete the vertex array object
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Delete our textures
	glDeleteTextures(1, &tex0);
	glDeleteTextures(1, &tex1);

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