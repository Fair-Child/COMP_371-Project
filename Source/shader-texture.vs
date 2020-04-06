//  A1_29644490
// COMP 371 Assignment 1
// Created by Matthew Salaciak 29644490.


#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;


uniform mat4 mvp;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

uniform vec3 eyes;

out vec3 vertexColor;
out vec2 vertexUV;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace;
out vec3 POS;





void main()
{
    
    
    
    //calculate normals here
    vec4 pos1 = vec4(aPos, 1.0);
    POS = (mvp * pos1 ).xyz - eyes;

    vertexUV = aUV;
    FragPos = vec3(mvp * vec4(aPos,1.0f));
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos,1.0f);
    gl_Position = projection * view * mvp * vec4(aPos, 1.0f);
    
    
    
    
}

