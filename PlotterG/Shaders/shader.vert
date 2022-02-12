
#version 330
#extension GL_ARB_separate_shader_objects : enable

uniform vec2 position;
uniform vec2 scale = vec2(1.0,1.0);

layout(location = 0) in vec2 vertex_position;
layout(location = 1) in vec3 vertex_color;

out vec3 frag_color;

void main(){
    gl_Position.xy = ((vertex_position + 1) * scale) + (position * 2 - 1);
    gl_Position.z = 0;
    gl_Position.w = 1.0;
    
    frag_color = vertex_color;
}
