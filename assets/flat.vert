#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Color;
layout(location = 3) in vec2 a_TexCoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform vec3 u_LightDir; // in world space

out vec3 v_Normal;
out vec3 v_Color;
out vec3 v_LightVector;
out vec2 v_TexCoord;

void main()
{
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    gl_Position = u_Projection * u_View * worldPos;

    // Normals in world space
    vec3 normalWorld = normalize(mat3(transpose(inverse(u_Model))) * a_Normal);
    // Light vector in world space
    vec3 lightDirWorld = normalize(-u_LightDir); // direction to light

    v_Normal = normalWorld;
    v_LightVector = lightDirWorld;
    v_Color = a_Color;
	v_TexCoord = a_TexCoord;
}
