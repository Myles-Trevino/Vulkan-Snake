#version 450
#extension GL_ARB_separate_shader_objects : enable

struct Light
{
   vec3 position;
   vec3 color;
};

layout(binding = 1) uniform sampler2D texture_sampler;

layout(location = 0) in vec3 fragment_position;
layout(location = 1) in vec2 fragment_uv;
layout(location = 2) in vec3 fragment_normal;
layout(location = 3) in Light fragment_lights[2];

layout(location = 0) out vec4 fragment_color;

vec3 linear_to_gamma_space(vec3 color){ return pow(color, vec3(1/2.2f)); }

float lambert_diffuse(vec3 light_direction, vec3 normal)
{ return dot(normalize(light_direction), normalize(normal)); }

void main()
{
   vec3 albedo = texture(texture_sampler, fragment_uv).rgb;
   vec3 diffuse;
   
   for(int i = 0; i < 2; ++i)
   {
      diffuse += fragment_lights[i].color*
	     lambert_diffuse(fragment_lights[i].position-fragment_position, fragment_normal);
   }
   
   fragment_color = vec4(linear_to_gamma_space(albedo*diffuse), 1);
}