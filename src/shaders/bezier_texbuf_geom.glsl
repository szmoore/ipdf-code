#version 150

uniform samplerBuffer bezier_buffer_texture; 
uniform isamplerBuffer bezier_id_buffer_texture; 

layout(lines) in;
layout(line_strip, max_vertices = 101) out;

in int objectid[];
in vec2 pixsize[];

//TODO: I thought this might be useful, maybe not.
float areatriangle(vec2 a, vec2 b, vec2 c)
{
	return (c.y-a.y)*(b.y-a.y) - (b.x - a.x)*(c.x-a.x);
}

void main()
{
	int bezierid = texelFetch(bezier_id_buffer_texture, objectid[0]).r;
	vec2 boundssize = gl_in[1].gl_Position.xy - gl_in[0].gl_Position.xy;
	vec2 coeff0 = texelFetch(bezier_buffer_texture, bezierid*4).rg;
	vec2 coeff1 = texelFetch(bezier_buffer_texture, bezierid*4+1).rg;
	vec2 coeff2 = texelFetch(bezier_buffer_texture, bezierid*4+2).rg;
	vec2 coeff3 = texelFetch(bezier_buffer_texture, bezierid*4+3).rg;

	vec2 boundspxsize = pixsize[0];
	int blen = clamp(int(abs(boundspxsize.x)),2,100);
	float invblen = 1.0f/float(blen);
	for (int i = 0; i <= blen; ++i)
	{
		float t = i * invblen;
		float oneminust = 1.0f - t;
		float bernstein0 = t*t*t;
		float bernstein1 = 3*t*t*oneminust;
		float bernstein2 = 3*t*oneminust*oneminust;
		float bernstein3 = oneminust*oneminust*oneminust;
		gl_Position = vec4((coeff0*bernstein0 + coeff1*bernstein1 + coeff2*bernstein2 + coeff3*bernstein3) * boundssize + gl_in[0].gl_Position.xy, 0.0, 1.0);
		EmitVertex();
		
	}
	EndPrimitive();
}

