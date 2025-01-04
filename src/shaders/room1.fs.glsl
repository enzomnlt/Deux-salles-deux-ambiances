#version 330 core

in vec3 vPosition_vs;
in vec3 vNormal_vs;
in vec2 vTexCoords;

out vec3 fFragColor;

uniform vec3 uKd;
uniform vec3 uKs;
uniform float uShininess;

uniform vec3 uLightPos1_vs;
uniform vec3 uLightIntensity1;

uniform vec3 uLightPos2_vs;
uniform vec3 uLightIntensity2;

uniform sampler2D uTexture;

vec3 blinnPhong(vec3 lightPos, vec3 lightIntensity) {
    vec3 N = normalize(vNormal_vs);
    vec3 L = normalize(lightPos - vPosition_vs);
    vec3 V = normalize(-vPosition_vs);
    vec3 H = normalize(L + V);

    float distance = length(lightPos - vPosition_vs);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.03 * (distance * distance));

    float diffuse = max(dot(N, L), 0.0);
    float specular = pow(max(dot(N, H), 0.0), uShininess);

    return attenuation * (uKd * lightIntensity * diffuse + uKs * lightIntensity * specular);
}

void main()
{
    vec3 color1 = blinnPhong(uLightPos1_vs, uLightIntensity1);
    vec3 color2 = blinnPhong(uLightPos2_vs, uLightIntensity2);
    vec3 lighting = color1 + color2;

    vec4 textureColor = texture(uTexture, vTexCoords);
    fFragColor = lighting * textureColor.rgb;
}