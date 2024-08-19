#pragma once
#include "Component/Mesh/BoxShape.h"
#include "Common.h"
#include "RenderCommon.h"


namespace tinyGL
{
	class CQuadShape;

	class CSkyBox
	{
	public:
		void Init();

		void Render(const glm::mat4& mvp);
		GLuint GetSkyBoxTextureId() const {return cube_map_id;}
		GLuint GetDiffuseIrradianceTexture() const {return irradiance_tex_id;}
		GLuint GetPrefilterTexture() const {return prefilter_map_id;}
		GLuint GetBRDFLutTexture() const {return  brdf_lut_map_id;}
		
	private:
		// todo：整理一下，现在有点乱
		GLuint shader_id = GL_NONE;
		GLuint sphere_to_cube_shader_id = GL_NONE;
		GLuint cube_map_id = GL_NONE;

		shared_ptr<CBoxShape> box_mesh;
		shared_ptr<CQuadShape> quad_shape;
		
		// 球形环境贴图转换为立方体贴图的帧缓冲
		GLuint preprocess_fbo = GL_NONE;
		GLuint preprocess_rbo = GL_NONE;
		GLuint sphere_map_texture = GL_NONE;
		// ambient light: 辐照图贴图
		GLuint irradiance_shader_id = GL_NONE;
		GLuint irradiance_tex_id = GL_NONE;

		// specular light
		GLuint prefilter_shader_id = GL_NONE;
		GLuint prefilter_map_id = GL_NONE; // 预滤波环境贴图
		// brdf查找表
		GLuint brdf_lut_shader_id = GL_NONE;
		GLuint brdf_lut_map_id = GL_NONE;
	};
}
