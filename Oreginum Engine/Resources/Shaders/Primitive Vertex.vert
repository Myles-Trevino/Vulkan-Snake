#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform Uniforms{ mat4 mvp; } uniforms;

layout(location = 0) in vec2 vertex_position;
layout(location = 1) in vec3 vertex_color;
layout(location = 2) in vec2 vertex_uv;

layout(location = 0) out vec3 fragment_color;
layout(location = 1) out vec2 fragment_uv;

void main()
{
   gl_Position = uniforms.mvp*vec4(vertex_position, 0.0, 1.0);
   fragment_color = vertex_color;
   fragment_uv = vertex_uv;
}
