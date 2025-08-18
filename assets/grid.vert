#version 330 core

// inspired by
// https://github.com/emeiri/ogldev/blob/master/Common/Shaders/infinite_grid.vs

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform vec3 u_CameraPosition;

out vec3 v_WorldPos;

const vec3 Pos[4] = vec3[4](
	vec3(-1000.0, 0.0, -1000.0), // bottom left
	vec3(1000.0, 0.0, -1000.0),  // bottom right
	vec3(1000.0, 0.0, 1000.0),   // top right
	vec3(-1000.0, 0.0, 1000.0)   // top left
);

const int Indices[6] = int[6](0, 2, 1, 2, 0, 3);

void main()
{
	int Index = Indices[gl_VertexID];

	vec3 vPos3 = Pos[Index];

	vPos3.x += u_CameraPosition.x;
	vPos3.z += u_CameraPosition.z;

	vec4 vPos4 = vec4(vPos3, 1.0);
	gl_Position = u_Projection * u_View * vPos4;

	v_WorldPos = vPos3;
}

