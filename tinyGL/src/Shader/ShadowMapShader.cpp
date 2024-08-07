#include "ShadowMapShader.h"

#include "Scene.h"

using namespace tinyGL;
using namespace glm;

void DirectionalLightShadowMapShader::InitDefaultShader()
{
    shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/shadowmap.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/shadowmap.frag")}
    };
    
    shader_id = LoadShaders(shader_path_map);

    assert(shader_id, "Shader load failed!");

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
}

void DirectionalLightShadowMapShader::UpdateShadowMapRender(const glm::vec3& light_direction,
    const glm::mat4& model_mat)
{

}

void PointLightShadowMapShader::InitDefaultShader()
{
    shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/shadowmap_pointlight.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/shadowmap_pointlight.frag")},
        {EShaderType::gs, CSceneLoader::ToResourcePath("shader/shadowmap_pointlight.geom")}
    };
    shader_id = LoadShaders(shader_path_map);

    assert(shader_id, "Shader load failed!");

    glGenFramebuffers(1, &shadowmap_fbo);
    // 创建点光源阴影贴图
    glGenTextures(1, &shadowmap_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowmap_texture);
    for(GLuint i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
            SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    

    glBindFramebuffer(GL_FRAMEBUFFER, shadowmap_fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowmap_texture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PointLightShadowMapShader::UpdateShadowMapRender(const glm::vec3& location, const glm::mat4& model_mat)
{
    // 点光源的阴影贴图
    GLfloat aspect = (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT;
    mat4 shadow_proj = perspective(radians(90.f), aspect, near_plane, far_plane);
    
    // 方向可以固定是朝向六个方向
    vector<mat4> shadow_transforms;
    shadow_transforms.push_back(shadow_proj * lookAt(location, location+vec3(1,0,0), vec3(0,-1,0)));
    shadow_transforms.push_back(shadow_proj * lookAt(location, location+vec3(-1,0,0), vec3(0,-1,0)));
    shadow_transforms.push_back(shadow_proj * lookAt(location, location+vec3(0,1,0), vec3(0,0,1)));
    shadow_transforms.push_back(shadow_proj * lookAt(location, location+vec3(0,-1,0), vec3(0,0,-1)));
    shadow_transforms.push_back(shadow_proj * lookAt(location, location+vec3(0,0,1), vec3(0,-1,0)));
    shadow_transforms.push_back(shadow_proj * lookAt(location, location+vec3(0,0,-1), vec3(0,-1,0)));

    Use();
    SetFloat("far_plane", far_plane);
    for(int i = 0; i < 6; ++i)
    {
        stringstream shadow_matrices_stream;
        shadow_matrices_stream <<  "shadow_matrices[" << i << "]";
        SetMat4(shadow_matrices_stream.str(), shadow_transforms[i]);
    }
    SetVec3("light_pos", location);
    // RenderScene();
    SetMat4("model", model_mat);
}
