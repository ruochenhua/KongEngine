#include "ModelMeshComponent.h"
#include "Render/RenderModule.hpp"

using namespace glm;
using namespace Kong;

CModelMeshComponent::CModelMeshComponent(const string& model_path)
{
	ImportObj(model_path);
}

