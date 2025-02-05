#pragma once
#include "Component/Mesh/BoxShape.h"
#include "Common.h"
#include "RenderCommon.hpp"
#include "RenderSystem.hpp"
#include "Component/Mesh/VolumetricCloud.h"
#include "Shader/SkyboxShader.h"


namespace Kong
{
	enum ESkyEnvStatus
	{
		no_sky = 0,
		skybox,
		atmosphere,
	};
	
	class SkyboxRenderSystem : public KongRenderSystem
	{
	public:
		SkyboxRenderSystem();
		void Init() override;
		void DrawUI() override;
		RenderResultInfo Draw(double delta, const RenderResultInfo& render_result_info, KongRenderModule* render_module) override;
		// IBL预处理HDR相关
		void PreprocessIBL(const string& hdr_file_path);
		void Render( int render_sky_status, GLuint depth_texture);
		void RenderCloud(GLuint depth_texture);
		void ChangeSkybox();

		void PreRenderUpdate();
		
		GLuint GetSkyBoxTextureId() const {return cube_map_id;}
		GLuint GetDiffuseIrradianceTexture() const {return irradiance_tex_id;}
		GLuint GetPrefilterTexture() const {return prefilter_map_id;}
		GLuint GetBRDFLutTexture() const {return  brdf_lut_map_id;}

		bool render_cloud = false;
		int render_sky_env_status = 2;
	private:
		// skybox渲染相关shader
		shared_ptr<SkyboxShader> skybox_shader;
		shared_ptr<AtmosphereShader> atmosphere_shader;
		
		shared_ptr<EquirectangularToCubemapShader> equirectangular_to_cubemap_shader;
		shared_ptr<IrradianceCalculationShader> irradiance_calculation_shader;
		shared_ptr<PrefilterCalculationShader> prefilter_calculation_shader;
		shared_ptr<BRDFLutCalculationShader> brdf_lut_calculation_shader;

		// skybox立方体贴图
		GLuint cube_map_id = GL_NONE;

		shared_ptr<VolumetricCloud> volumetric_cloud_;
		
		// skybox渲染用到的mesh信息
		shared_ptr<CBoxShape> box_mesh;
				
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
