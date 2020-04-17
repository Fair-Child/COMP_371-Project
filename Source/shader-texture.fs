//  A1_29644490
// COMP 371 Assignment 1
// Created by Matthew Salaciak 29644490.



#version 330 core



//in's
in vec3 vertexColor;
in vec2 vertexUV;
in vec3 objNormal;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;

//flat in's
flat in vec3 flatFragPos;
flat in vec4 flatFragPosLightSpace;


out vec4 FragColor;

//textures
uniform sampler2D snowTexture;
uniform sampler2D sandyTexture;
uniform sampler2D rockyTexture;
uniform sampler2D grassTexture;
uniform sampler2D waterTexture;
uniform sampler2D shadowMap;

//obj textures
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;

//mat in
in vec4 Ambient;
in vec4 Diffuse;
in vec4 Specular;



//bools
uniform  bool textureOn;
uniform bool flatOn;
uniform bool treeColor;

//lighting uniforms
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;



float shadowCalculation(vec4 fragPosLightSpace)
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



void main()
{
    
    //    calculate the normals here!
      vec3 result;
//    vec3 Normal = Normal;

    if(flatOn) {
         vec3 Normal = normalize( cross( dFdx( flatFragPos.xyz ), dFdy( flatFragPos.xyz ) ) );
        float shadow = shadowCalculation(flatFragPosLightSpace);

        float ambientStrength = 0.9;
        vec3 ambient = ambientStrength * lightColor;

        //diffuse
        vec3 norm = normalize(Normal);
        vec3 lightDirection = normalize(lightPos - flatFragPos);
        float diff = max(dot(norm, lightDirection), 0.0f);
        vec3 diffuse = diff * lightColor;

        float specularStrength = 0.8;
        vec3 viewDir = normalize(viewPos - flatFragPos);
        vec3 reflectDir = reflect(-lightDirection, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0f);
        vec3 specular = specularStrength * spec * lightColor;

        result = (ambient  + diffuse + specular) * (1 - shadow);
    } else if(!flatOn) {

        vec3 Normal = normalize( cross( dFdx( FragPos.xyz ), dFdy( FragPos.xyz ) ) );

        float shadow = shadowCalculation(FragPosLightSpace);

        float ambientStrength = 0.5;
        vec3 ambient = ambientStrength * lightColor;

        //diffuse
        vec3 norm = normalize(Normal);
        vec3 lightDirection = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDirection), 0.0f);
        vec3 diffuse = diff * lightColor;

        float specularStrength = 0.8;
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDirection, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0f);
        vec3 specular = specularStrength * spec * lightColor;

        result = (ambient  + diffuse + specular) * (1 - shadow);
    }


        if(textureOn) {

            if(FragPos.y <= 1) {


                vec4 waterColor = vec4(0.39f,0.65f,0.77f,1.0f);
                vec4 textResult= vec4(result * texture(waterTexture, vertexUV).rgb ,1.0f);
                FragColor = textResult * waterColor ; //water

            } else if (FragPos.y >= 1 && FragPos.y <=5.5){
                vec4 sandColor = vec4(0.86f,0.70f,0.30f,1.0f);
                vec4 textResult= vec4(result * texture(sandyTexture, vertexUV).rgb ,1.0f);
                //            FragColor = texture(sandyTexture, vertexUV) * sandColor; //sand
                FragColor = textResult * sandColor;

            } else if (FragPos.y >= 5.5 && FragPos.y <=17.5){
                vec4 grassColor =vec4(0.35f,0.56f,0.30f,1.0f);
                vec4 textResult= vec4(result * texture(grassTexture, vertexUV).rgb ,1.0f);
                //            FragColor = texture(grassTexture, vertexUV) * grassColor;;
                FragColor = textResult * grassColor;

            }else if (FragPos.y >= 17.5 && FragPos.y <=26.5){
                vec4 rockColor = vec4(0.75f,0.53f,0.4f,1.0f);
                vec4 textResult= vec4(result * texture(rockyTexture, vertexUV).rgb ,1.0f);
                //            FragColor = texture(rockyTexture, vertexUV) * rockColor;
                FragColor = textResult * rockColor;

            } else  if(FragPos.y >26.5){
                vec4 snowColor = vec4(1.0f);
                vec4 textResult= vec4(result * texture(snowTexture, vertexUV).rgb ,1.0f);
                //            FragColor = texture(snowTexture, vertexUV) * snowColor; //snow
                FragColor = textResult * snowColor;
            }

        } else if(!textureOn){

            if(FragPos.y <= 1) {
                vec3 waterColor = vec3(0.39f,0.65f,0.77f);
                FragColor = vec4(waterColor * result,1.0f); //water
            }
            else if (FragPos.y >= 1 && FragPos.y <=5.5){
                vec3 sandColor = vec3(0.86f,0.70f,0.30f);
                FragColor = vec4(sandColor* result ,1.0f); //sand
            }
            else if (FragPos.y >= 5.5 && FragPos.y <=17.5){
                vec3 grassColor =vec3(0.35f,0.56f,0.30f);
                FragColor = vec4(grassColor* result ,1.0f); //grass
            }
            else if (FragPos.y >= 17.5 && FragPos.y <=26.5){
                vec3 rockColor = vec3(0.75f,0.53f,0.4f);
                FragColor = vec4(rockColor* result ,1.0f); //rock
            }
            else if(FragPos.y >26.5) {
                vec3 snowColor = vec3(1.0f);
                FragColor = vec4(snowColor * result,1.0f);

            }
        }
    
    if(treeColor) {
       
        if(textureOn) {
            
        FragColor = vec4( texture(texture_diffuse1, vertexUV));
        } else if(!textureOn) {
             FragColor = vec4(Diffuse.rgb,1.0f);
        }

     
    
    }

}
