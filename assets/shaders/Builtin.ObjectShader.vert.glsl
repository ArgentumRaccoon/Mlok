#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in  vec3 in_position;

layout(set = 0, binding = 0) uniform global_uniform_object
{
    mat4 Projection;
    mat4 View;
} global_ubo; // applied to the whole scene

void main()
{
    gl_Position = global_ubo.Projection * global_ubo.View * vec4(in_position, 1.f);
}