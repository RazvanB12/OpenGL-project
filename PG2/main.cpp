#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp" //core glm functionality
#include "glm/gtc/matrix_transform.hpp" //glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp" //glm extension for computing inverse matrices
#include "glm/gtc/type_ptr.hpp" //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#pragma comment (lib , "winmm.lib")

#include <random>

#include <iostream>

// window
gps::Window myWindow;

// scena
// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;
GLuint lightDirLoc;
GLuint lightColorLoc;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 1.0f, 3.0f),
    glm::vec3(0.0f, 1.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

// models
gps::Model3D scene;
GLfloat angle;

// shaders
gps::Shader myBasicShader;

// declare skybox
gps::Shader skyBoxShader;
gps::SkyBox skyBoxDay;
std::vector<const GLchar*> skyBoxFacesDay;

gps::SkyBox skyBoxNight;
std::vector<const GLchar*> skyBoxFacesNight;

gps::SkyBox skyBoxWarm;
std::vector<const GLchar*> skyBoxFacesWarm;

gps::SkyBox skyBoxRain;
std::vector<const GLchar*> skyBoxFacesRain;

bool daySkyBoxEnable = true;
bool nightSkyBoxEnable = false;
bool warmSkyBoxEnable = false;
bool rainSkyBoxEnable = false;
bool skyboxTransition = false;

int retina_width, retina_height;

// pentru rotatie
float angleUp = 0.0f;
float angleRight = 0.0f;
float angleRotation = 0.0f;

// pentru filmare
bool playAnimation = false;
int nextPoint;

// pentru ceata
bool enableFog = false;

// pentru spotlight
bool enableSpotLight = false;
GLuint lightPosLoc;
GLuint lightDirFrontLoc;
glm::vec3 lightPos;
glm::vec3 lightDirFront;


// semn circulatie
gps::Model3D trafficSign;
glm::mat4 model2;
glm::mat3 normalMatrix2;

// strop de ploaie
const int noDrops = 100;
gps::Model3D waterDrop;
glm::mat4 model3[noDrops];
glm::mat3 normalMatrix3[noDrops];
float waterDropX[noDrops];
float waterDropY[noDrops];
float waterDropZ[noDrops];

// ploaie
bool enableRain = false;

// animatie skybox
bool enableSkyBoxAnimation = false;
float lightX = 1.0f;
float lightY = 1.0f;
float lightZ = 1.0f;
float day = true;

// fulger/tunet
float extraLight = 0.0f;
bool enableThunder = false;
bool thunderRise = true;

// a doua lumina
glm::vec3 lightDir2;
glm::vec3 lightColor2;
GLuint lightDirLoc2;
GLuint lightColorLoc2;

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    //TODO
}

void reset() {
    angleUp = 0.0f;
    angleRight = 0.0f;
    angleRotation = 0.0f;
    //update view matrix
    myCamera.resetCamera();
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    // compute normal matrix for scene
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}

void updateRainMatrix() {
    myBasicShader.useShaderProgram();
    for (int i = 0; i < noDrops; i++) {
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        waterDropY[i] -= 0.1f;
        model3[i] = glm::translate(glm::mat4(1.0f), glm::vec3(waterDropX[i], waterDropY[i], waterDropZ[i]));
        if (waterDropY[i] < 0) {
            waterDropY[i] = 15;
        }
        normalMatrix3[i] = glm::mat3(glm::inverseTranspose(view * model3[i]));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model3[i]));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix3[i]));
    }
}

void updateCameraMatrix() {
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    // compute normal matrix for scene
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    // stop movie
    if (playAnimation == true) {
        playAnimation = false;
        PlaySound(NULL, 0, 0);
        reset();
    }
}

void processMovement() {
    /*
    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        // update model matrix for scene
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for scene
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        // update model matrix for scene
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for scene
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }
    */
    
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        updateCameraMatrix();
	}
	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        updateCameraMatrix();
	}
	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        updateCameraMatrix();
	}
	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        updateCameraMatrix();
	}
    if (pressedKeys[GLFW_KEY_R]) {
        myCamera.move(gps::MOVE_UP, cameraSpeed);
        updateCameraMatrix();
    }
    if (pressedKeys[GLFW_KEY_F]) {
        myCamera.move(gps::MOVE_DOWN, cameraSpeed);
        updateCameraMatrix();
    }
    if (pressedKeys[GLFW_KEY_UP]) {
        angleUp += 0.01f;
        myCamera.rotate(angleUp, angleRight);
        updateCameraMatrix();
    }
    if (pressedKeys[GLFW_KEY_DOWN]) {
        angleUp -= 0.01f;
        myCamera.rotate(angleUp, angleRight);
        updateCameraMatrix();
    }
    if (pressedKeys[GLFW_KEY_RIGHT]) {
        angleRight += 0.01f;
        myCamera.rotate(angleUp, angleRight);
        updateCameraMatrix();
    }
    if (pressedKeys[GLFW_KEY_LEFT]) {
        angleRight -= 0.01f;
        myCamera.rotate(angleUp, angleRight);
        updateCameraMatrix();
    }
    if (pressedKeys[GLFW_KEY_0]) {
        playAnimation = false;
        PlaySound(NULL, 0, 0);
        reset();
    }
    if (pressedKeys[GLFW_KEY_1]) {
        angleUp = 0.0f;
        angleRight = 0.0f;
        angleRotation = 0.0f;
        //update view matrix
        myCamera.resetCamera();
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for scene
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        // start movie
        playAnimation = true;
        PlaySound(TEXT("Puya"), NULL, SND_ASYNC);
        nextPoint = 1;
    }
    if (pressedKeys[GLFW_KEY_2]) {
        // stop movie
        playAnimation = false;
        PlaySound(NULL, 0, 0);
        reset();
    }
    if (pressedKeys[GLFW_KEY_3]) {
        // enable fog
        enableFog = true;
        myBasicShader.useShaderProgram();
        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "enableFog"), enableFog);
        daySkyBoxEnable = false;
        rainSkyBoxEnable = true;
        nightSkyBoxEnable = false;
        warmSkyBoxEnable = false;
    }
    if (pressedKeys[GLFW_KEY_4]) {
        // disable fog
        enableFog = false;
        myBasicShader.useShaderProgram();
        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "enableFog"), enableFog);
        daySkyBoxEnable = true;
        rainSkyBoxEnable = false;
        nightSkyBoxEnable = false;
        warmSkyBoxEnable = false;
    } 
    if (pressedKeys[GLFW_KEY_5]) {
        // enable rain
        enableRain = true;
        daySkyBoxEnable = false;
        rainSkyBoxEnable = true;
        nightSkyBoxEnable = false;
        warmSkyBoxEnable = false;
        PlaySound(TEXT("rain"), NULL, SND_ASYNC);
    }
    if (pressedKeys[GLFW_KEY_6]) {
        // disable rain
        enableRain = false;
        daySkyBoxEnable = true;
        rainSkyBoxEnable = false;
        nightSkyBoxEnable = false;
        warmSkyBoxEnable = false;
        PlaySound(NULL, 0, 0);
    }
    if (pressedKeys[GLFW_KEY_7]) {
        // spot light
        lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
        myBasicShader.useShaderProgram();
        glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
        daySkyBoxEnable = false;
        rainSkyBoxEnable = false;
        nightSkyBoxEnable = true;
        warmSkyBoxEnable = false;
        // enable normal light
        enableSpotLight = true;
        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "enableSpotLight"), enableSpotLight);
    }
    if (pressedKeys[GLFW_KEY_8]) {
        // normal light
        lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
        lightColor2 = glm::vec3(1.0f, 1.0f, 1.0f);
        myBasicShader.useShaderProgram();
        glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
        glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightColor2"), 1, glm::value_ptr(lightColor2));
        daySkyBoxEnable = true;
        rainSkyBoxEnable = false;
        nightSkyBoxEnable = false;
        warmSkyBoxEnable = false;
        // enable normal light
        enableSpotLight = false;
        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "enableSpotLight"), enableSpotLight);
    }
    if (pressedKeys[GLFW_KEY_Z]) {
        // solid 
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (pressedKeys[GLFW_KEY_X]) {
        // wireframe
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (pressedKeys[GLFW_KEY_C]) {
        // point
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }
    if (pressedKeys[GLFW_KEY_V]) {
        // rotate right traffic sign
        model2 = glm::rotate(model2, glm::radians(-1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        normalMatrix2 = glm::mat3(glm::inverseTranspose(view * model2));
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix2));
    }
    if (pressedKeys[GLFW_KEY_B]) {
        // rotate left traffic sign
        model2 = glm::rotate(model2, glm::radians(+1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        normalMatrix2 = glm::mat3(glm::inverseTranspose(view * model2));
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix2));
    }
    if (pressedKeys[GLFW_KEY_N]) {
        // enable skybox transition
        enableSkyBoxAnimation = true;
        float day = true;
        lightX = 1.0f;
        lightY = 1.0f;
        lightZ = 1.0f;
    }
    if (pressedKeys[GLFW_KEY_M]) {
        // disable skybox transition
        enableSkyBoxAnimation = false;
        lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
        lightColor2 = glm::vec3(1.0f, 1.0f, 1.0f);
        myBasicShader.useShaderProgram();
        glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
        glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightColor2"), 1, glm::value_ptr(lightColor2));
        daySkyBoxEnable = true;
        rainSkyBoxEnable = false;
        nightSkyBoxEnable = false;
        warmSkyBoxEnable = false;
    }
    if (pressedKeys[GLFW_KEY_K]) {
        // enable Thunder
        extraLight = 0.0f;
        enableThunder = true;
        thunderRise = true;
        glfwSetTime(0.0f);
    }
    if (pressedKeys[GLFW_KEY_L]) {
        // disable Thunder
        enableThunder = false;
        PlaySound(NULL, 0, 0);
    }
    if (pressedKeys[GLFW_KEY_U]) {
        // enable first light
        lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
        myBasicShader.useShaderProgram();
        glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
    }
    if (pressedKeys[GLFW_KEY_I]) {
        // disable first light
        lightColor = glm::vec3(0.0f, 0.0f, 0.0f);
        myBasicShader.useShaderProgram();
        glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
    }
    if (pressedKeys[GLFW_KEY_O]) {
        // enable second light
        lightColor2 = glm::vec3(1.0f, 1.0f, 1.0f);
        myBasicShader.useShaderProgram();
        glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightColor2"), 1, glm::value_ptr(lightColor2));
    }
    if (pressedKeys[GLFW_KEY_P]) {
        // disable second light
        lightColor2 = glm::vec3(0.0f, 0.0f, 0.0f);
        myBasicShader.useShaderProgram();
        glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightColor2"), 1, glm::value_ptr(lightColor2));
    }
}

void initOpenGLWindow() {
    myWindow.Create(1920, 1080, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    glfwGetFramebufferSize(myWindow.getWindow(), &retina_width, &retina_height);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    scene.LoadModel("models/Scena/projectPG.obj");
    trafficSign.LoadModel("models/Sign/Sign.obj");
    waterDrop.LoadModel("models/WaterDrop/WaterDrop.obj");
}

void initShaders() {
    myBasicShader.loadShader("shaders/basic.vert", "shaders/basic.frag");
    
    skyBoxFacesDay.push_back("skybox/Day/right.tga");
    skyBoxFacesDay.push_back("skybox/Day/left.tga");
    skyBoxFacesDay.push_back("skybox/Day/top.tga");
    skyBoxFacesDay.push_back("skybox/Day/bottom.tga");
    skyBoxFacesDay.push_back("skybox/Day/back.tga");
    skyBoxFacesDay.push_back("skybox/Day/front.tga");

    skyBoxFacesNight.push_back("skybox/Night/right.bmp");
    skyBoxFacesNight.push_back("skybox/Night/left.bmp");
    skyBoxFacesNight.push_back("skybox/Night/top.bmp");
    skyBoxFacesNight.push_back("skybox/Night/bottom.bmp");
    skyBoxFacesNight.push_back("skybox/Night/back.bmp");
    skyBoxFacesNight.push_back("skybox/Night/front.bmp");

    skyBoxFacesWarm.push_back("skybox/Warm/right.tga");
    skyBoxFacesWarm.push_back("skybox/Warm/left.tga");
    skyBoxFacesWarm.push_back("skybox/Warm/top.tga");
    skyBoxFacesWarm.push_back("skybox/Warm/bottom.tga");
    skyBoxFacesWarm.push_back("skybox/Warm/back.tga");
    skyBoxFacesWarm.push_back("skybox/Warm/front.tga");

    skyBoxFacesRain.push_back("skybox/Rain/right.png");
    skyBoxFacesRain.push_back("skybox/Rain/left.png");
    skyBoxFacesRain.push_back("skybox/Rain/top.png");
    skyBoxFacesRain.push_back("skybox/Rain/bottom.png");
    skyBoxFacesRain.push_back("skybox/Rain/back.png");
    skyBoxFacesRain.push_back("skybox/Rain/front.png");

    skyBoxDay.Load(skyBoxFacesDay);
    skyBoxNight.Load(skyBoxFacesNight);
    skyBoxWarm.Load(skyBoxFacesWarm);
    skyBoxRain.Load(skyBoxFacesRain);
    skyBoxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyBoxShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(skyBoxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    glUniformMatrix4fv(glGetUniformLocation(skyBoxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void initRain() {
    for (int i = 0; i < noDrops; i++) {
        myBasicShader.useShaderProgram();

        waterDropX[i] = rand() % 40 - 20;
        waterDropY[i] = rand() % 15;
        waterDropZ[i] = rand() % 40 - 20;

        model3[i] = glm::translate(glm::mat4(1.0f), glm::vec3(waterDropX[i], waterDropY[i], waterDropZ[i]));
        modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model3[i]));
    }
}

void initUniforms() {
    myBasicShader.useShaderProgram();
    // create model matrix for scene
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for scene
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,0.1f, 150.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.1f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // a doua lumina
    lightDir2 = glm::vec3(0.0f, 1.0f, -1.0f);
    lightDirLoc2 = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir2");
    // send light dir to shader
    glUniform3fv(lightDirLoc2, 1, glm::value_ptr(lightDir2));

    lightColor2 = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc2 = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor2");
    // send light color to shader
    glUniform3fv(lightColorLoc2, 1, glm::value_ptr(lightColor2));
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // semn de circulatie
    model2 = glm::translate(glm::mat4(1.0f), glm::vec3(-23.0, 0.0, -3.5f));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ploaie
    initRain();
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void renderRain() {
    if (enableRain == true) {
        for (int i = 0; i < noDrops; i++) {
            myBasicShader.useShaderProgram();
            //send scene model matrix data to shader
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model3[i]));
            //send scene normal matrix data to shader
            glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix3[i]));
            waterDrop.Draw(myBasicShader);
        }
    }
}

void renderSceneObjects(gps::Shader shader) {
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // scena
    shader.useShaderProgram();
    
    //send scene model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send scene normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw scene
    scene.Draw(shader);
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // spotlight
    lightPos = myCamera.getCameraPosition();
    lightPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightPos");
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
    lightDirFront = myCamera.getCameraTarget();
    lightDirFrontLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDirFront");
    glUniform3fv(lightDirFrontLoc, 1, glm::value_ptr(lightDirFront));
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // semn de circulatie
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix2));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix2));
    trafficSign.Draw(shader);
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ploaie
    renderRain();
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    //draw skybox
    if (daySkyBoxEnable == true) {
        skyBoxDay.Draw(skyBoxShader, view, projection);
    }

    else if (nightSkyBoxEnable == true) {
        skyBoxNight.Draw(skyBoxShader, view, projection);
    }

    else if (warmSkyBoxEnable == true) {
        skyBoxWarm.Draw(skyBoxShader, view, projection);
    }
    else if (rainSkyBoxEnable == true) {
        skyBoxRain.Draw(skyBoxShader, view, projection);
    }
}

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderSceneObjects(myBasicShader);
}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

void play() {
    if (playAnimation == true) {
        glm::vec3 position = myCamera.getCameraPosition();

        if (nextPoint == 1) {
            if (position.x > -30.0f) {
                myCamera.move(gps::MOVE_LEFT, cameraSpeed);
            }
            if (position.y < 1.5f) {
                myCamera.move(gps::MOVE_UP, cameraSpeed);
            }
            if (position.z > -9.0f) {
                myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
            }
            if (angleRotation > -1.57f) {
                angleRotation -= 0.01f;
                myCamera.changeDirectionView(angleRotation);
            }
            if (position.x <= -30.0f && position.y >= 1.5f && position.z <= -9.0f && angleRotation <= -1.57f) {
                nextPoint++;
                Sleep(1000);
            }
        }

        else if (nextPoint == 2) {
            if (position.x > -40.0f) {
                myCamera.move(gps::MOVE_LEFT, cameraSpeed);
            }
            if (position.y < 4.5f) {
                myCamera.move(gps::MOVE_UP, cameraSpeed);
            }
            if (position.z < -2.5f) {
                myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
            }
            if (angleRotation < 0.0f) {
                angleRotation += 0.01f;
                myCamera.changeDirectionView(angleRotation);
            }
            if (position.x <= -40.0f && position.y >= 4.5f && position.z >= -2.5f && angleRotation >= 0.0f) {
                nextPoint++;
                Sleep(1000);
            }
        }

        else if (nextPoint == 3) {
            if (position.x < -20.0f) {
                myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
            }
            if (position.y > 1.5f) {
                myCamera.move(gps::MOVE_DOWN, cameraSpeed);
            }
            if (position.z < 13.5f) {
                myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
            }
            if (angleRotation < 1.57f) {
                angleRotation += 0.01f;
                myCamera.changeDirectionView(angleRotation);
            }
            if (position.x >= -20.0f && position.y <= 1.5f && position.z >= 13.5f && angleRotation >= 1.57f) {
                nextPoint++;
            }
        }

        else if (nextPoint == 4) {
            if (position.x < 0.0f) {
                myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
            }
            if (position.x >= 0.0f && position.y <= 1.5f && position.z >= 13.5f && angleRotation >= 1.57f) {
                nextPoint++;
            }
        }

        else if (nextPoint == 5) {
            if (position.x < 11.0f) {
                myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
            }
            if (position.z > 4.0f) {
                myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
            }
            if (position.x >= 11.0f && position.y <= 1.5f && position.z <= 4.0f && angleRotation >= 1.57f) {
                nextPoint++;
                Sleep(1000);
            }
        }

        else if (nextPoint == 6) {
            if (position.z > - 9.5f) {
                myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
            }
            if (angleRotation < 2.35f) {
                angleRotation += 0.01f;
                myCamera.changeDirectionView(angleRotation);
            }
            if (position.x >= 11.0f && position.y <= 1.5f && position.z <= -9.5f && angleRotation >= 2.35f) {
                nextPoint++;
                Sleep(1000);
            }
        }

        else if (nextPoint == 7) {
            if (position.x > 0.0f) {
                myCamera.move(gps::MOVE_LEFT, cameraSpeed);
            }
            if (position.y < 3.0f) {
                myCamera.move(gps::MOVE_UP, cameraSpeed);
            }
            if (position.z < 0.0f) {
                myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
            }
            if (angleRotation > 1.57f) {
                angleRotation -= 0.01f;
                myCamera.changeDirectionView(angleRotation);
            }
            if (position.x <= 0.0f && position.y >= 3.0f && position.z >= 0.0f && angleRotation <= 1.57f) {
                nextPoint++;
            }
        }

        else if (nextPoint == 8) {
            if (angleRotation < 7.85) {
                angleRotation += 0.005f;
                myCamera.changeDirectionView(angleRotation);
            }
            if (position.x <= 0.0f && position.y >= 3.0f && position.z >= 0.0f && angleRotation >= 7.85f) {
                nextPoint++;
            }
        }

        else if (nextPoint == 9) {
            if (position.x > -120.0f) {
                myCamera.move(gps::MOVE_LEFT, cameraSpeed);
            }
            if (position.y < 10.0f) {
                myCamera.move(gps::MOVE_UP, cameraSpeed);
            }
            if (position.x <= -120.0f && position.y >= 10.0f && position.z >= 0.0f && angleRotation >= 7.85f) {
                nextPoint++;
            }
        }

        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for scene
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    }
}

void skyBoxAnimation() {
    if (day == true) {
        if (lightX > 0.6f && lightY > 0.6f && lightZ > 0.6f) {
            lightY -= 0.005f;
            lightZ -= 0.005f;
            daySkyBoxEnable = true;
            nightSkyBoxEnable = false;
            warmSkyBoxEnable = false;
            rainSkyBoxEnable = false;
        }
        else if (lightX > 0.6f && lightY > 0.4f && lightZ > 0.4f) {
            lightY -= 0.005f;
            lightZ -= 0.005f;
            daySkyBoxEnable = false;
            nightSkyBoxEnable = false;
            warmSkyBoxEnable = true;
            rainSkyBoxEnable = false;
        }
        else if (lightX > 0.4f) {
            lightX -= 0.005f;
        }
        else if (lightX > 0.1f && lightY > 0.1f && lightZ > 0.1f) {
            lightX -= 0.005f;
            lightY -= 0.005f;
            lightZ -= 0.005f;
            daySkyBoxEnable = false;
            nightSkyBoxEnable = true;
            warmSkyBoxEnable = false;
            rainSkyBoxEnable = false;
        }
        else {
            day = false;
            Sleep(4000);
        }
    }

    else {
        if (lightX < 0.4f && lightY < 0.4f && lightZ < 0.4f) {
            lightX += 0.005f;
            lightY += 0.005f;
            lightZ += 0.005f;
            daySkyBoxEnable = false;
            nightSkyBoxEnable = true;
            warmSkyBoxEnable = false;
            rainSkyBoxEnable = false;
        }
        else if (lightX < 1.0f) {
            lightX += 0.005f;
            daySkyBoxEnable = false;
            nightSkyBoxEnable = false;
            warmSkyBoxEnable = true;
            rainSkyBoxEnable = false;
        }
        else if (lightY < 0.6f && lightZ < 0.6f) {
            lightY += 0.005f;
            lightZ += 0.005f;
        }
        else if (lightY < 1.0f && lightZ < 1.0f) {
            lightY += 0.005f;
            lightZ += 0.005f;
            daySkyBoxEnable = true;
            nightSkyBoxEnable = false;
            warmSkyBoxEnable = false;
            rainSkyBoxEnable = false;
        }
        else {
            day = true;
            Sleep(2000);
        }
    }
   
    lightColor = glm::vec3(lightX, lightY, lightZ);
    lightColor2 = glm::vec3(lightX, lightY, lightZ);
    myBasicShader.useShaderProgram();
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightColor2"), 1, glm::value_ptr(lightColor2));
}

void thunderAnimation() {
    if (enableThunder == true) {
        myBasicShader.useShaderProgram();
        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "enableThunder"), enableThunder);
        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "extraLight"), extraLight);
        if (thunderRise == true) {
            if (glfwGetTime() >= 2.3f && glfwGetTime() <= 2.4f) {
                PlaySound(TEXT("thunder"), NULL, SND_ASYNC);
            }
            if (glfwGetTime() >= 3.0f && extraLight <= 10.0f) {
                extraLight += 1.0f;
            }
            else if (extraLight >= 10.0f) {
                thunderRise = false;
            }
        }
        else {
            if (extraLight >= 1.0f) {
                extraLight -= 1.0f;
            }
            else if (glfwGetTime() >= 7.0f) {
                thunderRise = true;
                glfwSetTime(0.0f);
                PlaySound(NULL, 0, 0);
            }
        }
        //printf("%f\n", glfwGetTime());
    }
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
    setWindowCallbacks();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
        play();
        if (enableRain == true) {
            updateRainMatrix();
        }
        if (enableSkyBoxAnimation == true) {
            skyBoxAnimation();
        }
        thunderAnimation();
	    renderScene();
		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());
		glCheckError();

        //glm::vec3 pos = myCamera.getCameraPosition();
        //printf("pos = %f %f %f\n", pos.x, pos.y, pos.z);

        //glm::vec3 tar = myCamera.getCameraTarget();
        //printf("tar = %f %f %f\n", tar.x, tar.y, tar.z);

        //glm::vec3 dir = myCamera.getCameraFrontDirection();
        //printf("dir = %f %f %f\n", dir.x, dir.y, dir.z);

        //glm::vec3 up = myCamera.getCameraUpDirection();
        //printf("up = %f %f %f\n", up.x, up.y, up.z);

        //glm::vec3 rig = myCamera.getCameraRightDirection();
        //printf("right = %f %f %f\n", rig.x, rig.y, rig.z);
	}

	cleanup();

    return EXIT_SUCCESS;
}
