#include "BoxShape.h"

#include "Render/RenderModule.hpp"
#include "Scene.hpp"

using namespace Kong;
using namespace std;
string CBoxShape::box_model_path = "Engine/box/box.obj";

CBoxShape::CBoxShape()
{
    ImportObj(CSceneLoader::ToResourcePath(box_model_path));
}
