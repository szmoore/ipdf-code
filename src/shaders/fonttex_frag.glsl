#version 140

in vec2 fp_tex_coord;

out vec4 output_colour;

uniform sampler2D tex;
uniform vec4 colour;

void main()
{
	output_colour = colour;
	output_colour.a = texture(tex, fp_tex_coord).r;
}

