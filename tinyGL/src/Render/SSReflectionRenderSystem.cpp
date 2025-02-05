#include "SSReflectionRenderSystem.hpp"

#include "RenderModule.hpp"
#include "Shader/DeferInfoShader.h"

using namespace Kong;

SSReflectionRenderSystem::SSReflectionRenderSystem()
{
    m_Type = RenderSystemType::SS_REFLECTION;
}

void SSReflectionRenderSystem::Init()
{
    m_quadShape = make_shared<CQuadShape>();
    m_ssReflectionShader = std::make_shared<SSReflectionShader>();
}

RenderResultInfo SSReflectionRenderSystem::Draw(double delta, const RenderResultInfo& render_result_info,
    KongRenderModule* render_module)
{
    // scene color：post_process.GetScreenTexture()
    // scene normal：defer_buffer_.g_normal_
    // scene reflection mask: defer_buffer_.g_orm_
    // scene position: defer_buffer_.g_position_
    // scene depth存在于normal贴图的w分量上

    // 这里要关掉深度测试，否则会影响后面的水体渲染的流程
    glDisable(GL_DEPTH_TEST);
	
    m_ssReflectionShader->Use();
	auto defer_render_system = dynamic_cast<DeferRenderSystem*>(render_module->GetRenderSystemByType(RenderSystemType::DEFERRED));
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, defer_render_system->GetPositionTexture());
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, defer_render_system->GetNormalTexture());
    glActiveTexture(GL_TEXTURE0 + 2);
    // 用给后处理的texture作为scene color
    glBindTexture(GL_TEXTURE_2D, render_result_info.resultColor);
    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D, defer_render_system->GetOrmTexture());
	
    m_quadShape->Draw();
    glEnable(GL_DEPTH_TEST);
    
    return render_result_info;
}

void SSReflectionRenderSystem::DrawUI()
{
    
}
