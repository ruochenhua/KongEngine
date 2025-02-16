#pragma once
#include "MeshComponent.h"

namespace Kong
{
	class CBoxShape : public CMeshComponent
	{
	public:
		CBoxShape();
	
	private:
		
		static std::string box_model_path;
	};
}