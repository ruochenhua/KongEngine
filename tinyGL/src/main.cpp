//tiny openGL project
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "render.h"
#include "message.h"
#include "Engine.h"
#include "Scene.h"

#include "utilityshape.h"
#include "imgui.h"
using namespace glm;
using namespace std;
using namespace tinyGL;

int main()
{
	// Open a window and create its OpenGL context
	// (In the accompanying source code, this variable is global for simplicity)
	// 构建渲并初始化染类
	CRender* render = new CRender;
	render->Init();
	
	vector<shared_ptr<CRenderObj>> render_objs;
	vector<shared_ptr<Light>> lights;
	CSceneLoader::LoadScene("scene/hello_assimp.json", render_objs, lights);
	render->InitLights(lights);
	
	// auto body_manager = new Tap::CBodyManager();
	auto render_window = Engine::GetRenderWindow();
	double current_time, new_time;
	current_time = new_time = glfwGetTime();
	while (!glfwWindowShouldClose(render_window))
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow(); // Show demo window! :)
		// Rendering
		// render your GUI
		ImGui::Begin("Demo window");
		ImGui::Button("Hello!");
		ImGui::End();
		
		if(glfwGetKey(render_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(render_window, true);
		}
		render->Update(new_time - current_time);
		for(auto& render_obj : render_objs)
		{
			render->RenderSceneObject(render_obj);			
		}

		// (Your code clears your framebuffer, renders your other stuff etc.)
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		// (Your code calls glfwSwapBuffers() etc.)
		
		render->PostUpdate();
		current_time = new_time;
		new_time = glfwGetTime();



	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	return 0;
}
