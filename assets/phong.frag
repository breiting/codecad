#version 330 core

in vec3 v_normal;
in vec3 v_worldPos;

out vec4 fragColor;

uniform vec3 u_baseColor; // 0..1
uniform vec3 u_camPos;
uniform vec3 u_lightDir;  // Headlight: from camera view direction

void main() {
  vec3 N = normalize(v_normal);
  vec3 L = normalize(-u_lightDir);

  // Ambient
  vec3 ambient = 0.2 * u_baseColor;

  // Diffuse
  float ndotl = max(dot(N, L), 0.0);
  vec3 diffuse = ndotl * u_baseColor;

  // Specular (Blinn-Phong)
  vec3 V = normalize(u_camPos - v_worldPos);
  vec3 H = normalize(L + V);
  float nh = max(dot(N, H), 0.0);
  float spec = pow(nh, 32.0);
  vec3 specular = 0.2 * spec * vec3(1.0);

  fragColor = vec4(ambient + diffuse + specular, 1.0);
}
