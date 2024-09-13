#version 450 compatibility
layout(triangles, invocations = 6) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 light_space_matrix[16];


void main()
{
	for (int i = 0; i < 3; ++i)
	{
		gl_Position =
		light_space_matrix[gl_InvocationID] * gl_in[i].gl_Position;
		gl_Layer = gl_InvocationID;
		EmitVertex();
	}
	EndPrimitive();
} 

