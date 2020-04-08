//
// COMP 371 GROUP PROJECT
//
// Created by Matthew Salaciak 29644490.
//
// Inspired by the COMP 371 Lectures and Lab 2,3 and 4 and the following tutorials
// - https://learnopengl.com/Getting-started/Hello-Triangle
// - https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL (for shader class)
// - http://www.opengl-tutorial.org/beginners-tutorials/tutorial-3-matrices/
// - texutres from textures.com
// - https://www.enkisoftware.com/devlogpost-20150131-1-Normal-generation-in-the-pixel-shader fragshader calculate normal
//- thinmatrix for chunk terrain stuff
//- learnopengl for cube maps sky box
//- https://ronvalstar.nl/creating-tileable-noise-maps tips for how we made our noise work in the for loop part
// - sky box image from https://opengameart.org/content/sky-box-sunny-day
// biome color inspiration https://forum.unity.com/threads/random-biome-generation.512970/

//SimeplexNoise is a library https://github.com/SRombauts/SimplexNoise
//It created by Sébastien Rombauts



#include <iostream>
#include <list>
#define GLEW_STATIC 1
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/common.hpp>
#include "vertices.h"
#include "shader.h"
#include <time.h>
#include <vector>
#include "texture-loader.h"
#include "SimplexNoise.h"
#include "filesystem.h"
#include "model.h"



//define namespaces for glm and c++ std
using namespace glm;
using namespace std;




//global variables and functions for the project
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void renderLight(const GLuint &lamp_Shader);
float Remap (float value, float from1, float to1, float from2, float to2);
void createMap(Model &treeModel);
void renderTerrain(vector <GLuint> &VAO, Shader &shader, int &nIndices, vec3 &cameraPosition, Model &testModel);
void createTerrainGeometry(GLuint &VAO, GLuint &treeVAO, int &xOffset, int &yOffset, Model &treeModel);


//this is not used right now
bool textureOn = true;
bool shadowsOn = true;
bool updateMap = false;
vec3 lightpos (149.0f, 38.0f,151.0f);



//worldspace matrix

mat4 projectionMatrix = mat4(1.0f);
mat4 viewMatrix;
mat4 modelMatrix = mat4(1.0f);
mat4 modelViewProjection;


mat4 WorldTransformMatrix(1.f);
//textures

GLuint depthMap;
GLuint snowTextureID;
GLuint rockTextureID;
GLuint sandTextureID;
GLuint grassTextureID;
GLuint waterTextureID;
vector<GLuint> terrainVAOs(100);
vector<GLuint> treeVAOs(100);

//primatative rendering options
int primativeRender = GL_TRIANGLES;


//camera info
vec3 cameraPosition(0.0f,56.0f,30.0f);
vec3 cameraLookAt(0.0f, 0.0f, 0.0f);
vec3 cameraUp(0.0f, 1.0f, 0.0f);

//vectors that hold geomtry information





//params to start

const int renderDistance = 10;
const int xMapChunks = 10;
const int yMapChunks = 10;
const int mapX = 64;
const int mapY = 64;
int nIndices = mapX * mapY * 6;


//noise options (would love to make this user definable with a GUI ...a boy can dream

int octaves = 5;
float meshHeight = 25;  // Vertical scaling
float noiseScale = 64;  // Horizontal scaling
float persistence = 0.5;
float lacunarity = 2;





int main(int argc, char*argv[])
{
    
    //random number initialization and int primativeRender is used to store the default rendering option which is GL_TRIANGLES
    srand (time(NULL));
    
    float  fovAngle = 45.0f;
    
    
    // Initialize GLFW and OpenGL version
    
    glfwInit();
    
#if defined(PLATFORM_OSX)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    // On windows, we set OpenGL version to 2.1, to support more hardware
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#endif
    
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    int screenWidth, screenHeight;
    
    // Create Window and rendering context using GLFW, resolution is 1024x768
    GLFWwindow* window = glfwCreateWindow(1024, 768, "A1_29644490", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    
    
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);
    
    
    
    //texture shader for grid, olaf
    
    Shader textureShader(FileSystem::getPath("Source/shader-texture.vs").c_str(),FileSystem::getPath("Source/shader-texture.fs").c_str());
    Shader skyBoxShader(FileSystem::getPath("Source/skyBoxShader.vs").c_str(),FileSystem::getPath("Source/skyBoxShader.fs").c_str());
    Shader simpleShadow(FileSystem::getPath("Source/simple-shadow-shader.vs").c_str(),FileSystem::getPath("Source/simple-shadow-shader.fs").c_str());
    //    Shader bioShader(FileSystem::getPath("Source/bioshader.vs").c_str(),FileSystem::getPath("Source/bioshader.fs").c_str());
    
    
    //skybox VAO and VBO
    
    GLuint skyBoxVAO, skyBoxVBO;
    glGenVertexArrays(1, &skyBoxVAO);
    glGenBuffers(1, &skyBoxVBO);
    glBindVertexArray(skyBoxVBO);
    glBindBuffer(GL_ARRAY_BUFFER, skyBoxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    
    vector<std::string> faces
    {
        FileSystem::getPath("Xcode/skyBoxTextures/Daylight Box_Right.bmp"),
        FileSystem::getPath("Xcode/skyBoxTextures/Daylight Box_Left.bmp"),
        FileSystem::getPath("Xcode/skyBoxTextures/Daylight Box_Top.bmp"),
        FileSystem::getPath("Xcode/skyBoxTextures/Daylight Box_Bottom.bmp"),
        FileSystem::getPath("Xcode/skyBoxTextures/Daylight Box_Front.bmp"),
        FileSystem::getPath("Xcode/skyBoxTextures/Daylight Box_Back.bmp"),
    };
    
    
    
    GLuint cubemapTexture = loadCubemap(faces);
    skyBoxShader.use();
    //    glUseProgram(skyBoxShader);
    //    glUniform1i(glGetUniformLocation(skyBoxShader, "skybox"), 0);
    
    
    //load textures
    snowTextureID = TextureFromFile("snowtexture3.jpg", FileSystem::getPath("Xcode/Textures"));
    rockTextureID = TextureFromFile("rockyTexture.jpg", FileSystem::getPath("Xcode/Textures"));
    sandTextureID = TextureFromFile("sandyTexture.jpg", FileSystem::getPath("Xcode/Textures"));
    grassTextureID = TextureFromFile("grassTexture1.jpg", FileSystem::getPath("Xcode/Textures"));
    waterTextureID = TextureFromFile("waterTexture.jpg", FileSystem::getPath("Xcode/Textures"));
    
    
    
    // Other OpenGL states to set once
    // Enable Backface culling
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    //  glEnable(GL_FRAMEBUFFER_SRGB);
    
    // For frame time
    float lastFrameTime = glfwGetTime();
    double lastMousePosX, lastMousePosY;
    glfwGetCursorPos(window, &lastMousePosX, &lastMousePosY);
    
    //camera information for mouse implementation
    float cameraSpeed = 0.5f;
    float cameraFastSpeed = 2 * cameraSpeed;
    float cameraHorizontalAngle = -34.0f;
    float cameraVerticalAngle = 0.0f;
    
    
    
    //shadow depth map
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    
    
    // textures
    textureShader.use();
    textureShader.setInt("shadowMap", 0);
    textureShader.setInt("snowTexture", 1);
    textureShader.setInt("sandyTexture", 2);
    textureShader.setInt("rockyTexture", 3);
    textureShader.setInt("grassTexture", 4);
    textureShader.setInt("waterTexture", 5);
    
    
    // Create a test object:
    string test_path = "/Users/jeremygaudet/Documents/Xcode/COMP_371-Project/Xcode/Objects/lowpolytree/Lowpoly_tree_sample.obj";
    Model treeModel(test_path);
    
    
    
    
    
    
    
    
    
    
    
    
    
    // Entering Main While Loop
    // ------------------------
    while(!glfwWindowShouldClose(window))
    {
        
        if(!updateMap)
        {
            createMap(treeModel);
            updateMap = !updateMap;
        }
        
        
        
        
        float dt = glfwGetTime() - lastFrameTime;
        lastFrameTime += dt;
        bool fastCam = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
        float currentCameraSpeed = (fastCam) ? cameraFastSpeed : cameraSpeed;
        
        int mac_width, mac_height;
        glfwGetFramebufferSize(window, &mac_width, &mac_height);
        
        
        projectionMatrix = perspective(radians(fovAngle),1024.0f / 768.0f, 0.1f,300.0f);
        viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt,cameraUp );
        viewMatrix = viewMatrix * WorldTransformMatrix;
        
        //setting up the MVP of the world so I can place our objects within
        modelViewProjection = projectionMatrix * viewMatrix * modelMatrix;
        
        // set the background color to the greenish grey
        glClearColor(0.68f, 0.87f, 0.95f,1.0f);
        
        // clear the color and depth buffer at the beginning of each loop
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        
        
        
        //texture shader
        //        glUseProgram(textureShader);
        textureShader.use();
        
        //set view and projection materix in shader
        GLuint viewMatrix_texture = glGetUniformLocation(textureShader.ID, "view");
        glUniformMatrix4fv(viewMatrix_texture, 1, GL_FALSE, &viewMatrix[0][0]);
        
        GLuint projectionMatrix_texture = glGetUniformLocation(textureShader.ID, "projection");
        glUniformMatrix4fv(projectionMatrix_texture, 1, GL_FALSE, &projectionMatrix[0][0]);
        
        // do the same for the bioShader
        //        bioShader.use();
        //        bioShader.setMat4("projection", projectionMatrix);
        //        bioShader.setMat4("view", viewMatrix);
        
        
        textureShader.use();
        GLuint normalLook = glGetUniformLocation(textureShader.ID, "eyes");
        glUniform3f(normalLook, cameraPosition.x,cameraPosition.y,cameraPosition.z);
        
        
        
        
        // set light and view position
        GLuint lightPositionTexture = glGetUniformLocation(textureShader.ID, "lightPos");
        GLuint viewPositionTexture = glGetUniformLocation(textureShader.ID, "viewPos");
        glUniform3f(lightPositionTexture, lightpos.x,lightpos.y,lightpos.z);
        glUniform3f(viewPositionTexture, cameraPosition.x,cameraPosition.y,cameraPosition.z);
        
        //set lightColor for textureShader
        GLuint lightColor = glGetUniformLocation(textureShader.ID, "lightColor");
        glUniform3f(lightColor, 1.0f,1.0f,1.0f);
        
        mat4 lightProjection, lightView , lightSpaceMatrix;
        
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        
        
        //this part is largely inspired by learnopengl's shadow tutorial and lab 8
        //render shadows from lights perspective
        float near_plane = 1.0f, far_plane = 600.0f;
        
        
        lightProjection = perspective(radians(130.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane);
        
        
        lightView = lookAt(lightpos, vec3(0.0f), vec3(0.0, 0.0, 1.0));
        lightSpaceMatrix = lightProjection * lightView;
        // render scene from light's point of view
        simpleShadow.use();
        GLuint lightSpaceMatrixSimple = glGetUniformLocation(simpleShadow.ID, "lightSpaceMatrix");
        glUniformMatrix4fv(lightSpaceMatrixSimple, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
        
        renderTerrain(terrainVAOs, simpleShadow, nIndices, cameraPosition, treeModel);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // reset viewport
        glViewport(0, 0, mac_width, mac_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        
        glViewport(0, 0, mac_width, mac_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        //use textureshader to render the scene, and set the  boolean in the fragment shader to render wit shadows, if shadows aren't enabled then do it without
        textureShader.use();
        GLuint lightSpaceMatrixShader = glGetUniformLocation(textureShader.ID, "lightSpaceMatrix");
        glUniformMatrix4fv(lightSpaceMatrixShader, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
        
        
        //        glUniform1ui(glGetUniformLocation(textureShader, "textureOn"), 1);
        textureShader.setBool("textureOn", true);
        
        if (textureOn)
            textureShader.setBool("textureOn", true);
        else
            textureShader.setBool("textureOn", false);
        
        
        
        renderTerrain(terrainVAOs, textureShader, nIndices, cameraPosition, treeModel);
        
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyBoxShader.use();
        GLuint skyBoxViewMatrix = glGetUniformLocation(skyBoxShader.ID, "view");
        GLuint skyBoxProjectionMatrix = glGetUniformLocation(skyBoxShader.ID, "projection");
        mat4 view = mat4(mat3(viewMatrix));
        glUniformMatrix4fv(skyBoxViewMatrix, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(skyBoxProjectionMatrix, 1, GL_FALSE, &projectionMatrix[0][0]);
        
        // skybox cube
        glBindVertexArray(skyBoxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthMask(GL_TRUE);
        
        
        
        
        
        
        // End Frame
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        
        // this part here which controls the camera via the mouse X,Y inputs
        // it is edited and adapted from my solution to lab 4
        double mousePosX, mousePosY;
        glfwGetCursorPos(window, &mousePosX, &mousePosY);
        
        double dx = mousePosX - lastMousePosX;
        double dy = mousePosY - lastMousePosY;
        
        lastMousePosX = mousePosX;
        lastMousePosY = mousePosY;
        
        // Convert to spherical coordinates
        const float cameraAngularSpeed = 50.0f;
        cameraHorizontalAngle -= dx * cameraAngularSpeed * dt;
        cameraVerticalAngle   -= dy * cameraAngularSpeed * dt;
        
        
        cameraVerticalAngle = std::max(-90.0f, std::min(90.0f, cameraVerticalAngle));
        if (cameraHorizontalAngle > 360)
        {
            cameraHorizontalAngle -= 360;
        }
        else if (cameraHorizontalAngle < -360)
        {
            cameraHorizontalAngle += 360;
        }
        
        float theta = radians(cameraHorizontalAngle);
        float phi = radians(cameraVerticalAngle);
        
        cameraLookAt = vec3(cosf(phi)*cosf(theta), sinf(phi), -cosf(phi)*sinf(theta));
        vec3 cameraSideVector = cross(cameraLookAt, vec3(0.0f, 1.0f, 0.0f));
        normalize(cameraSideVector);
        
        
        
        //these are the following keybindings to control the olaf, the camera and the world orientation, textures, lighting and shadows
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // camera zoom in
        {
            cameraPosition.z -= currentCameraSpeed * dt*40;
            lightpos.z -= currentCameraSpeed * dt*40;
            
            
        }
        
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) // camera zoom out
        {
            cameraPosition.z += currentCameraSpeed * dt*40;
            lightpos.z += currentCameraSpeed * dt*40;
            
            
        }
        
        
        
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ) // move camera to the left
        {
            cameraPosition.x -= currentCameraSpeed * dt*40;
            lightpos.x -= currentCameraSpeed * dt*40;
            
            
        }
        
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) // move camera to the right
        {
            cameraPosition.x += currentCameraSpeed * dt*40;
            lightpos.x += currentCameraSpeed * dt*40;
            
        }
        
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) // camera zoom in
        {
            fovAngle = fovAngle  - 0.1f;
        }
        
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) // camera zoom out
        {
            fovAngle = fovAngle + 0.1f;
        }
        
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) // move camera down
        {
            cameraPosition.y -= currentCameraSpeed * dt*40;
            lightpos.y -= currentCameraSpeed * dt*40;
            
        }
        
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) // move camera up
        {
            cameraPosition.y += currentCameraSpeed * dt*40;
            lightpos.y += currentCameraSpeed * dt*40;
            
        }
        
        
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) // move camera up
        {
            float xTrans = 0.1;
            xTrans = xTrans + 0.001f;
            lightpos.x += xTrans;
        }
        
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) // move camera up
        {
            float xTrans = 0.1;
            xTrans = xTrans + 0.001f;
            lightpos.x -= xTrans;
        }
        
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) // move camera up
        {
            float yTrans = 0.1;
            yTrans = yTrans + 0.001f;
            lightpos.y += yTrans;
        }
        
        if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) // move camera up
        {
            float yTrans = 0.1;
            yTrans = yTrans + 0.001f;
            lightpos.y -= yTrans;
        }
        
        if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) // move camera up
        {
            
            float zTrans = 0.1;
            zTrans = zTrans + 0.001f;
            lightpos.z += zTrans ;
        }
        
        if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) // move camera up
        {
            float zTrans = 0.1;
            zTrans = zTrans + 0.001f;
            lightpos.z -= zTrans;
            
        }
        
        
        
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) // primative change to trianges
        {
            primativeRender = GL_TRIANGLES;
        }
        
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) // primative change to points
        {
            primativeRender = GL_POINTS;
        }
        
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) // primative change to lines
        {
            primativeRender = GL_LINES;
        }
        
        
        
        
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) // rotate X axis in the other orientation of the world
        {
            
            float x =5.01;
            float y=5.01;
            float z=5.01;
            x+=0.01;
            y+=0.01;
            z+=0.01;
            
            WorldTransformMatrix = WorldTransformMatrix * rotate(mat4(1.0f), radians(x),vec3(-1.0f,0.f, 0.f));
            
            
        }
        
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) // rotate X axis in the other orientation of the world
        {
            
            float x =5.01;
            float y=5.01;
            float z=5.01;
            x+=0.01;
            y+=0.01;
            z+=0.01;
            WorldTransformMatrix = WorldTransformMatrix * rotate(mat4(1.0f), radians(x),vec3(-1.0f,0.f, 0.f));
            
            
            
            
        }
        
        
        
        
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) //rotate Y axis of the world
        {
            float x =5.01;
            float y=5.01;
            float z=5.01;
            x+=0.01;
            y+=0.01;
            z+=0.01;
            WorldTransformMatrix = WorldTransformMatrix * rotate(mat4(1.0f), radians(x),vec3(0.0f,1.f, 0.f));
        }
        
        
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) // rotate Y axis of the world in the other orientation
        {
            float x =5.01;
            float y=5.01;
            float z=5.01;
            x+=0.01;
            y+=0.01;
            z+=0.01;
            WorldTransformMatrix = WorldTransformMatrix * rotate(mat4(1.0f), radians(x),vec3(0.0f,-1.0f, 0.f));
        }
        
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) // reset world orientation to original settings
        {
            WorldTransformMatrix = mat4(1.0f);
        }
        
        
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        
        
        viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp );
        projectionMatrix = perspective(radians(fovAngle), 1024.0f / 768.0f, 0.1f,600.0f);
        
        
        
    }
    
    
    // Shutdown GLFW
    glfwTerminate();
    
    return 0;
}



void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_X && action == GLFW_PRESS)
        textureOn = !textureOn;
    
    
    if (key == GLFW_KEY_I && action == GLFW_PRESS)
    {
        octaves += 1;
        updateMap = !updateMap;
    }
    
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
    {
        octaves -= 1;
        updateMap = !updateMap;
    }
    if (key == GLFW_KEY_B && action == GLFW_PRESS)
        textureOn = !textureOn;
    
    
    if (key == GLFW_KEY_J && action == GLFW_PRESS)
    {
        meshHeight += 1;
        updateMap = !updateMap;
    }
    
    if (key == GLFW_KEY_K && action == GLFW_PRESS)
    {
        meshHeight -= 1;
        updateMap = !updateMap;
    }
    
}




void createTerrainGeometry(GLuint &VAO, int &xOffset, int &yOffset, Model &treeModel)
{
    vector<float> vertices;
    vector <int> indices(6 * (mapY - 1) * (mapY - 1));
    vector<float> textureCoords;
    //    vector<float> bio;
    
    float amp  = 1;
    float freq = 1;
    
    //create vertices and noise
    float  rangedNoise =0;
    
    for (int y = 0; y < mapY ; y++) {
        for (int x = 0; x < mapX; x++) {
            vertices.push_back(x);
            textureCoords.push_back(x);
            amp  = 1;
            freq = 1;
            float noiseHeight = 0;
            for (int i = 0; i < octaves; i++) {
                float xSample = (x + xOffset * (mapX-1)) / noiseScale * freq;
                float ySample = (y + yOffset * (mapY-1)) / noiseScale * freq;
                
                float perlinValue = SimplexNoise::noise(xSample,ySample);
                noiseHeight += perlinValue * amp;
                
                amp  *= persistence;
                freq *= lacunarity;
            }
            rangedNoise = Remap(noiseHeight, -1.0, 1, 0, 1);
            
            
            vertices.push_back((rangedNoise * meshHeight));
            vertices.push_back(y);
            textureCoords.push_back(y);
        }
    }
    
    // generate a large list of semi-random model transformation matrices
    // ------------------------------------------------------------------
    unsigned int amount = mapX * mapY;
    glm::mat4 *modelMatrices;
    modelMatrices = new glm::mat4[5000];
    vector<float> vertices_copy = vertices;
    for ( int i = 0; i < 99; i++) {
        vertices_copy.insert(vertices_copy.end(), vertices.begin(), vertices.end());
    }
    
    int acounter = 0;
    for (int i = 0; i <yMapChunks; i++) {
        for (int j = 0; j < xMapChunks; j++) {
            for (int y = 0; y < mapY; y++) {
                for (int x = 0; x < mapX; x++) {
                    vertices_copy[acounter] = vertices_copy[acounter] + (-mapX / 2.0 + (mapX - 1) * j);
                    vertices_copy[acounter+2] = vertices_copy[acounter+2] + (-mapY / 2.0 + (mapY - 1) * i);
                    acounter += 3;
                }
            }
        }
    }
    
    int counter = 0;
    while (counter < 5000) {
        int c = (rand() % (mapY * mapX * 100)) * 3; // random number that is less than vertices_copy.size() [1,228,800] and divisible by 3
        float x = vertices_copy[c];
        float y = vertices_copy[c+1];
        float z = vertices_copy[c+2];
        
        if (y < 6.0 || y > 16.5)    // matches the grass numbers in the fragshader
            continue;
        
        mat4 model = mat4(1.0f);
        model = glm::translate(model, vec3(x, y, z));
        model = glm::scale(model, vec3(0.2f));
        modelMatrices[counter] = model;
        counter++;
    }
    
    
    
    // configure instanced array
    // -------------------------
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, (mapY * mapX) * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);
    
    
    // set transformation matrices as an instance vertex attribute (with divisor 1)
    // ----------------------------------------------------------------------------
    for (unsigned int i = 0; i < treeModel.meshes.size(); i++)
    {
        unsigned int VAO = treeModel.meshes[i].VAO;
        glBindVertexArray(VAO);
        // set attribute pointers for matrix (4 times vec4)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
        
        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        
        glBindVertexArray(0);
    }
    
    
    
    
    
    
    
    
    
    
    
    
    int pointer = 0;
    for(int y = 0; y < mapY - 1; y++)
    {
        for(int x = 0; x < mapX - 1; x++)
        {
            int topLeft = (y * mapY) + x;
            int topRight = topLeft + 1;
            int bottomLeft = ((y + 1) * mapY) + x;
            int bottomRight = bottomLeft + 1;
            indices[pointer++] = topLeft;
            indices[pointer++] = bottomLeft;
            indices[pointer++] = topRight;
            indices[pointer++] = topRight;
            indices[pointer++] = bottomLeft;
            indices[pointer++] = bottomRight;
        }
    }
    
    
    
    
    
    GLuint VBO[2], EBO;
    
    // VAO and VBO generateion
    glGenBuffers(2, VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    // terrain coordinates
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);
    
    // texture coordinates
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, textureCoords.size() * sizeof(float), &textureCoords[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    
}


void renderTerrain(vector <GLuint> &VAO, Shader &shader, int &nIndices, vec3 &cameraPosition, Model &testModel) {
    
    shader.use();
    GLuint modelViewProjection_terrain = glGetUniformLocation(shader.ID, "mvp");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    
    glActiveTexture(GL_TEXTURE0 +  1);
    glBindTexture(GL_TEXTURE_2D, snowTextureID);
    
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, sandTextureID);
    
    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D, rockTextureID);
    
    glActiveTexture(GL_TEXTURE0 + 4);
    glBindTexture(GL_TEXTURE_2D, grassTextureID);
    
    glActiveTexture(GL_TEXTURE0+ 5);
    glBindTexture(GL_TEXTURE_2D, waterTextureID);
    
    for (int y = 0; y < yMapChunks; y++)
        for (int x = 0; x < xMapChunks; x++) {
            
            shader.use();
            
            mat4 mvp = mat4(1.0f);
            mvp = translate(mvp, vec3(-mapX / 2.0 + (mapX - 1) * x, 0.0, -mapY / 2.0 + (mapY - 1) * y));
            glUniformMatrix4fv(modelViewProjection_terrain, 1, GL_FALSE, &mvp[0][0]);
            
            glBindVertexArray(VAO[x + y*xMapChunks]);
            glDrawElements(primativeRender, nIndices, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);   // reset
            
        }
    
    
    shader.setBool("instanceOn", true);
    if (testModel.textures_loaded.size() > 0)   // don't try to load a non-existant texture
        glBindTexture(GL_TEXTURE_2D, testModel.textures_loaded[0].id);
    
    int numberOfObjects = 5000;
    for (unsigned int i = 0; i < testModel.meshes.size(); i++)
    {
        glBindVertexArray(testModel.meshes.at(i).VAO);
        glDrawElementsInstanced(GL_TRIANGLES, testModel.meshes.at(i).indices.size(), GL_UNSIGNED_INT, 0, numberOfObjects);
        glBindVertexArray(0);
    }
    shader.setBool("instanceOn", false);
    
}

//helper function to map values to stay within a range
float Remap (float value, float from1, float to1, float from2, float to2)
{
    return (value - from1) / ((to1 - from1) * (to2 - from2) + from2);
}


void createMap(Model &treeModel)
{
    for (int y = 0; y < yMapChunks; y++)
    {
        for (int x = 0; x < xMapChunks; x++)
        {
            createTerrainGeometry(terrainVAOs[x + y*xMapChunks], x, y, treeModel);
        }
    }
}




