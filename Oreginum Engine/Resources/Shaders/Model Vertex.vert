#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform Uniform
{ mat4 model, view, projection; vec3 camera; } uniforms;

struct Light
{
   vec3 position;
   vec3 color;
};

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec2 vertex_uv;
layout(location = 2) in vec3 vertex_normal;

layout(location = 0) out vec3 fragment_position;
layout(location = 1) out vec2 fragment_uv;
layout(location = 2) out vec3 fragment_normal;
layout(location = 3) out Light fragment_lights[2];

out gl_PerVertex{ vec4 gl_Position; };

void main()
{
   vec4 position = uniforms.view*uniforms.model*vec4(vertex_position, 1);
   
   fragment_position = position.xyz;
   fragment_uv = vertex_uv;
   fragment_normal = mat3(uniforms.view*uniforms.model)*vertex_normal;

   Light lights[2] = Light[2](Light(mat3(uniforms.view)*vec3(3, -3, 0), vec3(1, 0, 0)),
      Light(mat3(uniforms.view)*vec3(-3, -3, 0), vec3(0, 1, 0)));
   for(int i = 0; i < 2; ++i){ fragment_lights[i] = lights[i]; }  

   gl_Position = uniforms.projection*position;
}