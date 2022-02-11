#pragma once

#include "GL/glew.h"

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path);
GLuint GenShaders(const char* vertex_file_string, const char* fragment_file_string);
