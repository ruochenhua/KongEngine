//tiny openGL project
#include "render.h"
#include "Engine.h"
#include "Scene.h"

#include "ui.h"
using namespace glm;
using namespace std;
using namespace Kong;
constexpr double FRAME_TIME_CAP = 1.0/200.0;
int main()
{
	// Open a window and create its OpenGL context
	// (In the accompanying source code, this variable is global for simplicity)
	// 构建渲并初始化染类
	auto render = CRender::GetRender();
	render->Init();
	auto ui_manager = CUIManager::GetUIManager();
	ui_manager->Init();
	
	// auto body_manager = new Tap::CBodyManager();
	auto render_window = Engine::GetRenderWindow();
	double current_time = glfwGetTime();
	double new_time = current_time;
	while (!glfwWindowShouldClose(render_window))
	{
		if(glfwGetKey(render_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(render_window, true);
		}

		double delta = new_time - current_time;
		if(delta > FRAME_TIME_CAP)
		{
			ui_manager->PreRenderUpdate(delta);
			render->Update(delta);

			CScene::GetScene()->PreRenderUpdate(delta);
		
			ui_manager->PostRenderUpdate();
			render->PostUpdate();
			current_time = new_time;
		}
		new_time = glfwGetTime();
	}

	ui_manager->Destroy();
	
	return 0;
}
