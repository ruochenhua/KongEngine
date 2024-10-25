#include "ui.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Actor.h"
#include "Engine.h"
#include "render.h"
#include "Scene.h"
#include "Component/LightComponent.h"
#include <filesystem>

using namespace Kong;

CUIManager* g_uimanager = new CUIManager;
vector<string> g_scene_files;
vector<const char*> g_scene_items;

vector<string> GetSceneFiles(const string& directory_path)
{
	vector<string> scene_files;
    
	for(const auto& entry : std::filesystem::directory_iterator(directory_path))
	{
		if(entry.is_regular_file())
		{
			scene_files.push_back(entry.path().filename().string());
		}
	}
	
	return scene_files;
}

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

	
	string file_directory = std::filesystem::current_path().parent_path().string() + "/resource/scene";
	g_scene_files = GetSceneFiles(file_directory);
	
	for (size_t i = 0; i < g_scene_files.size(); ++i) {
		g_scene_items.push_back(g_scene_files[i].c_str());
	}
}

void CUIManager::PreRenderUpdate(double delta)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	
	DescribeUIContent(delta);
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

void CUIManager::DescribeUIContent(double delta)
{
	//ImGui::ShowDemoWindow(); // Show demo window! :)

	// Rendering
	// render your GUI
	ImGui::Begin("Main");
	int frame_rate = static_cast<int>(round(1.0/delta));
	ImVec4 frame_rate_color = GetFrameRateColor(frame_rate);
	ImGui::TextColored(frame_rate_color, "frame_rate: %d", frame_rate);
	
	static int item_type = g_scene_items.size() - 1;
	
	ImGui::Combo("Scenes", &item_type, g_scene_items.data(), g_scene_items.size(), g_scene_items.size());
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);

	auto render_sys = CRender::GetRender();
	if(ImGui::Button("load scene"))
	{
		// Load scene
		string scene_name = g_scene_items[item_type];
		scene_name = "scene/" + scene_name;
		
		CScene::GetScene()->LoadScene(scene_name);
	}
	if(ImGui::TreeNode("skybox"))
	{
		if(ImGui::Button("change hdr background"))
		{
			CRender::GetRender()->ChangeSkybox();
		}

		ImGui::DragInt("sky display status", &render_sys->render_sky_env_status, 0.1f, 0, 2);
		
		ImGui::TreePop();
	}
	
	auto main_cam = render_sys->GetCamera();
	if(main_cam)
	{
		ImGui::DragFloat("cam exposure", &main_cam->exposure, 0.02f,0.01f, 10.0f);
		ImGui::DragFloat("cam speed", &main_cam->move_speed, 0.2f,1.0f, 100.0f);
	}

	ImGui::Checkbox("ssao", &render_sys->use_ssao);
	ImGui::Checkbox("screen space reflection", &render_sys->use_screen_space_reflection);
	ImGui::Checkbox("render cloud", &render_sys->m_SkyBox.render_cloud);
	
	if(ImGui::TreeNode("scene"))
	{
		auto actors = CScene::GetActors();
		unsigned actor_count = actors.size();
		for(auto actor : actors)
		{
			ImGui::PushID(actor->name.c_str());
			if(ImGui::TreeNode("","%s", actor->name.c_str()))
			{
				if(ImGui::TreeNode("", "transform:"))
				{
					if(ImGui::TreeNode("","location:"))
					{
						ImGui::DragFloat("lx", &actor->location.x, 0.2f);
						ImGui::DragFloat("ly", &actor->location.y, 0.2f);
						ImGui::DragFloat("lz", &actor->location.z, 0.2f);
						ImGui::TreePop();
					}
					if(ImGui::TreeNode("","rotation:"))
					{
						ImGui::DragFloat("rx", &actor->rotation.x, 0.2f);
						ImGui::DragFloat("ry", &actor->rotation.y, 0.2f);
						ImGui::DragFloat("rz", &actor->rotation.z, 0.2f);
						ImGui::TreePop();
					}
					if(ImGui::TreeNode("","scale:"))
					{
						ImGui::DragFloat("sx", &actor->scale.x, 0.02f, 0.01f, 100.f);
						ImGui::DragFloat("sy", &actor->scale.y, 0.02f, 0.01f, 100.f);
						ImGui::DragFloat("sz", &actor->scale.z, 0.02f, 0.01f, 100.f);
						ImGui::TreePop();
					}
					ImGui::TreePop();
				}

				auto light_comp = actor->GetComponent<CLightComponent>();
				if(light_comp)
				{
					if(ImGui::TreeNode("", "light:"))
					{
						if(ImGui::TreeNode("","color:"))
						{
							ImGui::DragFloat("color_x", &light_comp->light_color.x, 0.02f, 0.f, 100.f);
							ImGui::DragFloat("color_y", &light_comp->light_color.y, 0.02f, 0.f, 100.f);
							ImGui::DragFloat("color_z", &light_comp->light_color.z, 0.02f, 0.f, 100.f);
							ImGui::TreePop();
						}
						ImGui::TreePop();
					}
				}

				auto terrain = actor->GetComponent<Terrain>();
				if(terrain)
				{
					if(ImGui::TreeNode("", "terrain:"))
					{
						ImGui::DragFloat("amplitude", &terrain->amplitude, 0.02f, 0.f, 5000);
						ImGui::DragFloat("freq", &terrain->freq, 0.0001f, 0.f, 1.0f);
						ImGui::DragFloat("power", &terrain->power, 0.02f, 0.f, 32.f);
						ImGui::DragInt("octaves", &terrain->octaves, 0.02f, 0, 100);
						ImGui::TreePop();
					}
				}
					
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}
	
	ImGui::End();
}

ImVec4 CUIManager::GetFrameRateColor(int framerate)
{
	if(framerate > 120)
	{
		// very very smooth
		return ImVec4(1.f, 1.f, 1.0f, 1.0f);
	}

	if(framerate > 60)
	{
		// very smooth
		return ImVec4(0.f, 1.f, 1.0f, 1.0f);
	}

	if(framerate > 30)
	{
		// quite smooth
		return ImVec4(0.f, 1.f, 0.0f, 1.0f);
	}

	if(framerate > 15)
	{
		// laggy
		return ImVec4(1.f, 1.f, 0.0f, 1.0f);
	}
	
	// terrible
	return ImVec4(1.f, 0.f, 0.0f, 1.0f);
}
