#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform samplerCube cubemap_sampler;

layout(location = 0) in vec3 frag_camera_direction;

layout(location = 0) out vec4 fragment_color;

void main()
{
    fragment_color = texture(cubemap_sampler, vec3(frag_camera_direction.x,
	   -frag_camera_direction.y, frag_camera_direction.z));
}