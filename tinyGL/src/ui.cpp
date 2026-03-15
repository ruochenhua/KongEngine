#include "ui.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include "Render/Abstraction/IGraphicsDevice.hpp"
#include "Render/Abstraction/BackendType.hpp"
#ifdef RENDER_IN_VULKAN
#include <imgui_impl_vulkan.h>
#include "Render/GraphicsAPI/Vulkan/VulkanGraphicsDevice.hpp"
#else
#include <imgui_impl_opengl3.h>
#endif
#include "Actor.hpp"
#include "Utils.hpp"
#include "Scene.hpp"
#include "Component/LightComponent.h"
#include <filesystem>

#include "Component/Mesh/Terrain.h"
#include "Render/RenderModule.hpp"

using namespace Kong;

static KongUIManager g_UIManager;
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

KongUIManager& KongUIManager::GetUIManager()
{
	return g_UIManager;	
}

KongUIManager::KongUIManager()
{
	for (float& time : process_time)
	{
		time = 0.0f;
	}
}

void KongUIManager::Init(GLFWwindow* windowHandle, IGraphicsDevice* device)
{
	m_imguiVulkan = (device != nullptr && device->GetBackendType() == BackendType::Vulkan);
	m_imguiDescriptorPool = nullptr;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.AddMouseButtonEvent(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
	ImGui::StyleColorsDark();

	if (m_imguiVulkan)
	{
#ifdef RENDER_IN_VULKAN
		auto vulkanDevice = VulkanGraphicsDevice::GetGraphicsDevice();

		VkDescriptorPoolSize pool_sizes[] =
		{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = std::size(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		VkDescriptorPool imguiPool = VK_NULL_HANDLE;
		if(vkCreateDescriptorPool(vulkanDevice->GetDevice(), &pool_info, nullptr, &imguiPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create imgui descriptor pool");
		}
		m_imguiDescriptorPool = static_cast<void*>(imguiPool);

		ImGui_ImplGlfw_InitForVulkan(windowHandle, true);
		ImGui_ImplVulkan_InitInfo init_info{};
		init_info.Instance = vulkanDevice->m_instance;
		init_info.Device = vulkanDevice->m_device;
		init_info.PhysicalDevice = vulkanDevice->m_physicalDevice;
		init_info.QueueFamily = vulkanDevice->FindPhysicsQueueFamilies().graphicsFamily;
		init_info.Queue = vulkanDevice->m_graphicsQueue;
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.MinImageCount = 2;
		init_info.ImageCount = 2;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.RenderPass = VulkanGraphicsDevice::GetGraphicsDevice()->GetSwapChain()->GetRenderPass();
		init_info.Subpass = 0;
		init_info.DescriptorPool = imguiPool;

		ImGui_ImplVulkan_Init(&init_info);
#endif
	}
	else
	{
#ifndef RENDER_IN_VULKAN
		ImGui_ImplGlfw_InitForOpenGL(windowHandle, true);
		ImGui_ImplOpenGL3_Init();
#endif
	}
	
	string file_directory = std::filesystem::current_path().parent_path().string() + "/resource/scene";
	g_scene_files = GetSceneFiles(file_directory);
	
	for (size_t i = 0; i < g_scene_files.size(); ++i) {
		g_scene_items.push_back(g_scene_files[i].c_str());
	}

	for (int i = 0; i < TIME_RECORD_COUNT; ++i)
	{
		process_time[i] = 0;
	}
	
}

void KongUIManager::PreRenderUpdate(double delta)
{
	if (m_imguiVulkan)
	{
#ifdef RENDER_IN_VULKAN
		ImGui_ImplVulkan_NewFrame();
#endif
	}
	else
	{
#ifndef RENDER_IN_VULKAN
		ImGui_ImplOpenGL3_NewFrame();
#endif
	}
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	DescribeUIContent(delta);
	
}

void KongUIManager::PostRenderUpdate()
{
	if (m_imguiVulkan)
	{
#ifdef RENDER_IN_VULKAN
		ImGui::EndFrame();
#endif
	}
	else
	{
#ifndef RENDER_IN_VULKAN
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
	}
}

void KongUIManager::Destroy()
{
	if (m_imguiVulkan)
	{
#ifdef RENDER_IN_VULKAN
		ImGui_ImplVulkan_Shutdown();
		if (m_imguiDescriptorPool)
		{
			vkDestroyDescriptorPool(VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice(),
				static_cast<VkDescriptorPool>(m_imguiDescriptorPool), nullptr);
			m_imguiDescriptorPool = nullptr;
		}
#endif
	}
	else
	{
#ifndef RENDER_IN_VULKAN
		ImGui_ImplOpenGL3_Shutdown();
#endif
	}
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void KongUIManager::DescribeUIContent(double delta)
{
	// ImGui::ShowDemoWindow(); // Show demo window! :)
	// Rendering
	// render your GUI
	ImGui::Begin("Main");
	int frame_rate = static_cast<int>(round(1.0/delta));
	ImVec4 frame_rate_color = GetFrameRateColor(frame_rate);
	ImGui::TextColored(frame_rate_color, "frame_rate: %d", frame_rate);

//////////////////
	float average = 0.0f;
	for (int n = 0; n < TIME_RECORD_COUNT; n++)
		average += process_time[n];
	average /= (float)(TIME_RECORD_COUNT);
	char overlay[32];
	sprintf(overlay, "avg %f", average);
	
	process_time[process_time_offset] = delta;
	ImGui::PlotLines("Lines", process_time, IM_ARRAYSIZE(process_time), process_time_offset
		, overlay, 0.0f, 0.1f, ImVec2(0, 100));
	
	process_time_offset = (process_time_offset + 1) % TIME_RECORD_COUNT;

////////////////
	
	static int item_type = g_scene_items.size() - 1;
	
	ImGui::Combo("Scenes", &item_type, g_scene_items.data(), g_scene_items.size(), g_scene_items.size());
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);

	if(ImGui::Button("load scene"))
	{
		// Load scene
		string scene_name = g_scene_items[item_type];
		scene_name = "scene/" + scene_name;
		
		KongSceneManager::GetSceneManager().LoadScene(scene_name);
	}

	if(ImGui::TreeNode("scene"))
	{
		auto actors = KongSceneManager::GetActors();
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
						ImGui::DragFloat("height shift", &terrain->height_shift_, 0.1f, -100.0, 100.0);
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

ImVec4 KongUIManager::GetFrameRateColor(int framerate)
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