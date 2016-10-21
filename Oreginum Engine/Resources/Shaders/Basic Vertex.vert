#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform Uniforms
{
   mat4 model;
   mat4 view;
   mat4 projection;
   vec3 camera_position;
} uniforms;

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv;

layout(location = 0) out vec3 fragment_position;
layout(location = 1) out vec3 fragment_normal;
layout(location = 2) out vec2 fragment_uv;
layout(location = 3) out vec3 fragment_camera_position;

void main()
{
  gl_Position = uniforms.projection*uniforms.view*uniforms.model*vec4(vertex_position, 1);
  fragment_position = vec3(uniforms.model*vec4(vertex_position, 1));
  fragment_normal = normalize(mat3(transpose(inverse(uniforms.model)))*vertex_normal);
  fragment_uv = vertex_uv;
  fragment_camera_position = uniforms.camera_position;
}
