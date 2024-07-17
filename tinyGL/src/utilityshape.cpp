#include "utilityshape.h"

using namespace tinyGL;
std::vector<float> CUtilityBox::s_vBoxVertices = {	
	 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 

	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	
	0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	
	 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

CUtilityBox::CUtilityBox()
{
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
	std::vector<float> vertices = GetVertices();

	glGenVertexArrays(1, &m_RenderInfo.vertexArrayId);
	glBindVertexArray(m_RenderInfo.vertexArrayId);

	//init vertex buffer
	glGenBuffers(1, &m_RenderInfo.vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_RenderInfo.vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), &vertices[0], GL_STATIC_DRAW);

	//vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER,  m_RenderInfo.vertexBuffer);
	glVertexAttribPointer(
		0,                  // attribute 0.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		6*sizeof(float),	// stride
		(void*)0            // array buffer offset
	);
	glEnableVertexAttribArray(0);
	
	glVertexAttribPointer(
		1,                  // attribute 1. 
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized
		6*sizeof(float),                  // stride
		(void*)(3*sizeof(float))            // array buffer offset
	);
	glEnableVertexAttribArray(1);
	
	glBindVertexArray(GL_NONE);
	
	m_RenderInfo._vertex_size = vertices.size();
	m_RenderInfo._stride_count = 6;
}
