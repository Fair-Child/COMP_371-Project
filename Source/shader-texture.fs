//  A1_29644490
// COMP 371 Assignment 1
// Created by Matthew Salaciak 29644490.



#version 330 core




in vec3 vertexColor;
in vec2 vertexUV;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;

out vec4 FragColor;

uniform sampler2D textureSampler;
uniform sampler2D shadowMap;


uniform vec3 color;
uniform  bool textureOn;
uniform bool withText;
uniform bool shadowsOn;



uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    
    //bias for shadow acne using PCF
    float bias = 0.003;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 24.0;

    return shadow;
}

float shadowCalculationNoTexture(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    
    //bias for shadow acne using PCF
    float bias = 0.006;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 12.0;

    return shadow;
}



void main()
{
    
    if(FragPos.y <= 1) {
        FragColor = vec4(0.39f,0.65f,0.77f,1.0f); //water
    } else if (FragPos.y >= 1 && FragPos.y <=5.5){
        FragColor = vec4(0.86f,0.70f,0.30f,1.0f); //sand
    } else if (FragPos.y >= 5.5 && FragPos.y <=17.5){
        FragColor = vec4(0.35f,0.56f,0.30f,1.0f);
       }else if (FragPos.y >= 17.5 && FragPos.y <=26.5){
        FragColor = vec4(0.75f,0.53f,0.4f,0.36f);
       } else {
           FragColor = vec4(1.0f);
       }
        
    
}


