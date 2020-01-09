#include "stdafx.h"
#include "CollisionHelper.h"
#include "CollisionSolver.h"

namespace Tap
{
	CBoxCollisionHelper::CBoxCollisionHelper(CBoxShape *s0, const CTransform& t0,
		CBoxShape *s1, const CTransform& t1)
		: m_s0(s0), m_t0(t0)
		, m_s1(s1), m_t1(t1)
	{
	}

	bool CBoxCollisionHelper::IsCollide(SCollisionManifold& manifold)
	{
		glm::quat q0 = m_t0.GetRotation();
		glm::quat q1 = m_t1.GetRotation();

		//基础的六个轴,每个box三个
		glm::vec3 axis_0_x = q0 * glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec3 axis_0_y = q0 * glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 axis_0_z = q0 * glm::vec3(0.0f, 0.0f, 1.0f);

		glm::vec3 axis_1_x = q1 * glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec3 axis_1_y = q1 * glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 axis_1_z = q1 * glm::vec3(0.0f, 0.0f, 1.0f);
		
		glm::vec3 axis_array[] = {
			axis_0_x,axis_0_y,axis_0_z,
			axis_1_x,axis_1_y,axis_1_z,
			//需要检查edge case，是两个box的法线轴之间相互的差乘，共九个轴
			//目前为了简化,其实应该需要的时候再计算的
			glm::cross(axis_0_x, axis_1_x),
			glm::cross(axis_0_x, axis_1_y),
			glm::cross(axis_0_x, axis_1_z),

			glm::cross(axis_0_y, axis_1_x),
			glm::cross(axis_0_y, axis_1_y),
			glm::cross(axis_0_y, axis_1_z),

			glm::cross(axis_0_z, axis_1_x),
			glm::cross(axis_0_z, axis_1_y),
			glm::cross(axis_0_z, axis_1_z)
		};	

		float min_lap = 10000.0f;
		int min_idx = -1;
		for (int i = 0; i < 15; ++i)
		{
			glm::vec3 a = axis_array[i];

			//检查axis是不是用两条几乎平行的轴生成的,其实应该用lengthsquare的,没找到?
			if(glm::length(a) < 0.001) 
				continue;

			a = glm::normalize(a);
			float overlap = PenetrationOnAxis(a);
			if (overlap < 0)
			{				
				return false;
			}

			if (overlap < min_lap)
			{
				min_lap = overlap;
				min_idx = i;
			}
		}


		manifold._hit_normal = axis_array[min_idx];		

		manifold._hit_depth = min_lap;

		//寻找碰撞点
		//box的轴为最优解,一般也就是point to face
		if (min_idx < 6)
		{
			//t1 -> t0
			glm::vec3 select_axis = axis_array[min_idx];			

			// box 0 axes
			// 找到box 1的最近顶点
			if (min_idx < 3)
			{			
				glm::vec3 half_extent = m_t0.GetRotation() * m_s1->GetHalfExtent();
				if (glm::dot(half_extent * glm::vec3(1.0f, 0.0f, 0.0f), select_axis) < 0)
				{
					half_extent.x *= -1;
				}
				if (glm::dot(half_extent * glm::vec3(0.0f, 1.0f, 0.0f), select_axis) < 0)
				{
					half_extent.y *= -1;
				}
				if (glm::dot(half_extent * glm::vec3(0.0f, 0.0f, 1.0f), select_axis) < 0)
				{
					half_extent.z *= -1;
				}
				glm::vec3 point = m_t1.GetPosition() + half_extent;
				manifold._hit_pos = point;
			}
			// box 1 axes
			else
			{
				glm::vec3 half_extent = m_t0.GetRotation() * m_s0->GetHalfExtent();
				if (glm::dot(half_extent * glm::vec3(1.0f, 0.0f, 0.0f), select_axis) < 0)
				{
					half_extent.x *= -1;
				}
				if (glm::dot(half_extent * glm::vec3(0.0f, 1.0f, 0.0f), select_axis) < 0)
				{
					half_extent.y *= -1;
				}
				if (glm::dot(half_extent * glm::vec3(0.0f, 0.0f, 1.0f), select_axis) < 0)
				{
					half_extent.z *= -1;
				}
				glm::vec3 point = m_t0.GetPosition() + half_extent;
				manifold._hit_pos = point;
			}
			
			return true;
		}
		//edge to edge
		else
		{
			glm::vec3 half_0 = m_t0.Transform(m_s0->GetHalfExtent());
			glm::vec3 half_1 = m_t1.Transform(m_s1->GetHalfExtent());

			int idx = min_idx - 6;
			int idx_0 = idx / 3;	//按照排列,可以判断最佳的轴box的哪两个的轴的叉乘
			int idx_1 = idx % 3;

			glm::vec3 axis_vec[3] = { glm::vec3(1,0,0), glm::vec3(0,1,0),  glm::vec3(0,0,1) };
			//计算出两个轴的最短距离的向量,作为penetration depth
			for (int i = 0; i < 3; ++i)
			{
				//找出两个轴的中点,配合方向描述实线
				if (i == idx_0)
					half_0[i] = 0;
				else if (glm::dot(half_0*axis_vec[i], manifold._hit_normal) > 0)
				{
					half_0[i] *= -1;
				}

				if (i == idx_1)
					half_1[i] = 0;
				else if (glm::dot(half_1*axis_vec[i], manifold._hit_normal) < 0)
				{
					half_1[i] *= -1;
				}

			}

			glm::vec3 dir_0 = axis_array[idx_0];	//direction 0
			glm::vec3 dir_1 = axis_array[idx_1];	//direction 1

			//half_0 is point 0
			//half_1 is point 1
			//可以计算出两条线段的最近点
			glm::vec3 r = half_0 - half_1;
			float a = glm::dot(dir_0, dir_0);
			float b = glm::dot(dir_0, dir_1);
			float c = glm::dot(dir_0, r);			
			float e = glm::dot(dir_1, dir_1);
			float f = glm::dot(dir_1, r);
			float d = a * e - b * b;

			if (d == 0.0f)
				return false;	//两条线平行,不处理这种情况

			float s = (b*f - c * e) / d;
			float t = (a*f - b * c) / d;

			glm::vec3 point_0 = half_0 + dir_0 * s;
			glm::vec3 point_1 = half_1 + dir_1 * t;

			manifold._hit_pos = (point_0 + point_1) * 0.5f;

			return true;
		}
			   		 
		return false;
	}

	float CBoxCollisionHelper::PenetrationOnAxis(const glm::vec3& axis)
	{
		//投影两个box
		float proj0 = ProjectToAxis(axis, m_s0, m_t0);
		float proj1 = ProjectToAxis(axis, m_s1, m_t1);

		//把两个box的距离投影到轴上
		glm::vec3 dist = m_t0.GetPosition() - m_t1.GetPosition();
		float dist_abs = glm::abs(glm::dot(dist, axis));

		return proj0 + proj1 - dist_abs;
	}

	float CBoxCollisionHelper::ProjectToAxis(const glm::vec3& axis, CBoxShape* shape, const CTransform& trans)
	{
		//其实就是检查box的八个点在axis上的最大和最小,返回两个的差的一半
		glm::vec3 half_extent_world = shape->GetHalfExtent();
		
		glm::vec3 box_vertices[8] = {
			half_extent_world,
			half_extent_world * glm::vec3(-1.0f, 1.0f, 1.0f),
			half_extent_world * glm::vec3(-1.0f, -1.0f, 1.0f),
			half_extent_world * glm::vec3(-1.0f, -1.0f, -1.0f),
			half_extent_world * glm::vec3(1.0f, -1.0f, 1.0f),
			half_extent_world * glm::vec3(1.0f, -1.0f, -1.0f),
			half_extent_world * glm::vec3(1.0f, 1.0f, -1.0f),
			half_extent_world * glm::vec3(-1.0f, 1.0f, -1.0f)
		};

		for (int i = 0; i < 8; ++i)
		{
			box_vertices[i] = trans.Transform(box_vertices[i]);
		}

		float max_val = -99999.0f;
		float min_val = 99999.0f;

		for (int i = 0; i < 8; ++i)
		{
			float proj_val = glm::dot(box_vertices[i], axis);
			if (proj_val > max_val)
				max_val = proj_val;
			if (proj_val < min_val)
				min_val = proj_val;
		}
		
		return (max_val - min_val) * 0.5f;
	}
}