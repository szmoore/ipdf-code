#version 140
#extension GL_ARB_shading_language_420pack : require
#extension GL_ARB_explicit_attrib_location : require

layout(std140, binding=0) uniform ViewBounds
{
	float bounds_x;
	float bounds_y;
	float bounds_w;
	float bounds_h;
};

layout(location = 0) in vec2 position;

void main()
{
	vec2 transformed_position;
	transformed_position.x = (position.x - bounds_x) / bounds_w;
	transformed_position.y = (position.y - bounds_y) / bounds_h;
	// Transform to clip coordinates (-1,1, -1,1).
	gl_Position.x = (transformed_position.x*2) - 1;
	gl_Position.y = 1 - (transformed_position.y*2);
	gl_Position.z = 0.0;
	gl_Position.w = 1.0;
}
