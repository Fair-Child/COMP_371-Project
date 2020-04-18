// COMP-371 Group Project Team 15
// Procedural World Generation
// Matthew Salaciak 29644490
// Jeremy Gaudet 40045224
//  Elsa Donovan 26857655



#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec3 aColor;
layout (location = 4) in mat4 aInstanceMatrix;


//uniform matrices
uniform mat4 mvp;
//uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

//uniforms to calculate the normal's and change of Y coordinate
uniform vec3 eyes;
uniform float newY;

//out's
out vec3 vertexColor;
out vec2 vertexUV;
out vec3 objNormal;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace;

//mat in
uniform Mat{
    vec4 aAmbient;
    vec4 aDiffuse;
    vec4 aSpecular;
};

//mat out
 
out vec4 Ambient;
out vec4 Diffuse;
out vec4 Specular;



//flat variables
flat out vec3 flatFragPos;
flat out vec4 flatFragPosLightSpace;


uniform bool instanceOn;




void main()
{
    
    //mat assign to out
    Ambient = aAmbient;
    Diffuse = aDiffuse;
    Specular = aSpecular;
    
    //take the uniform newY to change the Y cordinate of the vertices
     vec3 calPos = aPos;
    
    vertexColor = aColor;
    
    //update the change in Y coordinate
    float changeY = calPos.y * newY;
    calPos.y = calPos.y + changeY;
    

    
    
    mat4 matrixTotal;
    
    if (!instanceOn) {
        
        objNormal = mat3(transpose(inverse(mvp))) * aNormal;
        
        
        vertexUV = aUV;
        FragPos = vec3(mvp * vec4(calPos,1.0f));
        FragPosLightSpace = lightSpaceMatrix * vec4(FragPos,1.0f);
        //set flat variables
        flatFragPos = FragPos;
        flatFragPosLightSpace =FragPosLightSpace;
        
        matrixTotal = projection * view * mvp;
        gl_Position = matrixTotal * vec4(calPos, 1.0f);
    }
    else {
        objNormal = mat3(transpose(inverse(aInstanceMatrix))) * aNormal;
        
        
        vertexUV = aUV;
        FragPos = vec3(aInstanceMatrix * vec4(calPos,1.0f));
        FragPosLightSpace = lightSpaceMatrix * vec4(FragPos,1.0f);
        //set flat variables
        flatFragPos = FragPos;
        flatFragPosLightSpace =FragPosLightSpace;
        matrixTotal = projection * view * aInstanceMatrix;
        gl_Position = matrixTotal * vec4(calPos, 1.0f);
    }
    

    
    
    
    
}

