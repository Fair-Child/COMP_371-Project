// COMP-371 Group Project Team 15
// Procedural World Generation
// Matthew Salaciak 29644490
// Jeremy Gaudet 40045224
//  Elsa Donovan 26857655


#include <iostream>
#include <list>
#define GLEW_STATIC 1
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/common.hpp>

using namespace glm;
using namespace std;




struct lamp
{
    lamp(vec3 _position, vec3 _normal)
    : position(_position), normal(_normal) {}
   
      vec3 position;
     
      vec3 normal;
};

 unsigned int numOfVerticeslightCube;

static const lamp lightcube[] = {
       lamp(vec3(-0.5f,-0.5f,-0.5f), vec3(0.0f,  0.0f, -1.0f)), //left - red
       lamp(vec3(-0.5f,-0.5f, 0.5f),  vec3(0.0f,  0.0f, -1.0f)),
       lamp(vec3(-0.5f, 0.5f, 0.5f),  vec3(0.0f,  0.0f, -1.0f)),
       lamp(vec3(-0.5f,-0.5f,-0.5f),  vec3(0.0f,  0.0f, -1.0f)),
       lamp(vec3(-0.5f, 0.5f, 0.5f), vec3(0.0f,  0.0f, -1.0f)),
       lamp(vec3(-0.5f, 0.5f,-0.5f), vec3(0.0f,  0.0f, -1.0f)),
       
       lamp(vec3( 0.5f, 0.5f,-0.5f), vec3(0.0f,  0.0f,  1.0f)), // far - blue
       lamp(vec3(-0.5f,-0.5f,-0.5f), vec3(0.0f,  0.0f,  1.0f)),
       lamp(vec3(-0.5f, 0.5f,-0.5f), vec3(0.0f,  0.0f,  1.0f)),
       lamp(vec3( 0.5f, 0.5f,-0.5f),  vec3(0.0f,  0.0f,  1.0f)),
       lamp(vec3( 0.5f,-0.5f,-0.5f),  vec3(0.0f,  0.0f,  1.0f)),
       lamp(vec3(-0.5f,-0.5f,-0.5f),  vec3(0.0f,  0.0f,  1.0f)),
       
       lamp(vec3( 0.5f,-0.5f, 0.5f),  vec3(-1.0f,  0.0f,  0.0f)), // bottom - turquoise
       lamp(vec3(-0.5f,-0.5f,-0.5f),  vec3(-1.0f,  0.0f,  0.0f)),
       lamp(vec3( 0.5f,-0.5f,-0.5f),  vec3(-1.0f,  0.0f,  0.0f)),
       lamp(vec3( 0.5f,-0.5f, 0.5f),  vec3(-1.0f,  0.0f,  0.0f)),
       lamp(vec3(-0.5f,-0.5f, 0.5f),  vec3(-1.0f,  0.0f,  0.0f)),
       lamp(vec3(-0.5f,-0.5f,-0.5f),  vec3(-1.0f,  0.0f,  0.0f)),
       
       lamp(vec3(-0.5f, 0.5f, 0.5f), vec3(1.0f,  0.0f,  0.0f)), // near - green
       lamp(vec3(-0.5f,-0.5f, 0.5f), vec3(1.0f,  0.0f,  0.0f)),
       lamp(vec3( 0.5f,-0.5f, 0.5f),  vec3(1.0f,  0.0f,  0.0f)),
       lamp(vec3( 0.5f, 0.5f, 0.5f),  vec3(1.0f,  0.0f,  0.0f)),
       lamp(vec3(-0.5f, 0.5f, 0.5f),  vec3(1.0f,  0.0f,  0.0f)),
       lamp(vec3( 0.5f,-0.5f, 0.5f),  vec3(1.0f,  0.0f,  0.0f)),
       
       lamp(vec3( 0.5f, 0.5f, 0.5f),  vec3(0.0f, -1.0f,  0.0f)), // right - purple
       lamp(vec3( 0.5f,-0.5f,-0.5f),   vec3(0.0f, -1.0f,  0.0f)),
       lamp(vec3( 0.5f, 0.5f,-0.5f),  vec3(0.0f, -1.0f,  0.0f)),
       lamp(vec3( 0.5f,-0.5f,-0.5f),   vec3(0.0f, -1.0f,  0.0f)),
       lamp(vec3( 0.5f, 0.5f, 0.5f),   vec3(0.0f, -1.0f,  0.0f)),
       lamp(vec3( 0.5f,-0.5f, 0.5f),   vec3(0.0f, -1.0f,  0.0f)),
       
       lamp(vec3( 0.5f, 0.5f, 0.5f),  vec3(0.0f,  1.0f,  0.0f)), // top - yellow
       lamp(vec3( 0.5f, 0.5f,-0.5f),  vec3(0.0f,  1.0f,  0.0f)),
       lamp(vec3(-0.5f, 0.5f,-0.5f), vec3(0.0f,  1.0f,  0.0f)),
       lamp(vec3( 0.5f, 0.5f, 0.5f), vec3(0.0f,  1.0f,  0.0f)),
       lamp(vec3(-0.5f, 0.5f,-0.5f),  vec3(0.0f,  1.0f,  0.0f)),
       lamp(vec3(-0.5f, 0.5f, 0.5f), vec3(0.0f,  1.0f,  0.0f))
   

  };

float skyboxVertices[] = {
    // positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};
