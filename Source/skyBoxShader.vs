// COMP-371 Group Project Team 15
// Procedural World Generation
// Matthew Salaciak 29644490
// Jeremy Gaudet 40045224
//  Elsa Donovan 26857655





#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
    
}  
