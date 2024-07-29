#include "ui.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Engine.h"
#include "Scene.h"
using namespace tinyGL;

CUIManager* g_uimanager = new CUIManager;

CUIManager* CUIManager::GetUIManager()
{
	return g_uimanager;	
}

void CUIManager::Init()
{
	// 初始化imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.AddMouseButtonEvent(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
	
	ImGui::StyleColorsDark();

	// 初始化imgui后端
	ImGui_ImplGlfw_InitForOpenGL(Engine::GetRenderWindow(), true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();
}

void CUIManager::PreRenderUpdate()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	
	DescribeUIContent();
}

void CUIManager::PostRenderUpdate()
{
	// (Your code clears your framebuffer, renders your other stuff etc.)
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	// (Your code calls glfwSwapBuffers() etc.)
}

void CUIManager::Destroy()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void CUIManager::DescribeUIContent()
{
	// ImGui::ShowDemoWindow(); // Show demo window! :)

	// Rendering
	// render your GUI
	ImGui::Begin("Demo window");
	// Select an item type
	const char* scene_items[] = {
		"hello",
		"hello_brdf",
		"hello_assimp",
		"hello_normal_map"
	};

	static int item_type = 3;
	ImGui::Combo("Scenes", &item_type, scene_items, IM_ARRAYSIZE(scene_items), IM_ARRAYSIZE(scene_items));
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);

	if(ImGui::Button("load scene"))
	{
		// Load scene
		string scene_name = scene_items[item_type];
		scene_name = "scene/" + scene_name + ".json";
		
		CScene::GetScene()->LoadScene(scene_name);
	}
	
	ImGui::End();
}