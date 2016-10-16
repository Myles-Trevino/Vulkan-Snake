#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texture_sampler;

layout(location = 0) in vec3 fragment_position;
layout(location = 1) in vec3 fragment_normal;
layout(location = 2) in vec2 fragment_uv;
layout(location = 3) in vec3 fragment_camera_position;

layout(location = 0) out vec4 out_color;

void main()
{
   //Attenuation
   float light_distance = length(fragment_camera_position-fragment_position);
   float attenuation = 1.0/(1+0.09*light_distance+0.032*(light_distance*light_distance));
   
   //Ambient
   vec3 ambient_color = vec3(0.1, 0.15, 0.2);
   vec3 ambient = ambient_color*texture(texture_sampler, fragment_uv).rgb;

   //Diffuse
   vec3 diffuse_color = vec3(0.9, 0.85, 0.8);
   vec3 light_direction = normalize(fragment_camera_position-fragment_position);
   float diffuse_brightness = max(dot(fragment_normal, light_direction), 0);
   vec3 diffuse = attenuation*diffuse_color*diffuse_brightness*texture(texture_sampler, fragment_uv).rgb;

   //Specular
   vec3 specular_color = vec3(1, 1, 1);
   vec3 view_direction = normalize(fragment_camera_position-fragment_position);
   vec3 reflect_direction = reflect(-light_direction, fragment_normal);
   float specular_brightness = pow(max(dot(view_direction, reflect_direction), 0), 6);
   vec3 specular = attenuation*specular_color*specular_brightness*texture(texture_sampler, fragment_uv).rgb;

   out_color = vec4(ambient+diffuse+specular, 1);
}