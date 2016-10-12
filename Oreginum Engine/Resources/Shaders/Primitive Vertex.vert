#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform Uniforms{ mat4 mvp; } uniforms;

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec2 vertex_uv;

layout(location = 0) out vec2 fragment_uv;

void main()
{
   gl_Position = uniforms.mvp*vec4(vertex_position, 1.0);
   fragment_uv = vertex_uv;
}
