/*
Display of 3D model

1) Loading a 3D mesh model and display it on the screen

2) Four modes are supported: wireframe mode, vertex mode, face mode, face and edge mode.  Mode is switched by keyboard. Under the face mode, the color of each face is different.

3) Rotate and translate the model by keyboard

4) Change colors under wireframe mode by keyboard.

Written by Xiaotian Cheng (cxt_tsinghua@126.com)
*/

#include "shader.h"

#include <iostream>
#include <ios>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#define N_MODES 3
inline void printUsage()
{
	std::cout << "DisplayOBJ.exe filename.obj" << std::endl;
}

// return random float ranging in [0, 1]
inline float randfloat()
{
	return ((float)rand() / (RAND_MAX));
}


// Load an obj file
void load_obj(const char* filename, std::vector<glm::vec3> &vertices, std::vector<glm::vec3> &normals, std::vector<GLushort> &triangles);
// Convert triangle elements to edges
void triangle2edge(const std::vector<GLushort> &triangles, std::vector<GLushort> &edges);
// Generate random vertices color
void color_generator(int n_vertices, std::vector<glm::vec3> &colors);

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
// key cache
static bool keys[1024];
// caps lock or not
static bool caps = false;
// wireframe edge color
static float edge_r = 1.0f, edge_g = 0.5f, edge_b = 0.2f;

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printUsage(); exit(1);
	}
	const char* filename = argv[1];

	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // for mac

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "DisplayOBJ", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_LOCK_KEY_MODS, GLFW_TRUE);
	// Set the required callback functions
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	
	// Initialize GLAD to setup the OpenGL Function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize glad" << std::endl;
		return -1;
	};
	// Setup OpenGL options
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	// Build and compile our shader program
	Shader myShader("main.vert.glsl", "main.frag.glsl");

	// Load vertices and triangles from obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<GLushort> triangles;
	load_obj(filename, vertices, normals, triangles);
	
	// Convert triangle indices to edge indices for edge visualization
	std::vector<GLushort> edges;
	triangle2edge(triangles, edges);

	// Create colors
	std::vector<glm::vec3> colors;
	color_generator(vertices.size(), colors);

	glm::vec3 flat_color;
	flat_color.r = edge_r;
	flat_color.g = edge_g;
	flat_color.b = edge_b;
	std::vector<glm::vec3> flat_colors(vertices.size(), flat_color);
	

	// Bind vertices to buffer
	GLuint VAOs[N_MODES];
	GLuint vbo_vertices;
	GLuint vbo_colors, vbo_flat_colors;
	GLuint ibo_triangles, ibo_edges;

	glGenVertexArrays(N_MODES, VAOs);
	glGenBuffers(1, &vbo_vertices); 
	glGenBuffers(1, &vbo_colors);
	glGenBuffers(1, &vbo_flat_colors);
	glGenBuffers(1, &ibo_triangles);
	glGenBuffers(1, &ibo_edges);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_colors);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_flat_colors);
	glBufferData(GL_ARRAY_BUFFER, flat_colors.size() * sizeof(glm::vec3), &flat_colors[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_triangles);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(GLushort), &triangles[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_edges);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, edges.size() * sizeof(GLushort), &edges[0], GL_STATIC_DRAW);

	// 1. vertice mode
	glBindVertexArray(VAOs[0]); 
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_flat_colors);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_flat_colors);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glBindVertexArray(0); // Unbind VAO


	// 2. wireframe mode
	glBindVertexArray(VAOs[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_flat_colors);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_flat_colors);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_edges);
	glBindVertexArray(0); // Unbind VAO

	// 3. face mode
	glBindVertexArray(VAOs[2]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_colors);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_colors);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_triangles);
	glBindVertexArray(0); // Unbind VAO

	// 4. face + wireframe mode
	// combine 2 + 3


	// Create transformations
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(75.0f), glm::vec3(1.0f, 1.0f, 1.0f));
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
	projection = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

	
	int currentMode = 1;
	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		// Render
		// Clear the colorbuffer
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		

		// Activate shader
		myShader.Use();
		// translate
		if (keys[GLFW_KEY_L] == true && !caps)
		{
			model = glm::translate(model, glm::vec3(-0.01f, 0.0f, 0.0));
		}
		else if (keys[GLFW_KEY_R] == true && !caps)
		{
			model = glm::translate(model, glm::vec3(0.01f, 0.0f, 0.0));
		}
		else if (keys[GLFW_KEY_U] == true && !caps)
		{
			model = glm::translate(model, glm::vec3(0.0f, 0.01f, 0.0));
		}
		else if (keys[GLFW_KEY_D] == true && !caps)
		{
			model = glm::translate(model, glm::vec3(0.0f, -0.01f, 0.0));
		}
		else if (keys[GLFW_KEY_F] == true && !caps)
		{
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.01f));
		}
		else if (keys[GLFW_KEY_B] == true && !caps)
		{
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.01f));
		}
		// rotate
		else if (keys[GLFW_KEY_L] == true && caps)
		{
			model = glm::rotate(model, glm::radians(3.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		}
		else if (keys[GLFW_KEY_R] == true && caps)
		{
			model = glm::rotate(model, glm::radians(3.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
		}
		else if (keys[GLFW_KEY_U] == true && caps)
		{
			model = glm::rotate(model, glm::radians(3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		else if (keys[GLFW_KEY_D] == true && caps)
		{
			model = glm::rotate(model, glm::radians(3.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		}
		else if (keys[GLFW_KEY_F] == true && caps)
		{
			model = glm::rotate(model, glm::radians(3.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		}
		else if (keys[GLFW_KEY_B] == true && caps)
		{
			model = glm::rotate(model, glm::radians(3.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		}


		// Get their uniform location
		GLint modelLoc = glGetUniformLocation(myShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(myShader.Program, "view");
		GLint projLoc = glGetUniformLocation(myShader.Program, "projection");
		// Pass them to the shaders
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		
		// buffer size
		int size;  glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

		if (keys[GLFW_KEY_1] == true) currentMode = 1;
		else if (keys[GLFW_KEY_2] == true) currentMode = 2;
		else if (keys[GLFW_KEY_3] == true) currentMode = 3;
		else if (keys[GLFW_KEY_4] == true) currentMode = 4;
		if (keys[GLFW_KEY_C] && (currentMode == 2 || currentMode == 4))
		{
			flat_color.r = randfloat();
			flat_color.g = randfloat();
			flat_color.b = randfloat();
			flat_colors.clear();
			flat_colors = std::vector<glm::vec3>(vertices.size(), flat_color);
			glBindVertexArray(VAOs[1]);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_flat_colors);
			glBufferData(GL_ARRAY_BUFFER, flat_colors.size() * sizeof(glm::vec3), &flat_colors[0], GL_DYNAMIC_DRAW);
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_flat_colors);
			glBindVertexArray(0);
		}
		if (currentMode == 1)
		{
			glBindVertexArray(VAOs[0]);
			glDrawArrays(GL_POINTS, 0, vertices.size());
		}
		else if (currentMode == 2)
		{
			glBindVertexArray(VAOs[1]);
			glDrawElements(GL_LINES, size / sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
		}
		else if (currentMode == 3)
		{
			glBindVertexArray(VAOs[2]);
			glDrawElements(GL_TRIANGLES, size / sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
		}
		else if (currentMode == 4)
		{
			glBindVertexArray(VAOs[1]);
			glDrawElements(GL_LINES, size / sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
			glBindVertexArray(VAOs[2]);
			glDrawElements(GL_TRIANGLES, size / sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
		}
		else
		{
			std::cerr << "Invalid Mode!" << std::endl;
			exit(1);
		}

		glBindVertexArray(0);
		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	// Properly de-allocate all resources once they've outlived their purpose
	glDeleteVertexArrays(N_MODES, VAOs);
	glDeleteBuffers(1, &vbo_vertices);
	glDeleteBuffers(1, &ibo_triangles);
	glDeleteBuffers(1, &ibo_edges);
	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return 0;
}


void load_obj(const char* filename, std::vector<glm::vec3> &vertices, std::vector<glm::vec3> &normals, std::vector<GLushort> &triangles)
{
	std::ifstream in(filename, std::ios::in);
	if (!in)
	{
		std::cerr << "Cannot open " << filename << std::endl; exit(1);
	}

	std::string line;
	while (getline(in, line))
	{
		if (line.substr(0, 2) == "v ")
		{
			std::istringstream s(line.substr(2));
			glm::vec3 v; s >> v.x; s >> v.y; s >> v.z; 
			vertices.push_back(v);
		}
		else if (line.substr(0, 2) == "f ")
		{
			std::istringstream s(line.substr(2));
			GLushort a, b, c;
			s >> a; s >> b; s >> c;
			a--; b--; c--;
			triangles.push_back(a); triangles.push_back(b); triangles.push_back(c);
		}
		/* anything else is ignored */
	}

	normals.resize(vertices.size(), glm::vec3(0.0, 0.0, 0.0));
	for (int i = 0; i < triangles.size(); i += 3)
	{
		GLushort ia = triangles[i];
		GLushort ib = triangles[i + 1];
		GLushort ic = triangles[i + 2];
		glm::vec3 normal = glm::normalize(glm::cross(
			glm::vec3(vertices[ib]) - glm::vec3(vertices[ia]),
			glm::vec3(vertices[ic]) - glm::vec3(vertices[ia])));
		normals[ia] = normals[ib] = normals[ic] = normal;
	}
}

void triangle2edge(const std::vector<GLushort> &triangles, std::vector<GLushort> &edges)
{
	for (int i = 0; i < triangles.size(); i += 3)
	{
		GLushort ia = triangles[i];
		GLushort ib = triangles[i + 1];
		GLushort ic = triangles[i + 2];
		edges.push_back(ia); edges.push_back(ib);
		edges.push_back(ib); edges.push_back(ic);
		edges.push_back(ic); edges.push_back(ia);
	}
}

void color_generator(int n_vertices, std::vector<glm::vec3> &colors)
{
	for (int i = 0; i < n_vertices; i++)
	{
		glm::vec4 c;
		c.r = randfloat();
		c.g = randfloat();
		c.b = randfloat();
		colors.push_back(c);
	}
}
// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
		caps = (mode & GLFW_MOD_CAPS_LOCK);
	}
	
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}