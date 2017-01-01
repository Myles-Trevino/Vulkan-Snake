#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texture_sampler;

layout(location = 0) in vec2 texture_coordinates;

layout(location = 0) out vec4 fragment_color;

void main()
{
    fragment_color = texture(texture_sampler, texture_coordinates);
}