#include "shaderprogram.h"
#include "log.h"

using namespace IPDF;

/**
 * Initialise a shader program using GLSL source files
 * @param geometry_file GLSL source for Geometry shader (optional)
 * @param vertex_file GLSL source for vertex shader
 * @param fragment_file GLSL source for fragment shader
 * If a filename is the empty string it will be ignored
 */
bool ShaderProgram::InitialiseShaders(const char * vertex_file, const char * fragment_file, const char * geometry_file)
{
	if (m_valid)
	{
		Error("Shader already valid?");
	}
	m_program = glCreateProgram();
	if (m_program == 0)
	{
		Error("glCreateProgram failed");
		m_valid = false;
		return m_valid;
	}
	m_valid = true;
	if (geometry_file != NULL && geometry_file[0] != '\0')
		m_valid &= AttachShader(geometry_file, GL_GEOMETRY_SHADER);
	if (vertex_file != NULL && vertex_file[0] != '\0')
		m_valid &= AttachShader(vertex_file, GL_VERTEX_SHADER);
	if (fragment_file != NULL && fragment_file[0] != '\0')
		m_valid &= AttachShader(fragment_file, GL_FRAGMENT_SHADER);

	if (!m_valid)
	{
		Warn("One or more AttachShader calls failed but we will link the shader anyway");
	}
	glLinkProgram(m_program);
	return m_valid;
}



/**
 * Destroy a shader program
 */
ShaderProgram::~ShaderProgram()
{
	m_valid = false;
	for(auto shader = m_shaders.begin(); shader != m_shaders.end(); ++shader)
	{
		glDetachShader(m_program, shader->obj);
		glDeleteShader(shader->obj);
	}

	if (m_program)
		glDeleteProgram(m_program);
}

/**
 * Get GLSL shader source from a file as a string
 * @param src_file filename to get the shader source from
 * @returns a char* string allocated with new[] (remember to delete[] it)
 */
char * ShaderProgram::GetShaderSource(const char * src_file) const
{
	char * src = NULL;
	FILE * file = fopen(src_file, "r");
	if (file == NULL)
	{
		Error("Could not open shader source file \"%s\": %s", src_file, strerror(errno));
		return NULL;
	}
	long start = ftell(file);

	if (fseek(file, 0, SEEK_END) != 0)
	{
		Error("Couldn't seek to end of file \"%s\": %s", src_file, strerror(errno));
		return NULL;
	}
	long end = ftell(file);
	if (start < 0 || end < 0 || end < start)
	{
		// I bet that now I've put this message here, it will occur at least once in the life of this code
		Error("Insane results from ftell(3) on file \"%s\"", src_file);
		return NULL;
	}
	size_t size = end - start;
	src = new char[size+1]; // Warning! Allocation of memory occuring! We might all die!
	rewind(file);
	if (fread(src, 1,size, file) != size)
	{
		Error("Error in fread on \"%s\": %s", src_file, strerror(errno));
		fclose(file);
		delete [] src;
		return NULL;
	}
	src[size] = '\0';
	fclose(file);
	return src;
	
}

bool ShaderProgram::AttachShader(const char * src_file, GLenum type)
{
	GLuint shader_obj = glCreateShader(type);
	glObjectLabel(GL_SHADER, shader_obj, -1, src_file);
	char * src = GetShaderSource(src_file);
	if (src == NULL)
	{
		Error("Couldn't get shader source.");
		return false;
	}
	

	glShaderSource(shader_obj, 1, &src, 0);
	glCompileShader(shader_obj);
	int did_compile = 0;
	glGetShaderiv(shader_obj, GL_COMPILE_STATUS, &did_compile);
	delete [] src; // Oh my goodness memory management is hard guys
	if (!did_compile)
	{
		char info_log[2048];

		glGetShaderInfoLog(shader_obj, 2048, NULL, info_log);
		Error("Shader compile error (file \"%s\"): %s (type %d)", src_file, info_log, type);
		return false;
	}

	m_shaders.push_back(Shader{type, shader_obj});
	glAttachShader(m_program, shader_obj);
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
