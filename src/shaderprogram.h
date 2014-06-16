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
		ShaderProgram() : m_program(0) {}
		~ShaderProgram();

		inline bool AttachGeometryProgram(const char * geometry_file) {return AttachShader(geometry_file, GL_GEOMETRY_SHADER);}
		inline bool AttachVertexProgram(const char * vertex_file) {return AttachShader(vertex_file, GL_VERTEX_SHADER);}
		inline bool AttachFragmentProgram(const char * fragment_file) {return AttachShader(fragment_file, GL_FRAGMENT_SHADER);}

		/** Read shaders from files and attach them
		 * @returns false if any of the shaders cannot be attached
		 */
		inline bool AttachShaderPrograms(const char * geometry_file, const char * vertex_file, const char * fragment_file)
		{
			return AttachGeometryProgram(geometry_file) && AttachVertexProgram(vertex_file) && AttachFragmentProgram(fragment_file);
		}
		bool Link(); // currently always returns true?
		const void Use() const;
		// Unfortunately, we don't require GL 4.3/ARB_explicit_uniform_location
		// which would make this obsolete. One uday Mesa will support it.
		// NOTE: We could actually get away with this by only using UBOs, as
		// Mesa supports ARB_shading_language_420pack, but that'd be a bit more
		// work right with the way some of our uniforms are laid out at the moment.
		const GLint GetUniformLocation(const char *uniform_name) const;

	private:
		void LazyCreateProgram();
		/** Read shader source from src_file and attach it as type **/
		bool AttachShader(const char * src_file, GLenum type);
		char * GetShaderSource(const char * src_file) const;
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
