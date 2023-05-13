#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;

vec4 fPosEye;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

// components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

// fog
uniform bool enableFog;

// spotlight
float constant;
float linear;
float quadratic;
uniform vec3 lightPos;
uniform vec3 lightDirFront;
uniform bool enableSpotLight;

// thunder
uniform bool enableThunder;
uniform float extraLight;

// second light
uniform vec3 lightDir2;
uniform vec3 lightColor2;
vec3 ambient2;
vec3 diffuse2;
vec3 specular2;

void computeDirLight()
{
    if (enableSpotLight == false){

        if (enableThunder == true){
            ambientStrength += extraLight;
        }
        //compute eye space coordinates
        fPosEye = view * model * vec4(fPosition, 1.0f);
        vec3 normalEye = normalize(normalMatrix * fNormal);

        //normalize light direction
        vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

        //compute view direction (in eye coordinates, the viewer is situated at the origin
        vec3 viewDir = normalize(- fPosEye.xyz);

        //compute ambient light
        ambient = ambientStrength * lightColor;

        //compute diffuse light
        diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

        //compute specular light
        vec3 reflectDir = reflect(-lightDirN, normalEye);
        float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
        specular = specularStrength * specCoeff * lightColor;

        // light 2
        //normalize light direction
        vec3 lightDirN2 = vec3(normalize(view * vec4(lightDir2, 0.0f)));

        ambient2 = ambientStrength * lightColor2;

        //compute diffuse light
        diffuse2 = max(dot(normalEye, lightDirN2), 0.0f) * lightColor2;

        //compute specular light
        vec3 reflectDir2 = reflect(-lightDirN2, normalEye);
        float specCoeff2 = pow(max(dot(viewDir, reflectDir2), 0.0f), 32);
        specular2 = specularStrength * specCoeff2 * lightColor2;
    }
}

void computeSpotLight()
{
    if (enableSpotLight == true){
        //compute eye space coordinates
        fPosEye = vec4(fPosition, 1.0f);
        vec3 normalEye = normalize(fNormal);

        //normalize light direction
        vec3 lightDirN = normalize(lightPos - lightDirFront);

        vec3 origin = vec3(0.0f);
    
        vec3 viewDir = normalize(lightPos - fPosEye.xyz);

        float diff = max(dot(normalEye, lightDirN), 0.0f);

        //compute ambient light
        ambient = ambientStrength * lightColor;

        //compute diffuse light
        diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

        //compute specular light
        vec3 reflectDir = reflect(lightDirN, normalEye);
        float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
        specular = specularStrength * specCoeff * lightColor;

        //lumina
        float theta = dot(lightDirN, normalize(-lightDirFront));
        float epsilon = 0.3f;
        float intensity = clamp((theta - 0.2f)/epsilon, 0.0, 1.0);
        diffuse *= intensity;
        specular *= intensity;

        float constant = 1.0f;
        float linear = 0.09f;
        float quadratic = 0.032f;

        float distance = length(lightPos - fPosEye.xyz);
        float attenuation = 1.0f / (constant + linear * distance + quadratic * distance * distance);

        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;
    }
}

float computeFog()
{
    float fogDensity = 0.05f;
	float fragmentDistance = length(fPosEye);
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
    return clamp(fogFactor, 0.0f, 1.0f);
}

void main() 
{
    computeDirLight();
    computeSpotLight();
    
    vec3 color = min(((ambient + ambient2) + (diffuse + diffuse2)) * texture(diffuseTexture, fTexCoords).rgb + (specular + specular2)* texture(specularTexture, fTexCoords).rgb, 1.0f);

    if (enableFog == true){
        vec4 fogColor = vec4(0.5f,0.5f,0.5f,1.0f);
        float fogFactor = computeFog();
        fColor = fogColor * (1- fogFactor) + vec4(color * fogFactor,1);
    }
    else{
        fColor = vec4(color, 1.0f);
    }
}
