#version 330
#extension GL_ARB_separate_shader_objects : enable

out vec3 color;

layout(location = 1) in vec3 vertex_color;

in vec3 frag_color;

void main(){
  color = frag_color;
}
