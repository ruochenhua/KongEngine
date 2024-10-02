#pragma once
#include "Component/Mesh/BoxShape.h"
#include "Common.h"
#include "RenderCommon.h"
#include "Component/Mesh/VolumetricCloud.h"
#include "Shader/SkyboxShader.h"


namespace Kong
{
	class CQuadShape;
	enum ESkyEnvStatus
	{
		no_sky = 0,
		skybox,
		atmosphere,
	};
	
	class CSkyBox
	{
	public:
		void Init();
		// IBL预处理HDR相关
		void PreprocessIBL(const string& hdr_file_path);
		void Render(const glm::mat4& mvp, int render_sky_status);
		void ChangeSkybox();

		void PreRenderUpdate();
		
		GLuint GetSkyBoxTextureId() const {return cube_map_id;}
		GLuint GetDiffuseIrradianceTexture() const {return irradiance_tex_id;}
		GLuint GetPrefilterTexture() const {return prefilter_map_id;}
		GLuint GetBRDFLutTexture() const {return  brdf_lut_map_id;}

		bool render_cloud = true;
	private:
		// skybox渲染相关shader
		shared_ptr<SkyboxShader> skybox_shader;
		shared_ptr<EquirectangularToCubemapShader> equirectangular_to_cubemap_shader;
		shared_ptr<IrradianceCalculationShader> irradiance_calculation_shader;
		shared_ptr<PrefilterCalculationShader> prefilter_calculation_shader;
		shared_ptr<BRDFLutCalculationShader> brdf_lut_calculation_shader;

		// skybox立方体贴图
		GLuint cube_map_id = GL_NONE;

		shared_ptr<VolumetricCloud> volumetric_cloud_;
		
		// skybox渲染用到的mesh信息
		shared_ptr<CBoxShape> box_mesh;
		shared_ptr<CQuadShape> quad_shape;
		
		// skybox贴图预处理帧缓冲
		GLuint preprocess_fbo = GL_NONE;
		GLuint preprocess_rbo = GL_NONE;
		GLuint sphere_map_texture = GL_NONE;

		// ambient light: 辐照图贴图
		GLuint irradiance_tex_id = GL_NONE;
		
		// specular light
		// 预滤波环境贴图
		GLuint prefilter_map_id = GL_NONE; 
		// brdf查找表
		GLuint brdf_lut_map_id = GL_NONE;

		unsigned current_skybox_idx = 0;
		vector<string> skybox_res_list;
	};
}
