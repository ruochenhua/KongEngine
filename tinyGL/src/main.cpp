//tiny openGL project
#include "render.h"
#include "model.h"
#include "message.h"
#include "Engine.h"

#include "TapBodyManager.h"
#include "utilityshape.h"
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
	
	string model_path = RESOURCE_PATH + "diablo3_pose/diablo3_pose.obj";
	string diffuse_tex_path = RESOURCE_PATH + "diablo3_pose/diablo3_pose_diffuse.tga";
	vector<string> shader_path = {
		RESOURCE_PATH + "shader/vertex.shader", RESOURCE_PATH + "shader/fragment.shader"
	};
	CModel* test_model = new CModel(model_path, diffuse_tex_path, shader_path);
	
	
	// render->AddModel(test_model, shader_path);
	render->AddRenderInfo(test_model->GetRenderInfo());
	vector <string> box_shader[] = {
		RESOURCE_PATH + "shader/utilitybox.vert", RESOURCE_PATH + "shader/utilitybox.frag"
	};

	CUtilityBox* test_box = new CUtilityBox(box_shader);
	
	render->AddRenderInfo(test_box->GetRenderInfo());
	
	auto body_manager = new Tap::CBodyManager();
	auto render_window = Engine::GetRenderWindow();
	float current_time, new_time;
	current_time = new_time = glfwGetTime();
	while (!glfwWindowShouldClose(render_window))
	{
		if(glfwGetKey(render_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(render_window, true);
		}
		render->Update(new_time - current_time);
		current_time = new_time;
		new_time = glfwGetTime();	
	}
	return 0;
}
