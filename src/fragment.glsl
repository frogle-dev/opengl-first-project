#version 460 core

out vec4 FragColor;

uniform samplerCube cubemap;
in vec3 texCoord;

in vec3 fragPos;


in vec3 normal;

struct Material
{
    samplerCube diffuse;
    samplerCube specular;
    samplerCube emission;
    float emissionStrength;
    float shininess;
};


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

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform SpotLight spotLight;

uniform vec3 viewPos;
uniform Material material;


vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);


void main()
{
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos - fragPos);

    // directional lights
    vec3 result = calcDirLight(dirLight, norm, viewDir);

    // point lights
    // for (int i = 0; i < NUM_POINT_LIGHTS; i++)
    // {
    //     result += calcPointLight(pointLights[i], norm, fragPos, viewDir);
    // }

    result += calcSpotLight(spotLight, norm, fragPos, viewDir);

    // emission
    vec3 emission = texture(material.emission, texCoord).rgb * material.emissionStrength;
    result += emission;

    FragColor = vec4(result, 1.0);
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
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoord));
    return (ambient + diffuse + specular);
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
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoord));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

    vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoord));

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
    // combine diffuse specular and ambient
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoord));
    diffuse *= intensity;
    specular *= intensity;

    return (ambient + diffuse + specular);  
}