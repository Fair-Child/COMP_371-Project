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

uniform sampler2D snowTexture;
uniform sampler2D sandyTexture;
uniform sampler2D rockyTexture;
uniform sampler2D grassTexture;
uniform sampler2D waterTexture;
uniform sampler2D shadowMap;


uniform vec3 color;
uniform  bool textureOn;
uniform bool shadowsOn;



uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

      

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
                     float shadowNoText = shadowCalculationNoTexture(FragPosLightSpace);
                      vec3 result;
                        float ambientStrength = 0.9;
                       vec3 ambient = ambientStrength * lightColor;
               
                       //diffuse
                       vec3 norm = normalize(Normal);
                       vec3 lightDirection = normalize(lightPos - FragPos);
                       float diff = max(dot(norm, lightDirection), 0.0f);
                       vec3 diffuse = diff * lightColor;
               
                       float specularStrength = 0.9;
                       vec3 viewDir = normalize(viewPos - FragPos);
                       vec3 reflectDir = reflect(-lightDirection, norm);
                       float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
                       vec3 specular = specularStrength * spec * lightColor;
                       result = (ambient  + diffuse + specular);
    
    
    if(textureOn) {
    
    if(FragPos.y <= 1) {
//        vec3 waterColor = vec3(0.39f,0.65f,0.77f);
        
        FragColor = texture(waterTexture, vertexUV); //water
    } else if (FragPos.y >= 1 && FragPos.y <=5.5){
//        vec3 sandColor = vec3(0.86f,0.70f,0.30f);
            FragColor = texture(sandyTexture, vertexUV); //sand
    } else if (FragPos.y >= 5.5 && FragPos.y <=17.5){
        vec3 grassColor =vec3(0.35f,0.56f,0.30f);
//        FragColor = vec4(grassColor* result ,1.0f); //grass
             FragColor = texture(grassTexture, vertexUV);
       }else if (FragPos.y >= 17.5 && FragPos.y <=26.5){
//           vec3 rockColor = vec3(0.75f,0.53f,0.4f);
         FragColor = texture(rockyTexture, vertexUV);
       } else {
           vec3 snowColor = vec3(1.0f);
//           FragColor = vec4(snowColor,1.0f);
           FragColor = texture(snowTexture, vertexUV); //snow
       }
        
    } else {
         if(FragPos.y <= 1) {
                vec3 waterColor = vec3(0.39f,0.65f,0.77f);
                
                FragColor = vec4(waterColor * result,1.0f); //water
            } else if (FragPos.y >= 1 && FragPos.y <=5.5){
                vec3 sandColor = vec3(0.86f,0.70f,0.30f);
                FragColor = vec4(sandColor* result ,1.0f); //sand
            } else if (FragPos.y >= 5.5 && FragPos.y <=17.5){
                vec3 grassColor =vec3(0.35f,0.56f,0.30f);
                FragColor = vec4(grassColor* result ,1.0f); //grass
               }else if (FragPos.y >= 17.5 && FragPos.y <=26.5){
                   vec3 rockColor = vec3(0.75f,0.53f,0.4f);
                FragColor = vec4(rockColor* result ,1.0f); //rock
               } else {
                   vec3 snowColor = vec3(1.0f);
                   FragColor = vec4(snowColor,1.0f);

               }
    }
}


