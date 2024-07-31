//tiny openGL project
#include "render.h"
#include "message.h"
#include "Engine.h"
#include "Scene.h"

#include "utilityshape.h"
#include "ui.h"
using namespace glm;
using namespace std;
using namespace tinyGL;

int main()
{
	// Open a window and create its OpenGL context
	// (In the accompanying source code, this variable is global for simplicity)
	// 构建渲并初始化染类
	auto render = CRender::GetRender();
	render->Init();
	auto ui_manager = CUIManager::GetUIManager();
	ui_manager->Init();

	// auto scene = CScene::GetScene();
	//
	// vector<shared_ptr<CRenderObj>> render_objs;
	// vector<shared_ptr<Light>> lights;
	// CSceneLoader::LoadScene("scene/hello_assimp.json", render_objs, lights);
	
	// auto body_manager = new Tap::CBodyManager();
	auto render_window = Engine::GetRenderWindow();
	double current_time, new_time;
	current_time = new_time = glfwGetTime();
	while (!glfwWindowShouldClose(render_window))
	{
		if(glfwGetKey(render_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(render_window, true);
		}

		double delta = new_time - current_time;
		ui_manager->PreRenderUpdate(delta);
		render->Update(delta);

		render->RenderShadowMap();			
		render->RenderSceneObject();			
		
		ui_manager->PostRenderUpdate();
		render->PostUpdate();
		current_time = new_time;
		new_time = glfwGetTime();
	}

	ui_manager->Destroy();
	
	return 0;
}
