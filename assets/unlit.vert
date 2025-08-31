#version 330 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inCol;

uniform mat4 uMVP;
out vec3 vCol;

void main() {
  vCol = inCol;
  gl_Position = uMVP * vec4(inPos, 1.0);
}
