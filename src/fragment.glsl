#version 460 core

out vec4 FragColor;

in vec3 fragPos;
in vec3 normal;

uniform sampler2DArray texArray;
in vec2 texCoord;

#define TEX_LAYER_WIDTH 4096
#define TEX_LAYER_HEIGHT 4096


struct Material
{
    int diffuseLayerCount;
    int diffuseStartLayer;
    int specularLayerCount;
    int specularStartLayer;
    int emissionLayerCount;
    int emissionStartLayer;

    float emissionStrength;
    float shininess;
};
uniform Material material;


struct DirLight
{
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;

struct PointLight 
{    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
#define NUM_POINT_LIGHTS 1  
uniform PointLight pointLights[NUM_POINT_LIGHTS];

struct SpotLight
{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform SpotLight spotLight;

uniform vec3 viewPos;


vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

vec3 calcAmbient(vec3 lightAmbient);
vec3 calcDiffuse(vec3 lightDiffuse, float diffuseAmount);
vec3 calcSpecular(vec3 lightSpecular, float specularAmount);



void main()
{
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos - fragPos);

    // directional lights
    vec3 result = calcDirLight(dirLight, norm, viewDir);

    // point lights
    for (int i = 0; i < NUM_POINT_LIGHTS; i++)
    {
        result += calcPointLight(pointLights[i], norm, fragPos, viewDir);
    }

    result += calcSpotLight(spotLight, norm, fragPos, viewDir);

    // emission
    vec3 emission = vec3(0.0);
    for (int i = 0; i < material.emissionLayerCount; i++)
    {
        emission += material.emissionStrength * vec3(texture(texArray, vec3(texCoord, float(material.emissionStartLayer + i))));
    }
    result += emission;

    FragColor = vec4(result, 1.0);
}


vec3 calcAmbient(vec3 lightAmbient)
{
    vec3 ambient = vec3(0.0);
    for (int i = 0; i < material.diffuseLayerCount; i++)
    {
        ambient += lightAmbient * vec3(texture(texArray, vec3(texCoord, float(material.diffuseStartLayer + i))));
    }

    return ambient;
}
vec3 calcDiffuse(vec3 lightDiffuse, float diffuseAmount)
{
    vec3 diffuse = vec3(0.0);
    for (int i = 0; i < material.diffuseLayerCount; i++)
    {
        diffuse += lightDiffuse * diffuseAmount * vec3(texture(texArray, vec3(texCoord, float(material.diffuseStartLayer + i))));
    }

    return diffuse;
}
vec3 calcSpecular(vec3 lightSpecular, float specularAmount)
{
    vec3 specular = vec3(0.0);
    for (int i = 0; i < material.specularLayerCount; i++)
    {
        specular += lightSpecular * specularAmount * vec3(texture(texArray, vec3(texCoord, float(material.specularStartLayer + i))));
    }

    return specular;
}


vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine diffuse specular and ambient
    vec3 result = calcAmbient(light.ambient) + calcDiffuse(light.diffuse, diff) + calcSpecular(light.specular, spec);
    return result;
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine diffuse specular and ambient
    vec3 ambient = calcAmbient(light.ambient);
    vec3 diffuse = calcDiffuse(light.diffuse, diff);
    vec3 specular = calcSpecular(light.specular, spec);
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);


    // angle between where the spotlight is pointing and the fragment
    float theta = dot(lightDir, normalize(-light.direction));
    // values for smooth cutoff at the edges
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = smoothstep(0.0, 1.0, (theta - light.outerCutOff) / epsilon);

    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient = calcAmbient(light.ambient);
    vec3 diffuse = calcDiffuse(light.diffuse, diff);
    vec3 specular = calcSpecular(light.specular, spec);
    diffuse *= intensity;
    specular *= intensity;

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);  
}