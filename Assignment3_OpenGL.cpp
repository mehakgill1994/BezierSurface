// Assignment3_OpenGL.cpp : Defines the entry point for the console application.
//


#include <iostream>
#include <vector>
#include <memory>
#include <Windows.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_AVX2
#define GLM_FORCE_ALIGNED
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_helper.h"

//window client area size

const int ClientAreaWidth = 800;
const int ClientAreaHeight = 800;

//each vertex is 3 floats each for the position

struct Vertex
{
	float position[3];
};

//file names

const char *pointsFilePath = "gumbo.dat";


//global variables

GLFWwindow *window;
GLuint gpuProgram;
GLuint vertexBuffer, vertexArray;

//locations of uniform variables in the GPU program
GLuint locMatWorld, locMatView, locMatProjection, locLightDirection,
locShowBorders, locCustomOuterTessFactor, locCustomInnerTessFactor;

//globals for time measurement
LARGE_INTEGER clockFreq;
LARGE_INTEGER prevTime;

int showBorders = false;

//rotation angle for the cube
float angle = 0.0f;
//user-specified tessellation factors
float customOuterTessFactor = 24.0f;
float customInnerTessFactor = 24.0f;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

bool InitWindow();

bool LoadShaders();
bool LoadModel();
void InitUniforms();
void InitInputAssembler();
void InitRasterizer();
void InitPerSampleProcessing();

void MainLoop();
void Render();
void UpdateRotation();
void Cleanup();

int main()
{
	if (!glfwInit())
	{
		std::cerr << "Failed to load GLFW!" << std::endl;
		return 1;
	}

	if (!InitWindow())
	{
		std::cerr << "Failed to create window!" << std::endl;
		Cleanup();
		return 1;
	}

	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Failed to load GLEW!" << std::endl;
		Cleanup();
		return 1;
	}

	//init time measurement
	::QueryPerformanceFrequency(&clockFreq);
	::QueryPerformanceCounter(&prevTime);

	if (!LoadShaders())
	{
		std::cerr << "Failed to load shaders!" << std::endl;
		Cleanup();
		return 1;
	}

	LoadModel();
	InitUniforms();
	InitInputAssembler();
	InitRasterizer();
	InitPerSampleProcessing();

	MainLoop();

	Cleanup();

	return 0;
}

bool InitWindow()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(ClientAreaWidth, ClientAreaHeight, "Assignment 3 - Bézier Surfaces", nullptr, nullptr);

	if (!window)
		return false;

	//set the window's OpenGL context as the current OpenGL context

	glfwMakeContextCurrent(window);

	//set event handlers for the window

	glfwSetKeyCallback(window, key_callback);

	//the parameter 1 means VSync is enabled
	//change to 0 to disable VSync

	glfwSwapInterval(1);

	return true;
}

bool LoadShaders()
{
	//load shaders

	GLenum vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLenum tessControlShader = glCreateShader(GL_TESS_CONTROL_SHADER);
	GLenum tessEvaluationShader = glCreateShader(GL_TESS_EVALUATION_SHADER);
	GLenum geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
	GLenum fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	bool success = false;

	if (try_compile_shader_from_file(vertexShader, "VertexShader.glsl") &&
		try_compile_shader_from_file(tessControlShader, "TessControlShader.glsl") &&
		try_compile_shader_from_file(tessEvaluationShader, "TessEvaluationShader.glsl") &&
		try_compile_shader_from_file(geometryShader, "GeometryShader.glsl") &&
		try_compile_shader_from_file(fragmentShader, "FragmentShader.glsl"))
	{
		gpuProgram = glCreateProgram();

		glAttachShader(gpuProgram, vertexShader);
		glAttachShader(gpuProgram, tessControlShader);
		glAttachShader(gpuProgram, tessEvaluationShader);
		glAttachShader(gpuProgram, geometryShader);
		glAttachShader(gpuProgram, fragmentShader);

		success = try_link_program(gpuProgram);
	}

	glDeleteShader(vertexShader);
	glDeleteShader(tessControlShader);
	glDeleteShader(tessEvaluationShader);
	glDeleteShader(geometryShader);
	glDeleteShader(fragmentShader);

	return success;
}

bool LoadModel()
{
	//TODO
	unsigned int numPatches;
	std::unique_ptr<int[]> rawPointData2(new int[1]);
		

		std::ifstream fin(pointsFilePath, std::ios::binary);

		if (!fin.is_open())
		{
			std::wcerr << "Unable to open: " << pointsFilePath << std::endl;
			return false;
		}

		fin.read(reinterpret_cast<char*>(rawPointData2.get()), sizeof(int));
		numPatches = rawPointData2[0];

		std::unique_ptr<float[]> rawPointData(new float[numPatches * 16 * 3]);

		fin.read(reinterpret_cast<char*>(rawPointData.get()), numPatches * 16 * 3 * sizeof(float));

		//for (int i = 0; i < numPatches * 16 * 3; i++)
		//{
			//std::cout << "the value of "<< i << "th element is " <<rawPointData[i] << std::endl;
			
		//}
	
	//create vertex buffers and store data into them

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	glBufferStorage(GL_ARRAY_BUFFER, numPatches * 16 * 3 * sizeof(float), rawPointData.get(), 0);

	return true;
}

void InitUniforms()
{
	locMatWorld = glGetUniformLocation(gpuProgram, "matWorld");
	locMatView = glGetUniformLocation(gpuProgram, "matView");
	locMatProjection = glGetUniformLocation(gpuProgram, "matProjection");
	locShowBorders = glGetUniformLocation(gpuProgram, "showBorders");
	locCustomInnerTessFactor = glGetUniformLocation(gpuProgram, "customInnerTessFactor");
	locCustomOuterTessFactor = glGetUniformLocation(gpuProgram, "customOuterTessFactor");
	locLightDirection = glGetUniformLocation(gpuProgram, "lightDirection");

	glProgramUniformMatrix4fv(gpuProgram, locMatWorld, 1, GL_FALSE, glm::value_ptr(glm::mat4()));
	glProgramUniform1i(gpuProgram, locShowBorders, showBorders);
	glProgramUniform1f(gpuProgram, locCustomOuterTessFactor, customOuterTessFactor);
	glProgramUniform1f(gpuProgram, locCustomInnerTessFactor, customInnerTessFactor);

	glm::vec3 lightDir = glm::normalize(glm::vec3(0.0f, 1.0f, 2.0f));
	glProgramUniform3fv(gpuProgram, locLightDirection, 1, glm::value_ptr(lightDir));

	glm::vec3 eyePosition(50.0f, 30.0f, 30.0f);
	glm::vec3 lookAt(10.0f, 5.0f, -8.0f);
	glm::vec3 upDirection(0.0f, 1.0f, 0.0f);

	glm::mat4 matView = glm::lookAtRH(eyePosition, lookAt, upDirection);
	glProgramUniformMatrix4fv(gpuProgram, locMatView, 1, GL_FALSE, glm::value_ptr(matView));

	const float fieldOfView = glm::quarter_pi<float>();
	const float nearPlane = 0.1f;
	const float farPlane = 100.0f;

	glm::mat4 matProjection = glm::perspectiveFovRH(fieldOfView, (float)ClientAreaWidth, (float)ClientAreaHeight, nearPlane, farPlane);
	glProgramUniformMatrix4fv(gpuProgram, locMatProjection, 1, GL_FALSE, glm::value_ptr(matProjection));
}

void InitInputAssembler()
{
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	//each vertex is 3 floats each for the position

	//					index		num_components	type		normalize?	stride			offset
	glVertexAttribPointer(0,		3,				GL_FLOAT,	GL_FALSE,	3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

void InitRasterizer()
{
	glViewport(0, 0, ClientAreaWidth, ClientAreaHeight);
	glEnable(GL_CULL_FACE);
}

void InitPerSampleProcessing()
{
	glClearColor(0.6f, 0.7f, 0.9f, 1.0f);
	glEnable(GL_DEPTH_TEST);
}

void Cleanup()
{
	glDeleteProgram(gpuProgram);
	glDeleteVertexArrays(1, &vertexArray);
	glDeleteBuffers(1, &vertexBuffer);

	glfwTerminate();
}

void Render()
{
	//clear the depth and color buffers

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//bind whatever needs to be bound

	glBindVertexArray(vertexArray);
	glUseProgram(gpuProgram);

	//TODO
	glPatchParameteri(GL_PATCH_VERTICES, 16);
	glDrawArrays(GL_PATCHES, 0, 128 * 16 * 3);

	//swap the back and front buffers

	glfwSwapBuffers(window);

	UpdateRotation();
}

void UpdateRotation()
{
	static const float angularSpeed = 1.5f;
	static const glm::vec3 rotationAxis(-1.0f, 1.5f, 1.5f);
	static const glm::vec3 centroid(0.0f, 5.0f, 8.0f);

	LARGE_INTEGER currentTime;
	::QueryPerformanceCounter(&currentTime);

	float elapsedTime = (float)(double(currentTime.QuadPart - prevTime.QuadPart) / clockFreq.QuadPart);
	angle += angularSpeed * elapsedTime;

	while (angle > glm::two_pi<float>())
		angle -= glm::two_pi<float>();

	glm::mat4 matWorld;
	matWorld = glm::rotate(matWorld, angle, rotationAxis);
	matWorld = glm::translate(matWorld, -centroid);

	glProgramUniformMatrix4fv(gpuProgram, locMatWorld, 1, GL_FALSE, glm::value_ptr(matWorld));

	prevTime = currentTime;
}

void MainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		Render();
	}
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_ESCAPE)
		{
			glfwSetWindowShouldClose(window, true);
		}
		else if (key == GLFW_KEY_B)
		{
			showBorders = !showBorders;
			glProgramUniform1i(gpuProgram, locShowBorders, showBorders);
		}
		else if (key == GLFW_KEY_EQUAL)
		{
			if (mods & GLFW_MOD_SHIFT)
			{
				if (customOuterTessFactor < 64.0f)
					customOuterTessFactor += 1.0f;
				std::cout << "customOuterTessFactor = " << customOuterTessFactor << std::endl;
				glProgramUniform1f(gpuProgram, locCustomOuterTessFactor, customOuterTessFactor);
			}
			else
			{
				if (customInnerTessFactor < 64.0f)
					customInnerTessFactor += 1.0f;
				std::cout << "customInnerTessFactor = " << customInnerTessFactor << std::endl;
				glProgramUniform1f(gpuProgram, locCustomInnerTessFactor, customInnerTessFactor);
			}
		}
		else if (key == GLFW_KEY_MINUS)
		{
			if (mods & GLFW_MOD_SHIFT)
			{
				if (customOuterTessFactor > 1.0f)
					customOuterTessFactor -= 1.0f;
				std::cout << "customOuterTessFactor = " << customOuterTessFactor << std::endl;
				glProgramUniform1f(gpuProgram, locCustomOuterTessFactor, customOuterTessFactor);
			}
			else
			{
				if (customInnerTessFactor > 1.0f)
					customInnerTessFactor -= 1.0f;
				std::cout << "customInnerTessFactor = " << customInnerTessFactor << std::endl;
				glProgramUniform1f(gpuProgram, locCustomInnerTessFactor, customInnerTessFactor);
			}
		}
	}
}