#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform Uniform
{ mat4 model, view, projection; vec3 camera; } uniforms;

layout(location = 0) in vec2 position;

layout(location = 0) out vec3 frag_camera_direction;

out gl_PerVertex{ vec4 gl_Position; };

void main()
{
   mat4 inverse_projection = inverse(uniforms.projection);
   mat3 inverse_model_view = transpose(mat3(uniforms.view*uniforms.model));
   vec3 unprojected = (inverse_projection*vec4(position, 0, 1)).xyz;
   frag_camera_direction = inverse_model_view*unprojected;

   gl_Position = vec4(position, 0, 1);
}