//#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = vec3(worldPos);
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;

    gl_Position = projection * view * worldPos;
}

//#shader fragment
#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_roughness1;

// directional light
uniform vec3 dirLightDir;
uniform vec3 dirLightColor;

// point light
uniform vec3 lightPos;
uniform vec3 lightColor;

uniform vec3 viewPos;

// material
uniform float materialShininess;
uniform float materialSpecularStrength;

void main()
{
    vec3 color = texture(texture_diffuse1, TexCoords).rgb;
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);


    float roughness = texture(texture_roughness1, TexCoords).r;

    float finalSpecularStrength = (1.0 - roughness) * materialSpecularStrength;
    float finalShininess = mix(8.0, materialShininess, 1.0 - roughness);

    // DIRECTIONAL LIGHT
    vec3 dirLightDirNorm = normalize(-dirLightDir);

    vec3 ambientDir = 0.10 * dirLightColor * color;

    float diffDir = max(dot(norm, dirLightDirNorm), 0.0);
    vec3 diffuseDir = diffDir * dirLightColor * color;

    vec3 reflectDirDir = reflect(-dirLightDirNorm, norm);
    float specDir = pow(max(dot(viewDir, reflectDirDir), 0.0), finalShininess);
    vec3 specularDir = finalSpecularStrength * specDir * dirLightColor;

    // POINT LIGHT
    vec3 lightDir = normalize(lightPos - FragPos);

    vec3 ambientPoint = 0.05 * lightColor * color;

    float diffPoint = max(dot(norm, lightDir), 0.0);
    vec3 diffusePoint = diffPoint * lightColor * color;

    vec3 reflectDirPoint = reflect(-lightDir, norm);
    float specPoint = pow(max(dot(viewDir, reflectDirPoint), 0.0), finalShininess);
    vec3 specularPoint = finalSpecularStrength * specPoint * lightColor;

    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.14 * distance + 0.07 * distance * distance);

    vec3 result =
    ambientDir + diffuseDir + specularDir +
    attenuation * (ambientPoint + diffusePoint + specularPoint);

    FragColor = vec4(result, 1.0);
}