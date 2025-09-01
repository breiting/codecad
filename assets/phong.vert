#version 330 core

layout(location=0) in vec3 in_position;
layout(location=1) in vec3 in_normal;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;
uniform mat3 u_normalMatrix;

out vec3 v_normal;
out vec3 v_worldPos;

void main() {
  vec4 wp = u_model * vec4(in_position, 1.0);
  v_worldPos = wp.xyz;
  v_normal = normalize(u_normalMatrix * in_normal);
  gl_Position = u_proj * u_view * wp;
}
