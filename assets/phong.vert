#version 330 core

layout(location=0) in vec3 inPosition;
layout(location=1) in vec3 inNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
uniform mat3 uNormalMatrix;

out vec3 vNormal;
out vec3 vWorldPos;

void main() {
  vec4 wp = uModel * vec4(inPosition, 1.0);
  vWorldPos = wp.xyz;
  vNormal = normalize(uNormalMatrix * inNormal);
  gl_Position = uProj * uView * wp;
}
