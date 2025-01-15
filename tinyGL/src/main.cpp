//tiny openGL project
#include <chrono>
#include <iostream>

#include "render.h"
#include "Engine.h"
#include "Scene.h"

#include "ui.h"
using namespace glm;
using namespace std;
using namespace Kong;
constexpr double FRAME_TIME_CAP = 1.0/200.0;

#define PROFILE_SCENE 0

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
			
			auto blit_start = std::chrono::high_resolution_clock::now();

			ui_manager->PreRenderUpdate(delta);
#if PROFILE_SCENE
			auto blit_end = std::chrono::high_resolution_clock::now();
			auto blit_duration = std::chrono::duration_cast<std::chrono::milliseconds>(blit_end - blit_start);
			blit_start = blit_end;
			std::cout << "ui manager PreRenderUpdate 代码执行时间: " << blit_duration.count() << " 毫秒" << std::endl;
#endif
			
			CScene::GetScene()->PreRenderUpdate(delta);
#if PROFILE_SCENE
			blit_end = std::chrono::high_resolution_clock::now();
			blit_duration = std::chrono::duration_cast<std::chrono::milliseconds>(blit_end - blit_start);
			blit_start = blit_end;
			std::cout << "scene PreRenderUpdate 代码执行时间: " << blit_duration.count() << " 毫秒" << std::endl;
#endif
			
			render->Update(delta);
#if PROFILE_SCENE
			blit_end = std::chrono::high_resolution_clock::now();
			blit_duration = std::chrono::duration_cast<std::chrono::milliseconds>(blit_end - blit_start);
			blit_start = blit_end;
			std::cout << "render update 代码执行时间: " << blit_duration.count() << " 毫秒" << std::endl;
#endif
			
			ui_manager->PostRenderUpdate();
#if PROFILE_SCENE
			blit_end = std::chrono::high_resolution_clock::now();
			blit_duration = std::chrono::duration_cast<std::chrono::milliseconds>(blit_end - blit_start);
			blit_start = blit_end;
			std::cout << "ui PostRenderUpdate 代码执行时间: " << blit_duration.count() << " 毫秒" << std::endl;
#endif
			
			render->PostUpdate();
#if PROFILE_SCENE
			blit_end = std::chrono::high_resolution_clock::now();
			blit_duration = std::chrono::duration_cast<std::chrono::milliseconds>(blit_end - blit_start);
			blit_start = blit_end;
			std::cout << "render PostUpdate 代码执行时间: " << blit_duration.count() << " 毫秒\n====================" << std::endl;
#endif
			
			current_time = new_time;
		}

		
		new_time = glfwGetTime();
	}

	ui_manager->Destroy();
	
	return 0;
}
