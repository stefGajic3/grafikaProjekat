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
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}

//#shader fragment
#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;

// directional light
uniform vec3 dirLightDir;
uniform vec3 dirLightColor;

// point light (lamp)
uniform vec3 lightPos;
uniform vec3 lightColor;

uniform vec3 viewPos;

void main()
{
    vec3 color = texture(texture_diffuse1, TexCoords).rgb;
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);


    vec3 dirLightDirNorm = normalize(-dirLightDir);

    float diffDir = max(dot(norm, dirLightDirNorm), 0.0);
    vec3 diffuseDir = diffDir * dirLightColor * color;

    vec3 reflectDir = reflect(-dirLightDirNorm, norm);
    float specDir = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specularDir = 0.1 * specDir * dirLightColor;

    vec3 ambientDir = 0.1 * dirLightColor * color;


    vec3 lightDir = normalize(lightPos - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * color;

    vec3 reflectDirPoint = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDirPoint), 0.0), 32.0);
    vec3 specular = 0.25 * spec * lightColor;


    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.14 * distance + 0.07 * distance * distance);

    vec3 ambientPoint = 0.05 * lightColor * color;


    vec3 result =
    ambientDir + diffuseDir + specularDir +
    attenuation * (ambientPoint + diffuse + specular);

    FragColor = vec4(result, 1.0);
}