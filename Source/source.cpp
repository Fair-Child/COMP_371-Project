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

//-- these links are what i used for help understanding heightmap and noise tiling (making the noise link up at the edges!)
// -https://pdfs.semanticscholar.org/5e95/c1c36a07a2919c3da123f17f9d408a1ea6a5.pdf
// - http://libnoise.sourceforge.net/noisegen/index.html
// - DAVID WOLF openGL 4.0 shading language cookbook page 269-270
// - chunk mapping https://en.wikibooks.org/wiki/OpenGL_Programming/Glescraft_1
// - thinmatrix help https://www.youtube.com/watch?v=qChQrNWU9Xw&list=PLRIWtICgwaX0u7Rf9zkZhLoLuZVfUksDP&index=37

//SimeplexNoise is a library https://github.com/SRombauts/SimplexNoise
//It  was created by Sébastien Rombauts



#include <iostream>
#include <list>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"



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


//bools
bool textureOn = true;
bool GUICONTROL = true;
bool flatOn = false;
bool updateMap =false;
bool show_demo_window = true;
bool cameraFirstPerson = true;

int TerrainMode = 0;



vec3 lightpos (149.0f, 38.0f,151.0f);


int counter =0;

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


vector<GLuint> VAO(100);

//params to start
const int xMapChunks = 1;
const int zMapChunks = 1;;
const int mapX = 1024;
const int mapZ = 1024;
int nIndices = mapX * mapZ * 6;
float heightPos [mapX*xMapChunks][mapZ*zMapChunks];

//camera info
//vec3 cameraPosition(0.0f,43.0f,30.0f);
vec3 cameraPosition(0.0f,43.0f,0.0f);

//vec3 cameraLookAt(0.0f, 0.0f, 0.0f);
vec3 cameraLookAt (0.810638f,-0.188706f, 0.554307f);

vec3 cameraUp(0.0f, 1.0f, 0.0f);

//primatative rendering options
int primativeRender = GL_TRIANGLES;



//functions
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void renderLight(const GLuint &lamp_Shader);
float Remap (float value, float from1, float to1, float from2, float to2);
void createMap(Model &model);
void renderTerrain(vector <GLuint> &VAO , Shader &shader,  int &nIndices,vec3 &cameraPosition, Model &model);
void createTerrianGeometry(GLuint &VAO, int &xOffset, int &zOffset, Model &model);
float getHeight(float x, float z);


// model
const int number_of_trees = 500;    // FIXME: This cannot go higher than 500! Don't know why!


//noise options
int octaves = 5;
float meshHeight = 25;  // Vertical scaling
float noiseScale = 64;  // Horizontal scaling
float persistence = 0.5;
float lacunarity = 2;
float xTrans = 0;

GLuint VBO[2], EBO;

float barryCentric(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 pos);




int main(int argc, char*argv[])
{
    
    //random number initialization
    srand( static_cast<unsigned int>(time(nullptr)));
    
    float  fovAngle = 45.0f;
    
    
    
    
    // Initialize GLFW and OpenGL version
    glfwInit();
    
#if defined(PLATFORM_OSX)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    const char* glsl_version = "#version 330";
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
    
    //    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
    Shader skyBoxShader(FileSystem::getPath("Source/skyBoxShader.vs").c_str(), FileSystem::getPath("Source/skyBoxShader.fs").c_str());
    //shader for simple shadows
    Shader simpleShadow(FileSystem::getPath("Source/simple-shadow-shader.vs").c_str(),FileSystem::getPath("Source/simple-shadow-shader.fs").c_str());
    
    //this allows us to send an out from the vertex shader to our feedback buffer
    const GLchar* feedbackVaryings[] = { "calPos" };
    glTransformFeedbackVaryings(textureShader.ID, 1, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
    
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
    glUniform1i(glGetUniformLocation(skyBoxShader.ID, "skybox"), 0);
    
    
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
    glUniform1i(glGetUniformLocation(textureShader.ID, "shadowMap"), 0);
    glUniform1i(glGetUniformLocation(textureShader.ID, "snowTexture"), 1);
    glUniform1i(glGetUniformLocation(textureShader.ID, "sandyTexture"), 2);
    glUniform1i(glGetUniformLocation(textureShader.ID, "rockyTexture"), 3);
    glUniform1i(glGetUniformLocation(textureShader.ID, "grassTexture"), 4);
    glUniform1i(glGetUniformLocation(textureShader.ID, "waterTexture"), 5);
    
    
    // create the tree model
    string tree_path = FileSystem::getPath("Xcode/obj/Tree_obj/tree.obj");
    Model poly_tree(tree_path);
    
    
    //create map but for ask for input variables for noise
    createMap(poly_tree);
    
    //this checks for the max height so it adjusts our collision detection
    
    
    
    
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
    
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    
    
    
    
    
    
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    
    
    
    
    // Entering Main Loop
    while(!glfwWindowShouldClose(window))
    {
        
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        if(updateMap) {
            createMap(poly_tree);
            updateMap = !updateMap;
            
        }
        
        
        
        
        
        
        
        float dt = glfwGetTime() - lastFrameTime;
        lastFrameTime += dt;
        bool fastCam = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
        float currentCameraSpeed = (fastCam) ? cameraFastSpeed : cameraSpeed;
        
        int mac_width, mac_height;
        glfwGetFramebufferSize(window, &mac_width, &mac_height);
        
        
        //        projectionMatrix = perspective(radians(fovAngle),1024.0f / 768.0f, 0.1f,300.0f);
        //        viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt,cameraUp );
        //        viewMatrix = viewMatrix * WorldTransformMatrix;
        
        //setting up the MVP of the world so I can place our objects within
        modelViewProjection = projectionMatrix * viewMatrix * modelMatrix;
        
        // set the background color to the greenish grey
        glClearColor(0.68f, 0.87f, 0.95f,1.0f);
        
        // clear the color and depth buffer at the beginning of each loop
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        
        
        
        //texture shader
        textureShader.use();
        
        //set view and projection materix in shader
        GLuint viewMatrix_texture = glGetUniformLocation(textureShader.ID, "view");
        GLuint projectionMatrix_texture = glGetUniformLocation(textureShader.ID, "projection");
        GLuint normalLook = glGetUniformLocation(textureShader.ID, "eyes");
        glUniform3f(normalLook, cameraPosition.x,cameraPosition.y,cameraPosition.z);
        
        
        //        glUniformMatrix4fv(viewMatrix_texture, 1, GL_FALSE, &viewMatrix[0][0]);
        //        glUniformMatrix4fv(projectionMatrix_texture, 1, GL_FALSE, &projectionMatrix[0][0]);
        
        
        
        //            //set light and view position
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
        
        
        lightProjection = glm::perspective(glm::radians(130.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane);
        
        
        lightView = glm::lookAt(lightpos, glm::vec3(0.0f), glm::vec3(0.0, 0.0, 1.0));
        lightSpaceMatrix = lightProjection * lightView;
        // render scene from light's point of view
        simpleShadow.use();
        GLuint lightSpaceMatrixSimple = glGetUniformLocation(simpleShadow.ID, "lightSpaceMatrix");
        glUniformMatrix4fv(lightSpaceMatrixSimple, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
        
        renderTerrain(VAO, simpleShadow, nIndices, cameraPosition, poly_tree);
        
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
        
        
        //sets textures and flat shading on and off
        
        glUniform1ui(glGetUniformLocation(textureShader.ID, "textureOn"), 1);
        
        if(textureOn) {
            glUniform1ui(glGetUniformLocation(textureShader.ID, "textureOn"), 1);
        } else {
            glUniform1ui(glGetUniformLocation(textureShader.ID, "textureOn"), 0);
        }
        
        
        glUniform1ui(glGetUniformLocation(textureShader.ID, "flatOn"), 0);
        if(flatOn) {
            glUniform1ui(glGetUniformLocation(textureShader.ID, "flatOn"), 1);
        } else{
            glUniform1ui(glGetUniformLocation(textureShader.ID, "flatOn"), 0);
        }
        
        
        
        glUniform1f(glGetUniformLocation(textureShader.ID, "newY"), xTrans);
        renderTerrain(VAO,textureShader, nIndices, cameraPosition, poly_tree);
        
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
        
        
        
        if (show_demo_window)
            
            
            // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            
            
            
            ImGui::Begin("World Control");                          // Create a window called "Hello, world!" and append into it.
            
            ImGui::Text("World Control Start Up.");                         // Display some text (you can use a format strings too)
            ImGui::Text("Please set parameters");
            ImGui::Text("press TAB when finished");
            
            ImGui::SliderFloat("Terrian Height Control", &xTrans, -0.99f, .199f);
            ImGui::Text("Sunlight Positon ");
            ImGui::SliderFloat("X", &lightpos.x, 0.0f, 200.f);
            ImGui::SliderFloat("Y", &lightpos.y,0.0f, 200.f);
            ImGui::SliderFloat("Z", &lightpos.z, 0.0f, 200.0f);
            
            
            if (ImGui::Button("Jagged Mode"))   {                         // Buttons return true when clicked (most widgets return true when edited/activated)
                TerrainMode =0;
                updateMap =!updateMap;
            }
            if (ImGui::Button("Smooth Mode"))   {                         // Buttons return true when clicked (most widgets return true when edited/activated)
                TerrainMode =1;
                updateMap =!updateMap;
            }
            if (ImGui::Button("Block Mode"))   {                         // Buttons return true when clicked (most widgets return true when edited/activated)
                TerrainMode =2;
                updateMap =!updateMap;
            }
            
            if (ImGui::Button("Flat Shading Mode"))   {                         // Buttons return true when clicked (most widgets return true when edited/activated)
                flatOn=!flatOn;
                
            }
            
            if (ImGui::Button("Textures On/Off"))   {                         // Buttons return true when clicked (most widgets return true when edited/activated)
                textureOn=!textureOn;
                
            }
            
            
            
            
            
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        // End Frame
        
        float heights =  getHeight(cameraPosition.x,cameraPosition.z);
        
        //        cout<<"cam.x:"<<cameraPosition.x<<" cam.y:"<<cameraPosition.y<<" cam.z:"<<cameraPosition.z<<endl;
        
        
        if(cameraPosition.y  + xTrans< (heights +5.99 )) {
            cameraPosition.y = (heights  +5.99) +xTrans;
            
        }
        
        
        
        double mousePosX, mousePosY;
        glfwGetCursorPos(window, &mousePosX, &mousePosY);
        
        double dx = mousePosX - lastMousePosX;
        double dy = mousePosY - lastMousePosY;
        
        lastMousePosX = mousePosX;
        lastMousePosY = mousePosY;
        
        // Convert to spherical coordinates
        
        const float cameraAngularSpeed = 50.0f;
        
        
        if(!GUICONTROL){
            cameraHorizontalAngle -= dx * cameraAngularSpeed * dt;
            cameraVerticalAngle   -= dy * cameraAngularSpeed * dt;
            
            
            cameraVerticalAngle = std::max(-85.0f, std::min(85.0f, cameraVerticalAngle));
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
            
            if (cameraFirstPerson)
            {
                viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp );
            }
            else
            {
                // Position of the camera is on the sphere looking at the point of interest (cameraPosition)
                float radius = 5.0f;
                vec3 position = cameraPosition - vec3(radius * cosf(phi)*cosf(theta),
                                                      radius * sinf(phi),
                                                      -radius * cosf(phi)*sinf(theta));
                
                viewMatrix = lookAt(position, cameraPosition, cameraUp);
            }
            
            
            
            
        } else {
            float theta = radians(cameraHorizontalAngle);
            float phi = radians(cameraVerticalAngle);
            if (cameraFirstPerson)
            {
                viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp );
            }
            else
            {
                // Position of the camera is on the sphere looking at the point of interest (cameraPosition)
                float radius = 5.0f;
                vec3 position = cameraPosition - vec3(radius * cosf(phi)*cosf(theta),
                                                      radius * sinf(phi),
                                                      -radius * cosf(phi)*sinf(theta));
                
                viewMatrix = lookAt(position, cameraPosition, cameraUp);
            }
            
            
        }
        
        
        //these are the following keybindings to control the olaf, the camera and the world orientation, textures, lighting and shadows
        
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) // camera zoom in
        {
            cameraPosition.z -= currentCameraSpeed * dt*40;
            lightpos.z -= currentCameraSpeed * dt*40;
            
            
        }
        
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) // camera zoom out
        {
            cameraPosition.z += currentCameraSpeed * dt*40;
            lightpos.z += currentCameraSpeed * dt*40;
            
            
        }
        
        
        
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ) // move camera to the left
        {
            cameraPosition.x -= currentCameraSpeed * dt*40;
            lightpos.x -= currentCameraSpeed * dt*40;
            
            
        }
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // move camera to the right
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
        
        
        
        
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) //rotate Y axis of the world
        {
            
            xTrans = xTrans + 0.001f;
            if(xTrans >= 0.199) {
                xTrans = 0.199;
            }
            
            
        }
        
        
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) // rotate Y axis of the world in the other orientation
        {
            
            xTrans = xTrans - 0.001f;
            
            if(xTrans <= -0.99) {
                xTrans = -0.99;
            }
            
        }
        
        
        
        
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        
        
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) // move camera down
        {
            cameraFirstPerson = true;
        }
        
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) // move camera down
        {
            cameraFirstPerson = false;
        }
        
        
        
        textureShader.use();
        
        projectionMatrix = perspective(radians(fovAngle),1024.0f / 768.0f, 0.1f,600.0f);
        
        glUniformMatrix4fv(viewMatrix_texture, 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(projectionMatrix_texture, 1, GL_FALSE, &projectionMatrix[0][0]);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        
    }
    
    
    
    
    // Shutdown GLFW
    glfwTerminate();
    
    return 0;
}



void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    
    
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        flatOn = !flatOn;
    
    
    if (key == GLFW_KEY_B && action == GLFW_PRESS)
        textureOn = !textureOn;
    
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS){
        if(!GUICONTROL){
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            
        }else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        GUICONTROL =!GUICONTROL;
        show_demo_window = !show_demo_window;
    }
    
}





void createTerrainGeometry(GLuint &VAO, int &xOffset, int &zOffset, Model& object_model) {
    vector<float> vertices;
    vector <int> indices(6 * (mapZ - 1) * (mapZ - 1));
    vector<float> textureCoords;
    
    
    
    float xSample =0;
    float zSample = 0;
    float amp  = 1;
    float freq = 1;
    
    
    //create vertices and noise
    float  rangedNoise =0;
    
    
    
    for (int z = 0; z < mapZ ; z++)
        for (int x = 0; x < mapX; x++) {
            vertices.push_back(x);
            textureCoords.push_back(x);
            amp  = 1;
            freq = 1;
            float noiseHeight = 0;
            for (int i = 0; i < octaves; i++) {
                
                
                //jagged
                if(TerrainMode ==0){
                    xSample = (xOffset * (mapX-1) + x-1)  / noiseScale * freq;
                    zSample = (zOffset * (mapZ-1) + z-1) / noiseScale * freq;
                }
                
                //smooth
                if(TerrainMode ==1) {
                    xSample = (xOffset * (mapX-1) + x-1)  / noiseScale;
                    zSample = (zOffset * (mapZ-1) + z-1) / noiseScale;
                }
                
                //block
                if(TerrainMode ==2) {
                    xSample = (xOffset * (mapX-1) + x-1)  / 32;
                    zSample = (zOffset * (mapZ-1) + z-1) / 32;
                }
                
                
                
                float perlinValue = SimplexNoise::noise(xSample,zSample);
                noiseHeight += perlinValue * amp;
                
                
                amp  *= persistence;
                freq *= lacunarity;
                
                
                
                
            }
            rangedNoise = Remap(noiseHeight, -1.0, 1, 0, 1);
            
            vertices.push_back(rangedNoise * meshHeight);
            vertices.push_back(z);
            textureCoords.push_back(z);
            
            float calc = rangedNoise * meshHeight;
            
            
            
            
            heightPos[z][x]=calc;
            
            
        }
    
    // generate a large list of semi-random model transformation matrices
    // ------------------------------------------------------------------
    //
    //    // we only want to do this once!
    if (VAO == 0) {
        glm::mat4 *modelMatrices;
        modelMatrices = new glm::mat4[number_of_trees];
        vector<float> vertices_copy = vertices;
        for ( int i = 0; i < 99; i++) {
            vertices_copy.insert(vertices_copy.end(), vertices.begin(), vertices.end());
        }
        
        int counter1 = 0;
        for (int i = 0; i <zMapChunks; i++) {
            for (int j = 0; j < xMapChunks; j++) {
                for (int y = 0; y < mapZ; y++) {
                    for (int x = 0; x < mapX; x++) {
                        vertices_copy[counter1] = vertices_copy[counter1] + ((mapX - 1) * j);
                        vertices_copy[counter1+2] = vertices_copy[counter1+2] + ((mapZ - 1) * i);
                        counter1 += 3;
                    }
                }
            }
        }
        //
        int counter2 = 0;
        while (counter2 < number_of_trees) {
            int c = (rand() % (mapZ * mapX )) * 3; // random number that is less than vertices_copy.size() [4,915,200] and divisible by 3
            float x = vertices_copy[c];
            float y = vertices_copy[c+1];
            float z = vertices_copy[c+2];
            
            
            if (y > 1 && y < 16 )   { // matches the grass numbers in the fragshader
                //
                
                mat4 model = mat4(1.0f);
                model = glm::translate(model, vec3(x, y, z));
                model = glm::scale(model, vec3(0.9f));
                modelMatrices[counter2] = model;
                counter2++;
            }
        }
        //
        //
        //
        //
        //        // configure instanced array
        //        // -------------------------
        unsigned int buffer;
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, (number_of_trees) * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);
        // FIXME: ^ I don't know why, but if "number_of_trees" is too big (ie. greater than 500 or so), this ^^ breaks!
        
        //
        //        // set transformation matrices as an instance vertex attribute (with divisor 1)
        //        // ----------------------------------------------------------------------------
        for (unsigned int i = 0; i < object_model.meshes.size(); i++)
        {
            unsigned int VAO = object_model.meshes[i].VAO;
            glBindVertexArray(VAO);
            // set attribute pointers for matrix (4 times vec4)
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
            glEnableVertexAttribArray(5);
            glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
            glEnableVertexAttribArray(6);
            glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
            glEnableVertexAttribArray(7);
            glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
            
            glVertexAttribDivisor(4, 1);
            glVertexAttribDivisor(5, 1);
            glVertexAttribDivisor(6, 1);
            glVertexAttribDivisor(7, 1);
            
            glBindVertexArray(0);
        }
    }
    
    
    
    
    //calculate indices
    int pointer = 0;
    for(int z = 0; z < mapZ - 1; z++)
    {
        for(int x = 0; x < mapX - 1; x++)
        {
            int topLeft = (z * mapZ) + x;
            int topRight = topLeft + 1;
            int bottomLeft = ((z + 1) * mapZ) + x;
            int bottomRight = bottomLeft + 1;
            indices[pointer++] = topLeft;
            indices[pointer++] = bottomLeft;
            indices[pointer++] = topRight;
            indices[pointer++] = topRight;
            indices[pointer++] = bottomLeft;
            indices[pointer++] = bottomRight;
        }
    }
    
    
    
    
    
    // VAO and VBO generateion
    glGenBuffers(2, VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    // terrian coordinates
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    
    // EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);
    
    
    
    
    //texture coordinates
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, textureCoords.size() * sizeof(float), &textureCoords[0], GL_STATIC_DRAW);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    
    
    
    
}




void renderTerrain(vector <GLuint> &VAO, Shader &shader, int &nIndices, vec3 &cameraPosition, Model &object_model) {
    
    
    
    shader.use();
    GLuint modelViewProjection_terrain = glGetUniformLocation(shader.ID, "mvp");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    
    glActiveTexture(GL_TEXTURE0 +  1);
    glBindTexture(GL_TEXTURE_2D, snowTextureID);
    
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, sandTextureID);
    //
    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D, rockTextureID);
    
    glActiveTexture(GL_TEXTURE0 + 4);
    glBindTexture(GL_TEXTURE_2D, grassTextureID);
    //
    glActiveTexture(GL_TEXTURE0+ 5);
    glBindTexture(GL_TEXTURE_2D, waterTextureID);
    
    
    
    for (int z = 0; z < 3; z++) {
        for (int x = 0; x < 3; x++) {
            
            
            mat4 mvp = glm::mat4(1.0f);
            mvp = translate(mvp, vec3( (mapX - 1) * x, 0.0, (mapZ - 1) * z));
            
            glUniformMatrix4fv(modelViewProjection_terrain, 1, GL_FALSE, &mvp[0][0]);
            
            glBindVertexArray(VAO[x + z*xMapChunks]);
            glDrawElements(primativeRender, nIndices, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);   // reset
            
            shader.setInt("treeColor", 1);
            mat4 tree_mvp = glm::mat4(1.0f);
            mat4 scaleSize = scale(tree_mvp, vec3(0.7f));
            
            
            
            float y = getHeight( x+36,  z+13);
            mat4 treeTranslate =translate(tree_mvp, vec3( (mapX - 1) * x +36, 0.0, (mapZ - 1) * z+13));
            tree_mvp = treeTranslate * scaleSize;
            
            if(y>4) {
                
                shader.setMat4("mvp", tree_mvp);
                object_model.Draw(shader);
                
                shader.setInt("treeColor", 0);
            }
            
            
        }
    }
    
    shader.setBool("instanceOn", true);
    
    shader.setInt("treeColor", 1);
    for (unsigned int i = 0; i < object_model.meshes.size(); i++)
    {
        
        glBindVertexArray(object_model.meshes.at(i).VAO);
        glDrawElementsInstanced(GL_TRIANGLES, object_model.meshes.at(i).indices.size(), GL_UNSIGNED_INT, 0, number_of_trees);
        glBindVertexArray(0);
        
    }
    
    shader.setInt("treeColor", 0);
    
    shader.setBool("instanceOn", false);
    
}

//helper function to map values to stay within a range
float Remap (float value, float from1, float to1, float from2, float to2)
{
    return (value - from1) / ((to1 - from1) * (to2 - from2) + from2);
}


void createMap(Model &model) {
    
    for (int z = 0; z < zMapChunks; z++){
        for (int x = 0; x < xMapChunks; x++) {
            createTerrainGeometry(VAO[x + z*xMapChunks], x, z, model);
            
        }
    }
}






float barryCentric(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 pos)
{
    float det = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
    float l1 = ((p2.z - p3.z) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.y - p3.z)) / det;
    float l2 = ((p3.z - p1.z) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.y - p3.z)) / det;
    float l3 = 1.0f - l1 - l2;
    return l1 * p1.y + l2 * p2.y + l3 * p3.y;
}


float getHeight(float x, float z) {
    
    float terrainX = x -1;
    float terrainZ = z-1;
    
    
    
    
    int height = mapX*xMapChunks;
    
    int gridSize = (mapX*xMapChunks) / height;
    
    
    int gridX = floor(terrainX/gridSize);
    int gridZ = floor(terrainZ / gridSize);
    
    
    
    
    if(gridX >= (height-1)|| gridZ >= (height-1)|| gridX < 0 || gridZ <0) {
        return 0;
    }
    
    int xCoordMod =  (int)terrainX % mapZ;
    int zCoordMod = (int)terrainZ % mapZ;
    
    
    float xCoord = (float)xCoordMod/(float)mapX;
    float zCoord = (float)zCoordMod/(float)mapZ;
    
    
    
    
    float answer;
    if(xCoord <= (1-zCoord)){
        answer = barryCentric(glm::vec3(0, heightPos[gridZ][gridX], 0),
                              glm::vec3(1, heightPos[gridZ+1][gridX], 0), glm::vec3(0, heightPos[gridZ][gridX + 1], 1), glm::vec2(xCoord, zCoord));
        
    }else {
        
        answer = barryCentric(glm::vec3(1, heightPos[gridZ+1][gridX], 0),
                              glm::vec3(1, heightPos[gridZ+1][gridX+1], 1), glm::vec3(0, heightPos[gridZ][gridX + 1], 1), glm::vec2(xCoord, zCoord));
        
        
        
    }
    
    
    
    
    
    return answer;
    
    
    
    
}





