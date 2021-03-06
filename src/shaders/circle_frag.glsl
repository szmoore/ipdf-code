#version 140

in vec2 objcoords;
out vec4 output_colour;

uniform vec4 colour;

void main()
{
	if ((objcoords.x)*(objcoords.x) + (objcoords.y)*(objcoords.y) > 1.0)
	{
		discard;
	}
	output_colour = colour;
}
