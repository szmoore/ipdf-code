#version 150

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;
out vec2 objcoords;

void main()
{
	gl_Position = gl_in[0].gl_Position;
	objcoords = vec2(-1.0, -1.0);
	EmitVertex();
	gl_Position = vec4(gl_in[0].gl_Position.x, gl_in[1].gl_Position.y, 0.0, 1.0);
	objcoords = vec2(-1.0, 1.0);
	EmitVertex();
	gl_Position = vec4(gl_in[1].gl_Position.x, gl_in[0].gl_Position.y, 0.0, 1.0);
	objcoords = vec2(1.0, -1.0);
	EmitVertex();
	gl_Position = gl_in[1].gl_Position;
	objcoords = vec2(1.0, 1.0);
	EmitVertex();
	EndPrimitive();
}
