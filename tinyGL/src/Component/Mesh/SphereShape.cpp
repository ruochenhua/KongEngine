#include "SphereShape.h"

#include "Scene.hpp"

using namespace Kong;

string SphereShape::sphere_model_path = "Engine/sphere/sphere.obj";

SphereShape::SphereShape()
{
    ImportObj(CSceneLoader::ToResourcePath(sphere_model_path));
}
