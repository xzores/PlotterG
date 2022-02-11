//Written by Jakob Furbo Enevoldsen 2022
//see readme and license (MIT) for details

#define GLEW_STATIC

#include "serial/serial.h"
#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <assert.h>
#include <string>
#include <filesystem>

#include "LoadShaders.h"




// An array of 3 vectors which represents 3 vertices
static const GLfloat g_vertex_buffer_data[] = {
   -1.0f, -1.0f, 0.0f,
   1.0f, -1.0f, 0.0f,
   0.0f,  1.0f, 0.0f,
};



struct Settings {
	
	int horizontal_views = 1;
	int vertical_views = 1;
	int number_of_channels = 1;
	int bitsize = 32;
	int view_buffer_size = 10000;
	int serial_speed = 115200;
	bool save_settings = true;
};



class View {
	
private:
	std::vector<float> data_points = std::vector<float>();
	int start_index = 0;

public:
	
	void add_data_points(std::vector<float>);
	void set_buffer_size(int);
	void set_render_dimensions(float, float, float, float);
	
	
	void render();


};




void error_callback(int error, const char* msg) {	
	printf(msg);
	printf("\n");
}




int main(int argc, char* argv[]) {

	///////////////////////// SETTINGS /////////////////////////
	
	Settings settings;

	//TODO: make settings
	//if (std::filesystem::exists("settings.pltg")) {
	//	std::filesystem::
	//}

	serial::PortInfo target_port;

	///////////////////////// SERIAL ///////////////////////////

	std::vector<serial::PortInfo> ports = serial::list_ports();

	for (int i = 0; i < ports.size(); i++) {
		printf("found serial port: %s \n", ports[i].port.c_str());
	}

	///////////////////////// ARGUMENTS /////////////////////////

	printf("Arguments: %i \n", argc);

	if (argc == 2) {
		
		bool found = false;

		for (int i = 0; i < ports.size(); i++) {
			
			if (ports[i].port == argv[1]) {
				target_port = ports[i];
				found = true;
			}
		}
		
		if(found)
			printf("Found given port: %s \n", argv[1]);
		else
			printf("Cannot find: %s \n", argv[1]);

	}
	else if (argc > 2) {
		printf("Only a single argument is valid, the name of the COM port should be passed. Example \"COM1\". Alternativly don't pass any arguments. \n");
	}
	

	///////////////////////// WINDOW /////////////////////////


	auto window_size_callback = [](GLFWwindow* window, int width, int height) {
		glViewport(0,0,width,height);
	};


	glfwSetErrorCallback(error_callback);
	
	if (!glfwInit()) {

		printf("glfw init failed");
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 

	GLFWwindow* window = glfwCreateWindow(640, 480, "PlotterG", NULL, NULL);

	glfwSetWindowSizeCallback(window, window_size_callback);

	if (!window)
	{
		glfwTerminate();
		printf("window creation failed");
		return 1;
	}

	glfwMakeContextCurrent(window); // Initialize GLEW

	///////////////////////// GLEW INIT /////////////////////////

	glewExperimental = true; // Needed in core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}


	//////////////////

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// This will identify our vertex buffer
	GLuint vertexbuffer;
	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint programID = LoadShaders("Shaders/shader.vert", "Shaders/shader.frag");

	glUseProgram(programID);
	
	//////////////////


	while (!glfwWindowShouldClose(window))
	{

		///////////////////////// RENDERING /////////////////////////

		glClear(GL_COLOR_BUFFER_BIT);

		// 1st attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
		glDisableVertexAttribArray(0);

		/////////////////////////////////////////////////////////////


		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	
	return 0;
}

