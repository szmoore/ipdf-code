#ifndef _SHADERPROGRAM_H
#define _SHADERPROGRAM_H

#include <vector>
#include "gl_core44.h"


namespace IPDF
{
	/**
	 * The "Shader" class represents a GLSL program made from shaders.
	 */
	class ShaderProgram
	{
	public:
		ShaderProgram() : m_program(0), m_valid(false) {};
		~ShaderProgram();
		bool InitialiseShaders(const char * vert_glsl_file, const char * frag_glsl_file, const char * geom_glsl_file = "");
		const void Use() const;

		bool Valid() const {return m_valid;}
		
		// Unfortunately, we don't require GL 4.3/ARB_explicit_uniform_location
		// which would make this obsolete. One uday Mesa will support it.
		// NOTE: We could actually get away with this by only using UBOs, as
		// Mesa supports ARB_shading_language_420pack, but that'd be a bit more
		// work right with the way some of our uniforms are laid out at the moment.
		const GLint GetUniformLocation(const char *uniform_name) const;

	private:
		char * GetShaderSource(const char * src_file) const;
		bool AttachShader(const char * src_file, GLenum type);

		GLuint m_program;
		struct Shader
		{
			GLenum type;
			GLuint obj;
		};
		std::vector<Shader> m_shaders;
		bool m_valid;
	};

}

#endif // _SHADERPROGRAM_H
