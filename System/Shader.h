#ifndef _RTE_SHADER_
#define _RTE_SHADER_

#include "Serializable.h"
#include "glm/glm.hpp"

namespace RTE {
	class Shader : public Serializable {
	public:
		SerializableOverrideMethods
		    SerializableClassNameGetter

		    Shader();
		Shader(const std::string &vertexFilename, const std::string &fragPath);
		virtual ~Shader();

		int Create() override;
		int Create(const std::string &vertexFilename, const std::string &fragPath);

		bool Compile(const std::string &vertexFilename, const std::string &fragPath);

		void Use();
#pragma region Uniform handling
		/// <summary>
		/// Returns the location of a uniform given by name.
		/// </summary>
		/// <param name="name">
		/// String containing the name of a uniform in this shader program.
		/// </param>
		/// <returns>
		/// A GLint containing the location of the requested uniform.
		/// </returns>
		int32_t GetUniformLocation(const std::string &name);

		/// <summary>
		/// Set a boolean uniform value in the active program by name.
		/// </summary>
		/// <param name="name">
		/// The name of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The boolean value to set the uniform to.
		/// </param>
		void SetBool(const std::string &name, bool value);

		/// <summary>
		/// Set an integer uniform value in the active program by name.
		/// </summary>
		/// <param name="name">
		/// The name of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The integer value to set the uniform to.
		/// </param>
		void SetInt(const std::string &name, int value);

		/// <summary>
		/// Set a float uniform value in the active program by name.
		/// </summary>
		/// <param name="name">
		/// The name of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The float value to set the uniform to.
		/// </param>
		void SetFloat(const std::string &name, float value);

		/// <summary>
		/// Set a float mat4 uniform value in the active program by name.
		/// </summary>
		/// <param name="name">
		/// The name of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The float mat4 value to set the uniform to.
		/// </param>
		void SetMatrix4f(const std::string &name, const glm::mat4 &value);

		/// <summary>
		/// Set a float mat4 uniform value in the active program by name.
		/// </summary>
		/// <param name="name">
		/// The name of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The float mat4 value to set the uniform to.
		/// </param>
		void SetMatrix(const std::string &name, const glm::mat4 &value) { SetMatrix4f(name, value); }

		void SetNMatrix(int32_t uniform, size_t n, const GLfloat* values);

		/// <summary>
		/// Set a float vec2 uniform value in the active program by name.
		/// </summary>
		/// <param name="name">
		/// The name of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The float vec2 value to set the uniform to.
		/// </param>
		void SetVector2f(const std::string &name, const glm::vec2 &value);

		/// <summary>
		/// Set a float vec3 uniform value in the active program by name.
		/// </summary>
		/// <param name="name">
		/// The name of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The float vec3 value to set the uniform to.
		/// </param>
		void SetVector3f(const std::string &name, const glm::vec3 &value);

		/// <summary>
		/// Set a float vec4 uniform value in the active program by name.
		/// </summary>
		/// <param name="name">
		/// The name of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The float vec4 value to set the uniform to.
		/// </param>
		void SetVector4f(const std::string &name, const glm::vec4 &value);

		/// <summary>
		/// Set a boolean uniform value in the active program by location.
		/// </summary>
		/// <param name="uniformLoc">
		/// The location of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The boolean value to set the uniform to.
		/// </param>
		static void SetBool(int32_t uniformLoc, bool value);

		/// <summary>
		/// Set an integer uniform value in the active program by location.
		/// </summary>
		/// <param name="uniformLoc">
		/// The location of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The integer value to set the uniform to.
		/// </param>
		static void SetInt(int32_t uniformLoc, int value);

		/// <summary>
		/// Set a float uniform value in the active program by location.
		/// </summary>
		/// <param name="uniformLoc">
		/// The location of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The float value to set the uniform to.
		/// </param>
		static void SetFloat(int32_t uniformLoc, float value);

		/// <summary>
		/// Set a float mat4 uniform value in the active program by location.
		/// </summary>
		/// <param name="uniformLoc">
		/// The location of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The float mat4 value to set the uniform to.
		/// </param>
		static void SetMatrix4f(int32_t uniformLoc, const glm::mat4 &value);

		/// <summary>
		/// Set a float mat4 uniform value in the active program by location.
		/// </summary>
		/// <param name="uniformLoc">
		/// The location of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The float mat4 value to set the uniform to.
		/// </param>
		static void SetMatrix(int32_t uniformLoc, const glm::mat4 &value) { SetMatrix4f(uniformLoc, value); };

		/// <summary>
		/// Set a float vec2 uniform value in the active program by location.
		/// </summary>
		/// <param name="uniformLoc">
		/// The location of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The float vec2 value to set the uniform to.
		/// </param>
		static void SetVector2f(int32_t uniformLoc, const glm::vec2 &value);

		/// <summary>
		/// Set a float vec3 uniform value in the active program by location.
		/// </summary>
		/// <param name="uniformLoc">
		/// The location of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The float vec3 value to set the uniform to.
		/// </param>
		static void SetVector3f(int32_t uniformLoc, const glm::vec3 &value);

		/// <summary>
		/// Set a float vec4 uniform value in the active program by location.
		/// </summary>
		/// <param name="uniformLoc">
		/// The location of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The float vec4 value to set the uniform to.
		/// </param>
		static void SetVector4f(int32_t uniformLoc, const glm::vec4 &value);
#pragma endregion

#pragma region Engine Defined Uniforms

		/// <summary>
		/// Get the location of the texture sampler uniform.
		/// </summary>
		/// <returns>
		/// The location of the texture uniform. This may be -1 if the shader doesn't use textures, in which case the value must not be set.
		/// </returns>
		int GetTextureUniform() { return m_TextureUniform; }

		/// <summary>
		/// Get the location of the color modifier uniform.
		/// </summary>
		/// <returns>
		/// The location of the color modifier uniform. This may be -1 if the shader doesn't use the color mod, in which case the value must not be set.
		/// </returns>
		int GetColorUniform() { return m_ColorUniform; }

		/// <summary>
		/// Get the location of the transformation matrix uniform.
		/// </summary>
		/// <returns>
		/// The location of the transformation matrix uniform. This may be -1 if the shader doesn't use transforms, in which case the value must not be set.
		/// </returns>
		int GetTransformUniform() { return m_TransformUniform; }

		/// <summary>
		/// Get the location of the transformation matrix uniform.
		/// </summary>
		/// <returns>
		/// The location of the UV transformation matrix uniform. This may be -1 if the shader doesn't use UV transforms, in which case the value must not be set.
		/// </returns>
		int GetUVTransformUniform() { return m_UVTransformUniform; }

		/// <summary>
		/// Get the location of the projection matrix uniform.
		/// </summary>
		/// <returns>
		/// The location of the color modifier uniform. This may be -1 if the shader doesn't apply projection, in which case the value must not be set.
		/// </returns>
		int GetProjectionUniform() { return m_ProjectionUniform; }

#pragma endregion
	private:
		uint32_t m_ProgramID;

		bool CompileShader(uint32_t shaderID, const std::string &data, std::string &error);
		bool Link(uint32_t vtxShader, uint32_t fragShader);
		void ApplyDefaultUniforms();

		std::string m_VertexFile;
		std::string m_FragmentFile;

		int m_TextureUniform; //!< Location of the texture uniform (sampler2d rteTexture).
		int m_ColorUniform; //!< Location of the colormod uniform (vec4 rteColor).
		int m_TransformUniform; //!< Location of the transform uniform (mat4 rteTransform).
		int m_UVTransformUniform; //!< Location of the UV transform uniform (mat4 rteUVTransform).
		int m_ProjectionUniform; //!< Location of the projection uniform (mat4 rteProjection).

		static const std::string c_ClassName;
	};
} // namespace RTE
#endif
