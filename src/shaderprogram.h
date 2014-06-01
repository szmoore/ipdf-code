#ifndef _SHADERPROGRAM_H
#define _SHADERPROGRAM_H

#include <vector>
#include "gl_core44.h"


namespace IPDF
{
	/*
	 * The "Shader" class represents a GLSL program made from shaders. 
	 */
	class ShaderProgram
	{
	public:
		ShaderProgram() : m_program(0) {}
		~ShaderProgram();
		bool AttachVertexProgram(const char *src);
		bool AttachFragmentProgram(const char *src);
		bool AttachGeometryProgram(const char *src);
		bool Link();
		const void Use() const;
		// Unfortunately, we don't require GL 4.3/ARB_explicit_uniform_location
		// which would make this obsolete. One uday Mesa will support it.
		// NOTE: We could actually get away with this by only using UBOs, as
		// Mesa supports ARB_shading_language_420pack, but that'd be a bit more
		// work right with the way some of our uniforms are laid out at the moment.
		const GLint GetUniformLocation(const char *uniform_name) const;
	private:
		void LazyCreateProgram();
		bool AttachShader(const char *src, GLenum type);
		GLuint m_program;
		struct Shader
		{
			GLenum type;
			GLuint obj;
		};
		std::vector<Shader> m_shaders;
	};

}

#endif // _SHADERPROGRAM_H
