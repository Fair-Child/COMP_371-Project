// COMP-371 Group Project Team 15
// Procedural World Generation
// Matthew Salaciak 29644490
// Jeremy Gaudet 40045224
//  Elsa Donovan 26857655




#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 mvp;

void main()
{
    
    
    gl_Position =  lightSpaceMatrix * mvp * vec4(aPos, 1.0);
}
