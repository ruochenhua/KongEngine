#include "utilityshape.h"

#include "render.h"
#include "Shader/BRDFShader.h"

using namespace tinyGL;
using namespace std;
vector<float> CUtilityBox::s_vBoxVertices = {	
	// positions          // normals           // texture coords
     0.5f, -0.5f, -0.5f, 
    -0.5f, -0.5f, -0.5f, 
     0.5f,  0.5f, -0.5f, 
    -0.5f,  0.5f, -0.5f, 
     0.5f,  0.5f, -0.5f, 
    -0.5f, -0.5f, -0.5f, 

    -0.5f, -0.5f,  0.5f, 
     0.5f, -0.5f,  0.5f, 
     0.5f,  0.5f,  0.5f, 
     0.5f,  0.5f,  0.5f, 
    -0.5f,  0.5f,  0.5f, 
    -0.5f, -0.5f,  0.5f, 

    -0.5f,  0.5f,  0.5f, 
    -0.5f,  0.5f, -0.5f, 
    -0.5f, -0.5f, -0.5f, 
    -0.5f, -0.5f, -0.5f, 
    -0.5f, -0.5f,  0.5f, 
    -0.5f,  0.5f,  0.5f, 

	0.5f,  0.5f, -0.5f,  
     0.5f,  0.5f,  0.5f, 
     0.5f, -0.5f, -0.5f, 
     0.5f, -0.5f,  0.5f, 
     0.5f, -0.5f, -0.5f, 
     0.5f,  0.5f,  0.5f, 

    -0.5f, -0.5f, -0.5f, 
     0.5f, -0.5f, -0.5f, 
     0.5f, -0.5f,  0.5f, 
     0.5f, -0.5f,  0.5f, 
    -0.5f, -0.5f,  0.5f, 
    -0.5f, -0.5f, -0.5f, 

	0.5f,  0.5f, -0.5f,  
    -0.5f,  0.5f, -0.5f, 
     0.5f,  0.5f,  0.5f, 
    -0.5f,  0.5f,  0.5f, 
     0.5f,  0.5f,  0.5f, 
    -0.5f,  0.5f, -0.5f
};

vector<float> CUtilityBox::s_vBoxNormals = {	
	// positions          // normals           // texture coords
     0.0f,  0.0f, -1.0f,
     0.0f,  0.0f, -1.0f,
     0.0f,  0.0f, -1.0f,
     0.0f,  0.0f, -1.0f,
     0.0f,  0.0f, -1.0f,
     0.0f,  0.0f, -1.0f,

    0.0f,  0.0f,  1.0f,
     0.0f,  0.0f,  1.0f,
     0.0f,  0.0f,  1.0f,
     0.0f,  0.0f,  1.0f,
     0.0f,  0.0f,  1.0f,
    0.0f,  0.0f,  1.0f,

     -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,

	1.0f,  0.0f,  0.0f, 
     1.0f,  0.0f,  0.0f,
      1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,

    0.0f, -1.0f,  0.0f,
     0.0f, -1.0f,  0.0f,
     0.0f, -1.0f,  0.0f,
     0.0f, -1.0f,  0.0f,
    0.0f, -1.0f,  0.0f,
    0.0f, -1.0f,  0.0f,

	0.0f,  1.0f,  0.0f, 
    0.0f,  1.0f,  0.0f,
     0.0f,  1.0f,  0.0f,
    0.0f,  1.0f,  0.0f,
     0.0f,  1.0f,  0.0f,
    0.0f,  1.0f,  0.0f,
};

vector<float> CUtilityBox::s_vBoxTexCoords = {	
	// positions          // normals           // texture coords
     1.0f,  0.0f,
    0.0f,  0.0f,
     1.0f,  1.0f,
    0.0f,  1.0f,
     1.0f,  1.0f,
    0.0f,  0.0f,

    0.0f,  0.0f,
     1.0f,  0.0f,
     1.0f,  1.0f,
     1.0f,  1.0f,
    0.0f,  1.0f,
    0.0f,  0.0f,

    1.0f,  0.0f,
    1.0f,  1.0f,
    0.0f,  1.0f,
    0.0f,  1.0f,
    0.0f,  0.0f,
    1.0f,  0.0f,

	1.0f,  1.0f,
     1.0f,  0.0f,
     0.0f,  1.0f,
     0.0f,  0.0f,
     0.0f,  1.0f,
     1.0f,  0.0f,

    0.0f,  1.0f,
     1.0f,  1.0f,
     1.0f,  0.0f,
     1.0f,  0.0f,
    0.0f,  0.0f,
    0.0f,  1.0f,

	1.0f,  1.0f,
     0.0f,  1.0f,
     1.0f,  0.0f,
    0.0f,  0.0f,
     1.0f,  0.0f,
    0.0f,  1.0f
};

CUtilityBox::CUtilityBox(const SRenderResourceDesc& render_resource_desc)
	: CMeshComponent(render_resource_desc)
{
	shader_data.reset();
	shader_data = make_shared<BRDFShader>();
	
	box_render_resource_desc = render_resource_desc;
	CMesh mesh;
	mesh.m_Vertex = s_vBoxVertices;
	mesh.m_Normal = s_vBoxNormals;
	mesh.m_TexCoord = s_vBoxTexCoords;
	mesh_list.push_back(mesh);
	
	GenerateRenderInfo();
}


std::vector<float> CUtilityBox::GetVertices() const
{
	return s_vBoxVertices;
}

std::vector<unsigned int> CUtilityBox::GetIndices() const
{
	return std::vector<unsigned int>();
}

void CUtilityBox::GenerateRenderInfo()
{
	auto& render_info = mesh_list[0].m_RenderInfo;
	// // load texture map
	const auto& texture_paths = box_render_resource_desc.texture_paths;
	auto diffuse_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::diffuse);
	if(diffuse_path_iter != texture_paths.end())
	{
		render_info.diffuse_tex_id = CRender::LoadTexture(diffuse_path_iter->second);
	}
	
	auto specular_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::specular);
	if(specular_path_iter != texture_paths.end())
	{
		render_info.specular_tex_id = CRender::LoadTexture(specular_path_iter->second);
	}
	
	auto normal_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::normal);
	if(normal_path_iter != texture_paths.end())
	{
		render_info.normal_tex_id = CRender::LoadTexture(normal_path_iter->second);
	}
	
	auto tangent_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::tangent);
	if(tangent_path_iter != texture_paths.end())
	{
		render_info.tangent_tex_id = CRender::LoadTexture(tangent_path_iter->second);
	}
	
	render_info.material = box_render_resource_desc.material;
	
}
