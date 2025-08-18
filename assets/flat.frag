#version 330 core

in vec3 v_Normal;
in vec3 v_Color;
in vec3 v_LightVector;
in vec2 v_TexCoord;

uniform sampler2D u_Texture;
uniform vec3 u_LightColor;
uniform vec3 u_MaterialColor;
uniform bool u_HasTexture;

out vec4 fragColor;

void main()
{
    vec3 normal = normalize(v_Normal);
    vec3 lightVec = normalize(v_LightVector);
    float diff = max(dot(normal, lightVec), 0.0);

    vec3 baseColor = (length(v_Color) > 0.01) ? v_Color : u_MaterialColor;
    vec3 litColor = baseColor * u_LightColor * diff;

    // Optional: Gamma correction
    // litColor = pow(litColor, vec3(1.0 / 2.2));

	if (u_HasTexture) {
		fragColor = texture(u_Texture, v_TexCoord);
	} else {
		fragColor = vec4(litColor, 1.0);
	}
}

