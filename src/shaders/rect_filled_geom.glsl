#version 150

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

void main()
{
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	gl_Position = vec4(gl_in[0].gl_Position.x, gl_in[1].gl_Position.y, 0.0, 1.0);
	EmitVertex();
	gl_Position = vec4(gl_in[1].gl_Position.x, gl_in[0].gl_Position.y, 0.0, 1.0);
	EmitVertex();
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();
	EndPrimitive();
}
