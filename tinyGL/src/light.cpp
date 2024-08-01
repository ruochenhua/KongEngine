#include "light.h"

#include "Scene.h"
#include "shader.h"

using namespace tinyGL;
using namespace glm;
const float SHADOWMAP_NEAR_PLANE = 1.0f;
const float SHADOWMAP_FAR_PLANE = 30.0f;

Light::Light(ELightType in_type)
    : shadowmap_fbo(0), shadowmap_texture(0), shadowmap_shader_id(0)
    , near_plane(SHADOWMAP_NEAR_PLANE), far_plane(SHADOWMAP_FAR_PLANE)
    , light_type(in_type)
{
    
}

DirectionalLight::DirectionalLight()
    : Light(ELightType::directional_light)
{
    // init shadow map
    glGenFramebuffers(1, &shadowmap_fbo);
	   
    glGenTextures(1, &shadowmap_texture);
    glBindTexture(GL_TEXTURE_2D, shadowmap_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat border_color[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
	   
    glBindFramebuffer(GL_FRAMEBUFFER, shadowmap_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowmap_texture, 0);
	   
    // 我们需要的只是在从光的透视图下渲染场景的时候深度信息，所以颜色缓冲没有用。
    // 然而，不包含颜色缓冲的帧缓冲对象是不完整的，所以我们需要显式告诉OpenGL我们不适用任何颜色数据进行渲染。
    // 我们通过将调用glDrawBuffer和glReadBuffer把读和绘制缓冲设置为GL_NONE来做这件事。
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	   
    // if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    // 	return -1;
    //
    map<SRenderResourceDesc::EShaderType, string> shader_paths = {
        {SRenderResourceDesc::EShaderType::vs, CSceneLoader::ToResourcePath("shader/shadowmap.vert")},
        {SRenderResourceDesc::EShaderType::fs, CSceneLoader::ToResourcePath("shader/shadowmap.frag")}
    };
    shadowmap_shader_id = Shader::LoadShaders(shader_paths);
}

glm::vec3 DirectionalLight::GetLightDir() const
{
    // rotation to direction
    /*
    * x = cos(yaw)*cos(pitch)
    * y = sin(yaw)*cos(pitch)
    * z = sin(pitch)
     */
    // vec3 dir;
    // dir.x = cos(rotation.z) * cos(rotation.y);
    // dir.y = sin(rotation.z) * cos(rotation.y);
    // dir.z = sin(rotation.y);
    // return normalize(dir);
    return normalize(rotation);
}

void DirectionalLight::RenderShadowMap()
{
    glBindFramebuffer(GL_FRAMEBUFFER, shadowmap_fbo);
    glClear(GL_DEPTH_BUFFER_BIT);
    // 光线投影采用正交投影矩阵
    mat4 light_proj = ortho(-20.f, 20.f, -20.f, 20.f, near_plane, far_plane);
    vec3 light_dir = GetLightDir();	// 默认写一个

    vec3 light_pos = light_dir * -5.f;
    mat4 light_view = lookAt(light_pos, vec3(0,0,0), vec3(0, 1, 0));
    light_space_mat = light_proj * light_view;

    glUseProgram(shadowmap_shader_id);
    Shader::SetMat4(shadowmap_shader_id, "light_space_mat", light_space_mat);
    // RenderScene();

    auto render_objs = CScene::GetScene()->GetSceneRenderObjects();
    for(auto render_obj : render_objs)
    {
        for(auto& mesh : render_obj->mesh_list)
        {
            const SRenderInfo& render_info = mesh.GetRenderInfo();
            glBindVertexArray(render_info.vertex_array_id);	// 绑定VAO
		
            mat4 model_mat = render_obj->GetModelMatrix();
            // mat4 mvp = projection_mat * mainCamera->GetViewMatrix() * model_mat; //
			
            Shader::SetMat4(shadowmap_shader_id, "model", model_mat);
            // Draw the triangle !
            // if no index, use draw array
            if(render_info.index_buffer == GL_NONE)
            {
                glDrawArrays(GL_TRIANGLES, 0, render_info.vertex_size / render_info.stride_count); // Starting from vertex 0; 3 vertices total -> 1 triangle	
            }
            else
            {		
                glDrawElements(GL_TRIANGLES, render_info.indices_count, GL_UNSIGNED_INT, 0);
            }
        }
        glBindVertexArray(GL_NONE);	// 解绑VAO
    }
	
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

PointLight::PointLight()
    : Light(ELightType::point_light)
{
    // 创建阴影贴图相关的framebuffer
    // 由于点光源的方向是四面八方的，所以点光源的shadowmap是一个立方体贴图
    glGenFramebuffers(1, &shadowmap_fbo);
    // 创建点光源阴影贴图
    glGenTextures(1, &shadowmap_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowmap_texture);
    for(GLuint i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
            SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, shadowmap_fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowmap_texture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


vec3 PointLight::GetLightDir() const
{
    // point light has no direction
    return vec3(0);
}

void PointLight::RenderShadowMap()
{
}
