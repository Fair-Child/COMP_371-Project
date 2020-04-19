// COMP-371 Group Project Team 15
// Procedural World Generation
// Matthew Salaciak 29644490
// Jeremy Gaudet 40045224
//  Elsa Donovan 26857655

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
#include <unordered_map>





//define namespaces for glm and c++ std
using namespace glm;
using namespace std;

//global variables and functions for the project


//bools to control key / gui toggles
bool textureOn = true;
bool GUICONTROL = true;
bool flatOn = false;
bool updateMap =false;
bool show_demo_window = true;
bool cameraFirstPerson = true;
bool changeHeight = false;

// used to determine which terrainmode it is using for world generation
int TerrainMode = 0;


//light position
vec3 lightpos (-25.0f, 12.0f,108.0f);






//counter
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

//array of VAO's
vector<GLuint> VAO(100);

//Procedural World Paramters (only adjust mapX and mapZ which control the size)
const int xMapChunks = 1;
const int zMapChunks = 1;;
const int mapX = 512;
const int mapZ = 512;
int nIndices = mapX * mapZ * 6;
float heightPos [mapX*xMapChunks][mapZ*zMapChunks];
float heightPosTrees [mapX*xMapChunks][mapZ*zMapChunks];

//camera info
vec3 cameraPosition(0.0f,43.0f,0.0f);
vec3 cameraLookAt (0.810638f,-0.188706f, 0.554307f);
vec3 cameraUp(0.0f, 1.0f, 0.0f);
float  fovAngle = 45.0f;

//primatative rendering options
int primativeRender = GL_TRIANGLES;



//functions
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void renderLight(const GLuint &lamp_Shader);
float Remap (float value, float from1, float to1, float from2, float to2);
void createMap(Model &model);
void renderTerrain(vector <GLuint> &VAO , Shader &shader,  int &nIndices,vec3 &cameraPosition, Model &model);
void createTerrainGeometry(GLuint &VAO, int &xOffset, int &zOffset, Model &model);
float getHeight(float x, float z);
float getHeightTrees(float x, float z);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processMouseScroll(float yoffset);
float barryCentric(vec3 p1, vec3 p2, vec3 p3, vec2 pos);


// # of tree objects
const int number_of_trees = 1000;


//noise paramteres
int octaves = 5;
float meshHeight = 25;
float noiseScale = 64;
float persistence = 0.5;
float lacunarity = 2;
float xTrans = 0;

//vbo and ebo
GLuint VBO[2], EBO;



int main(int argc, char*argv[])
{
    
    //random number initialization
    srand( static_cast<unsigned int>(time(nullptr)));
    
    
    
    
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
    GLFWwindow* window = glfwCreateWindow(1024, 768, "TEAM_15_PROJECT", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    
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
    
    
    
    // load shaders
    Shader textureShader(FileSystem::getPath("Source/shader-texture.vs").c_str(),FileSystem::getPath("Source/shader-texture.fs").c_str());
    Shader skyBoxShader(FileSystem::getPath("Source/skyBoxShader.vs").c_str(), FileSystem::getPath("Source/skyBoxShader.fs").c_str());
    //shader for simple shadows
    Shader simpleShadow(FileSystem::getPath("Source/simple-shadow-shader.vs").c_str(),FileSystem::getPath("Source/simple-shadow-shader.fs").c_str());
    
    
    
    //skybox VAO and VBO
    
    GLuint skyBoxVAO, skyBoxVBO;
    glGenVertexArrays(1, &skyBoxVAO);
    glGenBuffers(1, &skyBoxVBO);
    glBindVertexArray(skyBoxVBO);
    glBindBuffer(GL_ARRAY_BUFFER, skyBoxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    //skybox cube mape faces
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
    //          glEnable(GL_FRAMEBUFFER_SRGB);
    
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
    glUniform1i(glGetUniformLocation(textureShader.ID, "shadowMap"), 12);
    glUniform1i(glGetUniformLocation(textureShader.ID, "snowTexture"), 1);
    glUniform1i(glGetUniformLocation(textureShader.ID, "sandyTexture"), 2);
    glUniform1i(glGetUniformLocation(textureShader.ID, "rockyTexture"), 3);
    glUniform1i(glGetUniformLocation(textureShader.ID, "grassTexture"), 4);
    glUniform1i(glGetUniformLocation(textureShader.ID, "waterTexture"), 5);
    
    
    // create the tree model
    string tree_path = FileSystem::getPath("Xcode/obj/lowpolytree/Lowpoly_tree_sample.obj");
    Model poly_tree(tree_path);
    
    
    
    
    
    
    //create map
    createMap(poly_tree);
    
    
    //imGUI setup Config
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    float near_plane = 1.0f, far_plane = 300.0f;
    
    // Entering Main Loop
    // ------------------
    while(!glfwWindowShouldClose(window))
    {
        
        //imGUI set up
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        //checks to see if the updateMap was called, if so it then updates map.
        if(updateMap)
        {
            createMap(poly_tree);
            updateMap = !updateMap;
        }
        
        float dt = glfwGetTime() - lastFrameTime;
        lastFrameTime += dt;
        bool fastCam = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
        float currentCameraSpeed = (fastCam) ? cameraFastSpeed : cameraSpeed;
        
        int mac_width, mac_height;
        glfwGetFramebufferSize(window, &mac_width, &mac_height);
        
        
        //setting up the MVP of the world so I can place our objects within
        modelViewProjection = projectionMatrix * viewMatrix * modelMatrix;
        
        // set the background color to the greenish grey
        glClearColor(0.68f, 0.87f, 0.95f,1.0f);
        
        // clear the color and depth buffer at the beginning of each loop
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        
        // texture shader
        textureShader.use();
        
        // set view and projection materix in shader
        GLuint viewMatrix_texture = glGetUniformLocation(textureShader.ID, "view");
        GLuint projectionMatrix_texture = glGetUniformLocation(textureShader.ID, "projection");
        GLuint normalLook = glGetUniformLocation(textureShader.ID, "eyes");
        glUniform3f(normalLook, cameraPosition.x,cameraPosition.y,cameraPosition.z);
        
        
        // set light and view position
        GLuint lightPositionTexture = glGetUniformLocation(textureShader.ID, "lightPos");
        GLuint viewPositionTexture = glGetUniformLocation(textureShader.ID, "viewPos");
        glUniform3f(lightPositionTexture, lightpos.x,lightpos.y,lightpos.z);
        glUniform3f(viewPositionTexture, cameraPosition.x,cameraPosition.y,cameraPosition.z);
        
        // set lightColor for textureShader
        GLuint lightColor = glGetUniformLocation(textureShader.ID, "lightColor");
        glUniform3f(lightColor, 1.0f,1.0f,1.0f);
        
        mat4 lightProjection, lightView , lightSpaceMatrix;
        
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        
        
        // this part is largely inspired by learnopengl's shadow tutorial and lab 8
        // render shadows from lights perspective
        
        
        
        lightProjection = perspective(radians(130.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane);
        
        
        lightView = glm::lookAt(lightpos, vec3(0.0f), glm::vec3(0.0, 0.0, 1.0));
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
        
        // use textureshader to render the scene, and set the  boolean in the fragment shader to render wit shadows, if shadows aren't enabled then do it without
        textureShader.use();
        GLuint lightSpaceMatrixShader = glGetUniformLocation(textureShader.ID, "lightSpaceMatrix");
        glUniformMatrix4fv(lightSpaceMatrixShader, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
        
        
        // sets textures and flat shading on and off
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
        mat4 rotateSkyBox = glm::rotate(mat4(1.0f), 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        mat4 view = mat4(mat3(viewMatrix));
        view = view *rotateSkyBox;
        glUniformMatrix4fv(skyBoxViewMatrix, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(skyBoxProjectionMatrix, 1, GL_FALSE, &projectionMatrix[0][0]);
        
        // skybox cube
        glBindVertexArray(skyBoxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthMask(GL_TRUE);
        
        
        //imGUI implementation adapted from their openGL example
        if (show_demo_window)
            
        {
            
            ImGui::Begin("World Control");
            
            ImGui::Text("World Control Start Up.");
            ImGui::Text("Please set parameters");
            ImGui::Text("press TAB when finished");
            
            ImGui::SliderFloat("Terrian Height Control", &xTrans, -0.99f, .199f);
            ImGui::Text("Sunlight Positon ");
            ImGui::SliderFloat("X", &lightpos.x, -300.0f, 300.f);
            ImGui::SliderFloat("Y", &lightpos.y,-300.0f, 300.f);
            ImGui::SliderFloat("Z", &lightpos.z, -300.0f, 300.0f);
            
            
            if (ImGui::Button("Jagged Mode"))   {
                TerrainMode =0;
                updateMap =!updateMap;
            }
            if (ImGui::Button("Smooth Mode"))   {
                TerrainMode =1;
                updateMap =!updateMap;
            }
            if (ImGui::Button("Block Mode"))   {
                TerrainMode =2;
                updateMap =!updateMap;
            }
            
            if (ImGui::Button("Flat Shading Mode"))   {
                flatOn=!flatOn;
                
            }
            
            if (ImGui::Button("Textures On/Off"))   {
                textureOn=!textureOn;
                
            }
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        
        //get the heights from the 2d height position array
        
        float heights =  getHeight(cameraPosition.x,cameraPosition.z);
        float treeHeights = getHeightTrees(cameraPosition.x, cameraPosition.z);
        
        //collision detection for camera and terrain
        
        if(cameraPosition.y  + xTrans< (heights +5.99 )) {
            cameraPosition.y = (heights  +5.99) +xTrans;
            
        }
        
        //collision detection for camera and tree objs
        if(cameraPosition.y  + xTrans< (treeHeights +7.99 )) {
            cameraPosition.y = (treeHeights  +7.99) +xTrans;
            
        }
        
        
        
        double mousePosX, mousePosY;
        glfwGetCursorPos(window, &mousePosX, &mousePosY);
        
        double dx = mousePosX - lastMousePosX;
        double dy = mousePosY - lastMousePosY;
        
        lastMousePosX = mousePosX;
        lastMousePosY = mousePosY;
        
        // Convert to spherical coordinates
        
        const float cameraAngularSpeed = 10.0f;
        
        //if gui control is enabled, make the mouse not control the camera, if gui control is disabled, let the mouse control the camera
        if(!GUICONTROL){
            cameraHorizontalAngle -= dx * cameraAngularSpeed * dt;
            cameraVerticalAngle   -= dy * cameraAngularSpeed * dt;
            
            float theta = radians(cameraHorizontalAngle);
            float phi = radians(cameraVerticalAngle);
            
            cameraLookAt = vec3(cosf(phi)*cosf(theta), sinf(phi), -cosf(phi)*sinf(theta));
            vec3 cameraSideVector = cross(cameraLookAt, vec3(0.0f, 1.0f, 0.0f));
            normalize(cameraSideVector);
            
            
            
            cameraVerticalAngle = std::max(-85.0f, std::min(85.0f, cameraVerticalAngle));
            if (cameraHorizontalAngle > 360)
            {
                cameraHorizontalAngle -= 360;
            }
            else if (cameraHorizontalAngle < -360)
            {
                cameraHorizontalAngle += 360;
            }
            
            
            
            if (cameraFirstPerson)
            {
                viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp );
                
                near_plane = 1.0f;
                far_plane = 300.0f;
                
            }
            else
            {
                
                
                viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp );
                
                near_plane = 1.0f;
                far_plane = 125.0f;
            }
            
            
        } else {
            
            if (cameraFirstPerson)
            {
                viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp );
                
            }
            else
            {
                
                viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp );
                
                near_plane = 1.0f;
                far_plane = 125.0f;
            }
            
        }
        
        //these are the following keybindings to control the world
        
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) //camera  move left
        {
            cameraPosition.z -= currentCameraSpeed * dt*40;
            
            
            
        }
        
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) //camera move right
        {
            cameraPosition.z += currentCameraSpeed * dt*40;
            
            
            
        }
        
        
        
        
        
        
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS )//camera  move backwards
        {
            cameraPosition.x -= currentCameraSpeed * dt*40;
            
            
            
        }
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) //camera a move forward
        {
            cameraPosition.x += currentCameraSpeed * dt*40;
            
            
        }
        
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) //light pos move up
        {
            
            cameraPosition.y -= currentCameraSpeed * dt*40;
            
        }
        
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) // light pos move up
        {
            
            cameraPosition.y += currentCameraSpeed * dt*40;
            
        }
        
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) //move light and camera down
        {
            
            lightpos.y -= currentCameraSpeed * dt*40;
            
        }
        
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) //move light and camera up
        {
            lightpos.y += currentCameraSpeed * dt*40;
            
        }
        
        
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) //move light in x direction
        {
            float xTrans = 0.1;
            xTrans = xTrans + 0.001f;
            lightpos.x += xTrans;
        }
        
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) // move light in negative x direction
        {
            float xTrans = 0.1;
            xTrans = xTrans + 0.001f;
            lightpos.x -= xTrans;
        }
        
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) // move light up
        {
            float yTrans = 0.1;
            yTrans = yTrans + 0.001f;
            lightpos.y += yTrans;
        }
        
        if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) // move light down
        {
            float yTrans = 0.1;
            yTrans = yTrans + 0.001f;
            lightpos.y -= yTrans;
        }
        
        if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) // move light in z direction
        {
            
            float zTrans = 0.1;
            zTrans = zTrans + 0.001f;
            lightpos.z += zTrans ;
        }
        
        if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) // move light in z negative direction
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
        
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) //change height upwards of world
        {
            xTrans = xTrans + 0.001f;
            if(xTrans >= 0.199)
                xTrans = 0.199;
        }
        
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) // change height downwards of world
        {
            xTrans = xTrans - 0.001f;
            if(xTrans <= -0.99)
                xTrans = -0.99;
        }
        
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        
        
        
        textureShader.use();
        
        glfwSetScrollCallback(window, scroll_callback);
        projectionMatrix = perspective(radians(fovAngle),1024.0f / 768.0f, near_plane,far_plane);
        
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
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) //toggle flat shading
        flatOn = !flatOn;
    
    if (key == GLFW_KEY_B && action == GLFW_PRESS) // toggle textures
        textureOn = !textureOn;
    
    if (key == GLFW_KEY_M && action == GLFW_PRESS) // toggle camera view
        cameraFirstPerson = !cameraFirstPerson;
    
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) // toggle gui control
    {
        if(!GUICONTROL)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        GUICONTROL =!GUICONTROL;
        show_demo_window = !show_demo_window;
    }
}


//create terrain geomtry function
void createTerrainGeometry(GLuint &VAO, int &xOffset, int &zOffset, Model& object_model)
{
    vector<float> vertices;
    vector <int> indices(6 * (mapZ - 1) * (mapZ - 1));
    vector<float> textureCoords;
    
    float xSample = 0;
    float zSample = 0;
    float amp  = 1;
    float freq = 1;
    
    //create vertices and noise
    float  rangedNoise = 0;
    
    for (int z = 0; z < mapZ ; z++)
        for (int x = 0; x < mapX; x++)
        {
            vertices.push_back(x);
            textureCoords.push_back(x);
            amp  = 1;
            freq = 1;
            float noiseHeight = 0;
            for (int i = 0; i < octaves; i++)
            {
                //jagged
                if(TerrainMode ==0)
                {
                    xSample = (xOffset * (mapX-1) + x-1)  / noiseScale * freq;
                    zSample = (zOffset * (mapZ-1) + z-1) / noiseScale * freq;
                }
                //smooth
                if(TerrainMode ==1)
                {
                    xSample = (xOffset * (mapX-1) + x-1)  / noiseScale;
                    zSample = (zOffset * (mapZ-1) + z-1) / noiseScale;
                }
                //block
                if(TerrainMode ==2)
                {
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
    glm::mat4 *modelMatrices;
    modelMatrices = new glm::mat4[number_of_trees];
    
    int counter = 0;
    while (counter < number_of_trees)
    {
        int c = (rand() % (mapZ * mapX)) * 3; // random number that is less than vertices_copy.size()[4,915,200] and divisible by 3
        float x = vertices[c];
        float y = vertices[c+1];
        float z = vertices[c+2];
        
        if (y < 1 || y > 16)    // matches the grass numbers in the fragshader
            continue;
        
        mat4 model = mat4(1.0f);
        model = glm::translate(model, vec3(x, y, z));
        model = glm::scale(model, vec3(0.15f));
        float rotAngle = (rand() % 360);
        model = glm::rotate(model, rotAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrices[counter] = model;
        heightPosTrees[(int)z][(int)x] = y;
        counter++;
    }
    
    // configure instanced array
    // -------------------------
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, (number_of_trees) * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);
    
    // set transformation matrices as an instance vertex attribute (with divisor 1)
    // ----------------------------------------------------------------------------
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
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, textureCoords.size() * sizeof(float), &textureCoords[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    
}

//render geometry and tree objects
void renderTerrain(vector <GLuint> &VAO, Shader &shader, int &nIndices, vec3 &cameraPosition, Model &object_model) {
    
    shader.use();
    GLuint modelViewProjection_terrain = glGetUniformLocation(shader.ID, "mvp");
    glActiveTexture(GL_TEXTURE0 + 12);
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
    
    for (int z = 0; z < zMapChunks; z++)
    {
        for (int x = 0; x < xMapChunks; x++)
        {
            mat4 mvp = glm::mat4(1.0f);
            mvp = translate(mvp, vec3( (mapX - 1) * x, 0.0, (mapZ - 1) * z));
            
            glUniformMatrix4fv(modelViewProjection_terrain, 1, GL_FALSE, &mvp[0][0]);
            
            glBindVertexArray(VAO[x + z*xMapChunks]);
            glDrawElements(primativeRender, nIndices, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);   // reset
            
        }
    }
    
    
    //if the height is adjusted below this dont draw the trees.
    if( xTrans>= -0.843) {
        shader.setBool("instanceOn", true);
        shader.setInt("treeColor", 1);
        
        for (unsigned int i = 0; i < object_model.meshes.size(); i++)
        {
            if (i % 2 == 0)
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D,8);
            }
            
            else
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D,9);
            }
            glBindVertexArray(object_model.meshes.at(i).VAO);
            glBindBufferRange(GL_UNIFORM_BUFFER,0, object_model.getUniformIndex().at(i),0,object_model.getMaterialSize().at(i));
            glDrawElementsInstanced(primativeRender, object_model.meshes.at(i).indices.size(), GL_UNSIGNED_INT, 0, number_of_trees);
            glBindVertexArray(0);
        }
        
        shader.setInt("treeColor", 0);
        shader.setBool("instanceOn", false);
    }
}

//helper function to map values to stay within a range
float Remap (float value, float from1, float to1, float from2, float to2)
{
    return (value - from1) / ((to1 - from1) * (to2 - from2) + from2);
}

//creates the map and calls create terrain geometry
void createMap(Model &model)
{
    for (int z = 0; z < zMapChunks; z++)
    {
        for (int x = 0; x < xMapChunks; x++)
        {
            createTerrainGeometry(VAO[x + z*xMapChunks], x, z, model);
        }
    }
}

//barrycentric calcuation function from ThinMatrix opengl tutorial
float barryCentric(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 pos)
{
    float det = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
    float l1 = ((p2.z - p3.z) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.y - p3.z)) / det;
    float l2 = ((p3.z - p1.z) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.y - p3.z)) / det;
    float l3 = 1.0f - l1 - l2;
    return l1 * p1.y + l2 * p2.y + l3 * p3.y;
}

//get height funciton inspired by ThinMatric opengl tutorial
float getHeight(float x, float z)
{
    float terrainX = x -1;
    float terrainZ = z-1;
    
    int height = mapX*xMapChunks;
    
    int gridSize = (mapX*xMapChunks) / height;
    
    int gridX = floor(terrainX/gridSize);
    int gridZ = floor(terrainZ / gridSize);
    
    if(gridX >= (height-1)|| gridZ >= (height-1)|| gridX < 0 || gridZ <0)
        return 0;
    
    int xCoordMod =  (int)terrainX % mapZ;
    int zCoordMod = (int)terrainZ % mapZ;
    
    float xCoord = (float)xCoordMod/(float)mapX;
    float zCoord = (float)zCoordMod/(float)mapZ;
    
    float answer;
    if(xCoord <= (1-zCoord))
    {
        answer = barryCentric(glm::vec3(0, heightPos[gridZ][gridX], 0),
                              glm::vec3(1, heightPos[gridZ+1][gridX], 0), glm::vec3(0, heightPos[gridZ][gridX + 1], 1), glm::vec2(xCoord, zCoord));
    }
    else
    {
        answer = barryCentric(glm::vec3(1, heightPos[gridZ+1][gridX], 0),
                              glm::vec3(1, heightPos[gridZ+1][gridX+1], 1), glm::vec3(0, heightPos[gridZ][gridX + 1], 1), glm::vec2(xCoord, zCoord));
    }
    
    return answer;
}

// whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    processMouseScroll(yoffset);
}

// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void processMouseScroll(float yoffset)
{
    if (fovAngle >= 10.0f && fovAngle <= 45.0f)
        fovAngle -= yoffset;
    if (fovAngle <= 10.0f)
        fovAngle = 10.0f;
    if (fovAngle >= 45.0f)
        fovAngle = 45.0f;
}

//get height funciton inspired by ThinMatric opengl tutorial
float getHeightTrees(float x, float z)
{
    float terrainX = x -1;
    float terrainZ = z-1;
    
    int height = mapX*xMapChunks;
    
    int gridSize = (mapX*xMapChunks) / height;
    
    int gridX = floor(terrainX/gridSize);
    int gridZ = floor(terrainZ / gridSize);
    
    if(gridX >= (height-1)|| gridZ >= (height-1)|| gridX < 0 || gridZ <0)
        return 0;
    
    int xCoordMod =  (int)terrainX % mapZ;
    int zCoordMod = (int)terrainZ % mapZ;
    
    float xCoord = (float)xCoordMod/(float)mapX;
    float zCoord = (float)zCoordMod/(float)mapZ;
    
    float answer;
    if(xCoord <= (1-zCoord))
    {
        answer = barryCentric(glm::vec3(0, heightPosTrees[gridZ][gridX], 0),
                              glm::vec3(1, heightPosTrees[gridZ+1][gridX], 0), glm::vec3(0, heightPosTrees[gridZ][gridX + 1], 1), glm::vec2(xCoord, zCoord));
    }
    else
    {
        answer = barryCentric(glm::vec3(1, heightPosTrees[gridZ+1][gridX], 0),
                              glm::vec3(1, heightPosTrees[gridZ+1][gridX+1], 1), glm::vec3(0, heightPosTrees[gridZ][gridX + 1], 1), glm::vec2(xCoord, zCoord));
    }
    
    return answer;
}
