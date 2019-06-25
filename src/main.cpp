//tiny openGL project
#include "render.h"
#include "model.h"
using namespace glm;
using namespace std;
int main()
{
	CRender* render = new CRender;
	render->Init();

	CModel* test_model = new CModel;
	test_model->ImportObj("../../../resource/african_head/african_head.obj");

	string shader_path[] = {
		"../../../resource/shader/vertex.shader", "../../../resource/shader/fragment.shader"
	};
	render->AddModel(test_model, shader_path);

	while (1)
	{
		render->Update();
	}
}
