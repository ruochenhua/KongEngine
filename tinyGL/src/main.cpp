//tiny openGL project
#include "render.h"
#include "model.h"
#include "message.h"

#include "TapBodyManager.h"
using namespace glm;
using namespace std;
using namespace tinyGL;

int main()
{
	// 构建渲并初始化染类
	CRender* render = new CRender;
	render->Init();

	CModel* test_model = new CModel;
	string model_path = "../../../../resource/diablo3_pose/diablo3_pose.obj";
	string diffuse_tex_path = "../../../../resource/diablo3_pose/diablo3_pose_diffuse.tga";
	test_model->ImportObj(model_path, diffuse_tex_path);

	string shader_path[] = {
		"../../../../resource/shader/vertex.shader", "../../../../resource/shader/fragment.shader"
	};

	render->AddModel(test_model, shader_path);

	auto body_manager = new Tap::CBodyManager();

	while (render->Update())
	{
	}
	return 0;
}
