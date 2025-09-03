#version 330 core

in vec3 vNormal;
in vec3 vWorldPos;

out vec4 fragColor;

uniform vec3 uBaseColor; // 0..1
uniform vec3 uCamPos;
uniform vec3 uLightDir;  // Headlight: from camera view direction

void main() {
  vec3 N = normalize(vNormal);
  vec3 L = normalize(-uLightDir);
  vec3 V = normalize(uCamPos - vWorldPos);
  vec3 H = normalize(L + V);

  vec3 ambient = 0.08 * uBaseColor;

  // Half-Lambert / Wrap Diffuse
  float wrap = 0.5;   
  float ndotl = dot(N, L);
  float diff = clamp(ndotl * 0.5 + wrap * 0.5, 0.0, 1.0);
  vec3 diffuse = 0.9 * diff * uBaseColor;

  float nh = max(dot(N, H), 0.0);
  float spec = (diff > 0.0) ? pow(nh, 24.0) : 0.0; 
  vec3 specular = 0.05 * spec * vec3(1.0);       

  vec3 color = ambient + diffuse + specular;

  fragColor = vec4(color, 1.0);
}
