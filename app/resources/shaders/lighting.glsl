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

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;

// directional light
uniform vec3 dirLightDir;
uniform vec3 dirLightColor;

// point light
uniform vec3 lightPos;
uniform vec3 lightColor;

// point shadows
uniform samplerCube depthMap;
uniform float far_plane;
uniform bool shadows;

uniform vec3 viewPos;

// material
uniform float materialShininess;
uniform float materialSpecularStrength;

float ShadowCalculation(vec3 fragPos)
{
    vec3 fragToLight = fragPos - lightPos;

    float currentDepth = length(fragToLight);

    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;

    vec3 sampleOffsetDirections[20] = vec3[]
    (
    vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
    vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
    vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
    vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
    );

    for (int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(
            depthMap,
            fragToLight + sampleOffsetDirections[i] * diskRadius
        ).r;

        closestDepth *= far_plane;

        
        if (currentDepth - bias > closestDepth)
        shadow += 1.0;
    }

    shadow /= float(samples);

    return shadow;
}
void main()
{
    vec3 color = texture(texture_diffuse1, TexCoords).rgb;
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Directional light
    vec3 dirLightDirNorm = normalize(-dirLightDir);

    vec3 ambientDir = 0.10 * dirLightColor * color;

    float diffDir = max(dot(norm, dirLightDirNorm), 0.0);
    vec3 diffuseDir = diffDir * dirLightColor * color;

    vec3 reflectDirDir = reflect(-dirLightDirNorm, norm);
    float specDir = pow(max(dot(viewDir, reflectDirDir), 0.0), materialShininess);
    vec3 specularDir = materialSpecularStrength * specDir * dirLightColor;


    // Point light
    vec3 lightDir = normalize(lightPos - FragPos);

    vec3 ambientPoint = 0.05 * lightColor * color;

    float diffPoint = max(dot(norm, lightDir), 0.0);
    vec3 diffusePoint = diffPoint * lightColor * color;

    vec3 reflectDirPoint = reflect(-lightDir, norm);
    float specPoint = pow(max(dot(viewDir, reflectDirPoint), 0.0), materialShininess);
    vec3 specularPoint = materialSpecularStrength * specPoint * lightColor;

    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.14 * distance + 0.07 * distance * distance);

    float shadow = shadows ? ShadowCalculation(FragPos) : 0.0;

    vec3 pointLighting =
    ambientPoint + (1.0 - shadow) * (diffusePoint + specularPoint);

    vec3 result =
    ambientDir + diffuseDir + specularDir +
    attenuation * pointLighting;

    FragColor = vec4(result, 1.0);
    BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}