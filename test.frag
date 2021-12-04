#version 330 core

in vec3 fragPos;
in vec3 fragColor;
in vec3 n;

uniform vec3 light;
//vec3 light = vec3(5, 10, 0);

out vec4 color;

in vec2 tc;
uniform sampler2D sampler;

void main() {
	vec3 tex = texture(sampler, tc).xyz;
	vec3 lightDir = normalize(light - fragPos);
    vec3 normal = normalize(n);
    float diff = max(dot(lightDir, normal), 0.0);
	vec3 ambiant = 0.12 * tex;
	//color = vec4(diff * fragColor, 1.0);
	color = vec4(diff * tex + ambiant, 1.0);
}
