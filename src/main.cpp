//tiny openGL project
#include "render.h"
#include "model.h"
using namespace glm;

int main()
{
	CRender* render = new CRender;
	render->Init();

	CModel* test_model = new CModel;
	test_model->ImportObj("../../../resource/african_head/african_head.obj");

	std::vector<float> render_vertices = test_model->GetVertices();
	render->AddRenderVertices(render_vertices);

	while (1)
	{
		render->Update();
	}
}
