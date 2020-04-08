//  A1_29644490
// COMP 371 Assignment 1
// Created by Matthew Salaciak 29644490.


#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;

//uniform matrices
uniform mat4 mvp;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

//uniforms to calculate the normal's and change of Y coordinate
uniform vec3 eyes;
uniform float newY;

//out's
out vec3 vertexColor;
out vec2 vertexUV;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace;


//flat variables
flat out vec3 flatFragPos;
flat out vec4 flatFragPosLightSpace;






void main()
{
    //take the uniform newY to change the Y cordinate of the vertices
    vec3 calPos = aPos;

    //update the change in Y coordinate
    float changeY = calPos.y * newY;
    calPos.y = calPos.y + changeY;
    
    //calculate normals here


    
    vertexUV = aUV;
    FragPos = vec3(mvp * vec4(calPos,1.0f));
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos,1.0f);
    //set flat variables
    flatFragPos = FragPos;
    flatFragPosLightSpace =FragPosLightSpace;
   
    gl_Position = projection * view * mvp * vec4(calPos, 1.0f);
    
    
    
    
}

