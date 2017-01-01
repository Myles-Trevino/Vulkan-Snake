#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform Uniform{ mat4 matrix; vec3 color; } uniforms;

layout(location = 0) in vec2 position;

layout(location = 0) out vec2 texture_coordinates;

out gl_PerVertex{ vec4 gl_Position; };

void main()
{
   gl_Position = uniforms.matrix*vec4(position, 0, 1);
   texture_coordinates = position;
}