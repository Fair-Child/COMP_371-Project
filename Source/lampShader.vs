// COMP-371 Group Project Team 15
// Procedural World Generation
// Matthew Salaciak 29644490
// Jeremy Gaudet 40045224
//  Elsa Donovan 26857655


#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 mvp;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
in vec3 color;
out vec3 vertexColor;

void main()
{
    gl_Position = mvp * vec4(aPos, 1.0f);
    vertexColor = color;
}
