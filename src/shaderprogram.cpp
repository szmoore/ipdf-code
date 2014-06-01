#include "shaderprogram.h"
#include "log.h"

using namespace IPDF;

ShaderProgram::~ShaderProgram()
{
	for(auto shader : m_shaders)
	{
		glDetachShader(m_program, shader.obj);
		glDeleteShader(shader.obj);
	}

	if (m_program)
		glDeleteProgram(m_program);
}

void ShaderProgram::LazyCreateProgram()
{
	if (!m_program)
	{
		m_program = glCreateProgram();
	}
}

bool ShaderProgram::AttachShader(const char *src, GLenum type)
{
	GLuint shader_obj = glCreateShader(type);
	glShaderSource(shader_obj, 1, &src, 0);
	glCompileShader(shader_obj);
	int did_compile = 0;
	glGetShaderiv(shader_obj, GL_COMPILE_STATUS, &did_compile);
	if (!did_compile)
	{
		char info_log[2048];

		glGetShaderInfoLog(shader_obj, 2048, nullptr, info_log);
		Error("Shader compile error: %s\n", info_log);
		return false;
	}

	m_shaders.push_back(Shader{type, shader_obj});
	LazyCreateProgram();
	glAttachShader(m_program, shader_obj);
	return true;
}

bool ShaderProgram::AttachVertexProgram(const char *src)
{
	return AttachShader(src, GL_VERTEX_SHADER);
}

bool ShaderProgram::AttachFragmentProgram(const char *src)
{
	return AttachShader(src, GL_FRAGMENT_SHADER);
}

bool ShaderProgram::AttachGeometryProgram(const char *src)
{
	return AttachShader(src, GL_GEOMETRY_SHADER);
}

bool ShaderProgram::Link()
{
	glLinkProgram(m_program);
	return true;
}

const void ShaderProgram::Use() const
{
	glUseProgram(m_program);
}

const GLint ShaderProgram::GetUniformLocation(const char *name) const
{
	return glGetUniformLocation(m_program, name);
}
