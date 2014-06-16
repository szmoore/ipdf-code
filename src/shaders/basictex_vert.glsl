#version 140
#extension GL_ARB_shading_language_420pack : require
#extension GL_ARB_explicit_attrib_location : require

layout(std140, binding=0) uniform Viewport
{
	float width;
	float height;
};

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 tex_coord;

out vec2 fp_tex_coord;

void main()
{
	// Transform to clip coordinates (-1,1, -1,1).
	gl_Position.x = (position.x*2/width) - 1;
	gl_Position.y = 1 - (position.y*2/height);
	gl_Position.z = 0.0;
	gl_Position.w = 1.0;
	fp_tex_coord = tex_coord;
}

