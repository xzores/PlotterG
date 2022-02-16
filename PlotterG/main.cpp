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
   -1.0f, -1.0f,
   1.0f, -1.0f,
   1.0f,  1.0f,

   1.0f,  1.0f,
   -1.0f,  1.0f,
   -1.0f, -1.0f,
};


struct Settings {
	
	int horizontal_views = 8;
	int vertical_views = 5;
	int number_of_channels = 1;
	int bitsize = 32;
	int view_buffer_size = 10000;
	int serial_speed = 115200;
	float min_value = -std::pow(2,15);
	float max_value = std::pow(2, 15);
	bool save_settings = true;
	float view_outer_margin = 0.01;
	float view_inner_margin = 0.005;
	float outer_color_r = 0.09, outer_color_g = 0.09, outer_color_b = 0.09;
	float inner_color_r = 0.35, inner_color_g = 0.35, inner_color_b = 0.35;
	float background_color_r = 0.15, background_color_g = 0.15, background_color_b = 0.15;
	float signal_color_r = 0.15, signal_color_g = 0.15, signal_color_b = 0.7;
};


struct Mesh {

	GLuint VAO;
	GLuint vertex_position_buffer;
	GLuint vertex_color_buffer;
	std::vector<GLfloat> positions = std::vector<GLfloat>();
	std::vector<GLfloat> colors = std::vector<GLfloat>();

	GLuint pos_ID;
	GLuint size_ID;

	float pos_x, pos_y, size_x, size_y;

	void init(const GLuint& shader_ID) {

		pos_ID = glGetUniformLocation(shader_ID, "position");
		size_ID = glGetUniformLocation(shader_ID, "scale");

		///////// GENERATE VAO AND VBO's /////////
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &vertex_position_buffer);
		glGenBuffers(1, &vertex_color_buffer);
		
		///////// BINDING THE VBO TO THE VAO /////////
		glBindVertexArray(VAO);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			2,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_color_buffer);
		glVertexAttribPointer(
			1,                  // attribute 1. No particular reason for 1, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
		glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind VBO
		glBindVertexArray(0); //unbind VAO

	}


	void upload_position_data() {
		///////// FILLING THE VBO WITH DATA /////////
		glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * positions.size(), &(positions[0]), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind VBO
	}

	void upload_color_data() {
		///////// FILLING THE VBO WITH DATA /////////
		glBindBuffer(GL_ARRAY_BUFFER, vertex_color_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * colors.size(), &(colors[0]), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind VBO
	}

	void upload_data() {
		upload_position_data();
		upload_color_data();
	}

	void render(const GLenum& render_mode, const int& first, const int& last) {

		glUniform2f(pos_ID, pos_x, pos_y);
		glUniform2f(size_ID, size_x, size_y);

		glBindVertexArray(VAO);
		glDrawArrays(render_mode, first, last - first);
		glBindVertexArray(0);
	}

};

struct View {


private:

	int start_index = 0;
	int buffer_size;

public:

	Mesh background_mesh;
	Mesh data_mesh;

	void init(const int& buffer_size, const GLuint& shader_ID) {

		this->buffer_size = buffer_size;

		background_mesh.init(shader_ID);
		data_mesh.init(shader_ID);
	}

	void set_data() {
		background_mesh.upload_data();
		data_mesh.upload_data();
	}

	void render() {

		background_mesh.render(GL_TRIANGLES, 0, background_mesh.positions.size() / 2);
		glClear(GL_DEPTH_BUFFER_BIT);


		data_mesh.render(GL_LINE_STRIP, 0, data_mesh.positions.size() / 2);

	}


	void add_data_points(const std::vector<float>& data_points, const Settings& settings) {
		
		for (int i = 0; i < std::min(data_points.size(), (data_mesh.positions.size() - start_index)/2); i++) {
			
			float v = ((float)data_points[i] / (settings.max_value - settings.min_value)) * 2;
			data_mesh.positions[data_mesh.positions.size() - (start_index * 2 + 1) ] = v;
			start_index++;

			//printf("%i\n", start_index);
			if (start_index * 2 >= data_mesh.positions.size() - 1) {
				start_index = 0;
			}

		}

		data_mesh.upload_data();

	}


	void set_render_dimensions(float pos_x, float pos_y, float size_x, float size_y) {
		
		background_mesh.pos_x = pos_x;
		background_mesh.pos_y = pos_y;
		background_mesh.size_x = size_x;
		background_mesh.size_y = size_y;

		data_mesh.pos_x = pos_x;
		data_mesh.pos_y = pos_y;
		data_mesh.size_x = size_x;
		data_mesh.size_y = size_y;
	}
	
};



void error_callback(int error, const char* msg) {	
	printf(msg);
	printf("\n");
}



int main(int argc, char* argv[]) {

	///////////////////////// SETTINGS /////////////////////////
	
	Settings settings;

	std::vector<View> views = std::vector<View>();
	
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
	
	glewExperimental = true; // Needed for core profile
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

	glClearColor(settings.background_color_r, settings.background_color_g, settings.background_color_b, 1);

	///////////////////////// SHADER /////////////////////////

	GLuint programID = LoadShaders("Shaders/shader.vert", "Shaders/shader.frag");

	glUseProgram(programID);
	
	///////////////////////// SETUP VIEWS /////////////////////////

	//we might want to re init the views later
	auto setup_views = [&settings, &views, programID]()
	{
		views.resize(0); //remove all previos views

		float x_size = 1.0f / settings.horizontal_views;
		float y_size = 1.0f / settings.vertical_views;

		float outer_margin_resize = settings.view_outer_margin;
		float margin_resize = std::min(x_size * outer_margin_resize, y_size * outer_margin_resize);

		float outer_r = settings.outer_color_r;
		float outer_g = settings.outer_color_g;
		float outer_b = settings.outer_color_b;

		float inner_r = settings.inner_color_r;
		float inner_g = settings.inner_color_g;
		float inner_b = settings.inner_color_b;

		float r = settings.signal_color_r;
		float g = settings.signal_color_g;
		float b = settings.signal_color_b;


		for (int x = 0; x < settings.horizontal_views; x++) {
			for (int y = 0; y < settings.vertical_views; y++) {

				View new_view = View();
				new_view.init(settings.view_buffer_size, programID);
				new_view.set_render_dimensions(x * x_size + settings.view_outer_margin/8, y * y_size + settings.view_outer_margin/8, x_size - margin_resize, y_size - margin_resize);

				////// setting up the data stuff //////
				for (int i = 0; i < (sizeof(g_vertex_buffer_data) / sizeof(g_vertex_buffer_data[0])); i += 2) {
					new_view.background_mesh.positions.push_back(g_vertex_buffer_data[i]);
					new_view.background_mesh.positions.push_back(g_vertex_buffer_data[i + 1]);

					new_view.background_mesh.colors.push_back(outer_r);
					new_view.background_mesh.colors.push_back(outer_g);
					new_view.background_mesh.colors.push_back(outer_b);
				}

				for (int i = 0; i < (sizeof(g_vertex_buffer_data) / sizeof(g_vertex_buffer_data[0])); i += 2) {
					new_view.background_mesh.positions.push_back(g_vertex_buffer_data[i] * (1 - settings.view_inner_margin - settings.view_outer_margin) );
					new_view.background_mesh.positions.push_back(g_vertex_buffer_data[i + 1] * (1 - settings.view_inner_margin - settings.view_outer_margin) );

					new_view.background_mesh.colors.push_back(inner_r);
					new_view.background_mesh.colors.push_back(inner_g);
					new_view.background_mesh.colors.push_back(inner_b);
				}

				new_view.data_mesh.positions.reserve(settings.view_buffer_size * 2);
				new_view.data_mesh.colors.reserve(settings.view_buffer_size * 3);

				float k = (1.0f / settings.view_buffer_size);

				for (int i = 0; i < settings.view_buffer_size; i+=2) {

					new_view.data_mesh.positions.push_back(i * k * 2 * (1 - settings.view_inner_margin - settings.view_outer_margin) - 1 + (settings.view_inner_margin + settings.view_outer_margin));
					new_view.data_mesh.positions.push_back(0);
					
					new_view.data_mesh.colors.push_back(r);
					new_view.data_mesh.colors.push_back(g);
					new_view.data_mesh.colors.push_back(b);
				}

				new_view.set_data();
				/////////////////////////


				views.push_back(new_view);

			}
		}
	};

	setup_views();

	while (!glfwWindowShouldClose(window))
	{




		///////////////////////// RENDERING /////////////////////////

		glClear(GL_COLOR_BUFFER_BIT);
		
		for (int i = 0; i < views.size(); i++) {

			////////TODO DELETE////////
			//float v = ((((float)rand()) / (float)RAND_MAX) * (settings.max_value - settings.min_value)) + settings.min_value;
			//std::vector<float> new_data = { v, v, v, v, v, v, v, v, v, v};
			//views[i].add_data_points(new_data, settings);
			//////////////////////////

			views[i].render();

		}

		/////////////////////////////////////////////////////////////

		////glfw stuff////
		glfwSwapBuffers(window);
		glfwPollEvents();
		//////////////////
	}

	glfwDestroyWindow(window);
	
	return 0;
}

