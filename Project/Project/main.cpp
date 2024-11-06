#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>


void cameraAnimation();

// window
gps::Window myWindow;
const unsigned int SHADOW_WIDTH = 16384;
const unsigned int SHADOW_HEIGHT = 16384;
int retina_width, retina_height;

float lastX = myWindow.getWindowDimensions().width / 2.0f;
float lastY = myWindow.getWindowDimensions().height / 2.0f;
bool firstMouse = true;

float yaw = -90.0f;
float pitch = 0.0f;

std::vector<const GLchar*> faces;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::mat4 lightRotation;

// light parameters
glm::vec3 lightDir = glm::vec3(500.0f, 1000.0f, 0.0f);
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);;
   
// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;


GLuint shadowMapFBO;
GLuint depthMapTexture;
GLuint textureID;

// camera
/*gps::Camera myCamera(
    glm::vec3(0.0f, 0.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));
    */

gps::Camera camera(glm::vec3(0.0f, 6.0f, 30.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

float angleY = 0.0f;
GLfloat lightAngle;

// models
gps::Model3D goblin;
gps::Model3D terrain;
gps::Model3D tree1;
gps::Model3D tree2;
gps::Model3D tree3;
gps::Model3D tree4;
gps::Model3D tree5;
gps::Model3D tree6;
gps::Model3D tree7;
gps::Model3D tree8;
gps::Model3D tree9;
gps::Model3D tree10;
gps::Model3D tree11;
gps::Model3D tree12;
gps::Model3D tree13;
gps::Model3D tree14;
gps::Model3D tree15;
gps::Model3D ancientTree;
gps::Model3D ancientTree2;
gps::Model3D ancientTree3;
gps::Model3D ritual;
gps::Model3D house;
gps::Model3D guard;
gps::Model3D guard2;
gps::Model3D lantern;
gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D launchingPad;
GLfloat angle;

//Skybox
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

// shaders
gps::Shader myBasicShader;
gps::Shader depthMapShader;
gps::Shader lightShader;
gps::Shader fragmentDiscarding;

int LightEnabler = 1;

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

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    yaw += xoffset * 0.05f;
    pitch += yoffset * 0.05f;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    camera.rotate(pitch, yaw);
    view = camera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}

void processMovement() {

	if (pressedKeys[GLFW_KEY_W]) {
        camera.move(gps::MOVE_FORWARD, cameraSpeed);
        view = camera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
        camera.move(gps::MOVE_BACKWARD, cameraSpeed);
        view = camera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
        camera.move(gps::MOVE_LEFT, cameraSpeed);
        view = camera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
        camera.move(gps::MOVE_RIGHT, cameraSpeed);
        view = camera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }
    if (pressedKeys[GLFW_KEY_X]) {
        myBasicShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "LightEnabler"), LightEnabler);
        LightEnabler = !LightEnabler;
    }
}


int timeIndex;
void cameraAnimation()
{
    if (timeIndex < 30)
    {
        pitch += 0.5f;
        camera.rotate(pitch, yaw);
        view = camera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (timeIndex > 70 && timeIndex < 140)
    {
        camera.move(gps::MOVE_FORWARD, cameraSpeed);
        view = camera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (timeIndex > 170 && timeIndex < 200)
    {
        pitch -= 0.5f;
        camera.rotate(pitch, yaw);
        view = camera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (timeIndex > 200 && timeIndex < 250)
    {
        camera.move(gps::MOVE_FORWARD, cameraSpeed);
        view = camera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (timeIndex > 250 && timeIndex < 480)
    {
        yaw -= 0.3f;
        camera.rotate(pitch, yaw);
        view = camera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (timeIndex > 480 && timeIndex < 550) {

        camera.rotate(pitch, yaw);
        view = camera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
       
    if (timeIndex > 550 && timeIndex < 780)
    {
        yaw += 0.4f;
        camera.rotate(pitch, yaw);
        view = camera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (timeIndex > 780 && timeIndex < 900)
    {
        camera.move(gps::MOVE_FORWARD, cameraSpeed);
        view = camera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (timeIndex > 900 && timeIndex < 1050)
    {
        yaw += 0.4f;
        camera.rotate(pitch, yaw);
        view = camera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (timeIndex > 1050 && timeIndex < 1250) {
        camera.rotate(pitch, yaw);
        view = camera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
        
    if (timeIndex > 1136 && timeIndex < 1336)
    {
        yaw -= 0.4f;
        camera.rotate(pitch, yaw);
        view = camera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (timeIndex > 1336 && timeIndex < 1456)
    {
        camera.move(gps::MOVE_FORWARD, cameraSpeed);
        view = camera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (timeIndex > 1456 && timeIndex < 1680) {

        yaw += 0.4f;
        camera.rotate(pitch, yaw);
        view = camera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (timeIndex > 1750 && timeIndex < 1800) {

        pitch -= 0.2f;
        camera.rotate(pitch, yaw);
        view = camera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (timeIndex > 1800 && timeIndex < 2100) {

        camera.move(gps::MOVE_FORWARD, cameraSpeed);
        view = camera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (timeIndex > 2100 && timeIndex < 2150) {

        pitch += 0.2f;
        camera.rotate(pitch, yaw);
        view = camera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    timeIndex++;
}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Core");
}

void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void initOpenGLState() {

	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); 
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE); 
	glCullFace(GL_BACK); 
	glFrontFace(GL_CCW); 
}

void initModels() {
    goblin.LoadModel("models/goblin/goblinballoon.obj");
    terrain.LoadModel("models/terrain/ground.obj");
    tree1.LoadModel("models/tree/7xp_leytree_a01.obj");
    tree2.LoadModel("models/tree/7xp_leytree_a01.obj");
    tree3.LoadModel("models/tree/7xp_leytree_a01.obj");
    tree4.LoadModel("models/tree/7xp_leytree_a01.obj");
    tree5.LoadModel("models/tree/7xp_leytree_a01.obj");
    tree6.LoadModel("models/tree/7xp_leytree_a01.obj");
    tree7.LoadModel("models/tree/7xp_leytree_a01.obj");
    tree8.LoadModel("models/tree/7xp_leytree_a01.obj");
    tree9.LoadModel("models/tree/7xp_leytree_a01.obj");
    tree10.LoadModel("models/tree/7xp_leytree_a01.obj");
    tree11.LoadModel("models/tree/7xp_leytree_a01.obj");
    tree12.LoadModel("models/tree/7xp_leytree_a01.obj");
    tree13.LoadModel("models/tree/7xp_leytree_a01.obj");
    tree14.LoadModel("models/tree/7xp_leytree_a01.obj");
    tree15.LoadModel("models/tree/7xp_leytree_a01.obj");
    ancientTree.LoadModel("models/ancient/7an_ancienttree_c02.obj");
    ancientTree2.LoadModel("models/ancient2/7an_ancienttree_c01.obj");
    ancientTree3.LoadModel("models/ancient2/7an_ancienttree_c01.obj");
    house.LoadModel("models/house/blacksmith_ab.obj");
    guard.LoadModel("models/guard/bloodelfmale_guard.obj");
    guard2.LoadModel("models/guard/bloodelfmale_guard.obj");
    lantern.LoadModel("models/lantern/nightelflantern01.obj");
    launchingPad.LoadModel("models/pad/8du_kezan_horde_goblin_launchpad01.obj");
    ritual.LoadModel("models/ritual/8wi_witch_ritualcircle01.obj");
}

void initShaders() {
	myBasicShader.loadShader(
        "shaders/shaderStart.vert",
        "shaders/shaderStart.frag");
    myBasicShader.useShaderProgram();
}

void initUniforms() {

	myBasicShader.useShaderProgram();

    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	view = camera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	projection = glm::perspective(glm::radians(45.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "lantern"), 1, glm::value_ptr(glm::vec3(83.0f, 4.0f, 7.0f)));

	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));



    mySkyBox.Load(faces);
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
    view = camera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
        glm::value_ptr(view));
    projection = glm::perspective(glm::radians(60.0f), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
    glm::value_ptr(projection));

}

float animationIndex;
void renderGoblin(gps::Shader shader) {

    shader.useShaderProgram();

    animationIndex += 0.03;
    float ascendingOffset = 0.5 * animationIndex;
    float initialPos = 5.0f;
    glm::mat4 goblinModel = glm::translate(glm::mat4(1.0f), glm::vec3(animationIndex, initialPos + ascendingOffset, 0.0f));
    initialPos = 0;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(goblinModel));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    goblin.Draw(shader);
}


void renderPad(gps::Shader shader) {

    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-20.0f, 0.0f, -5.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    launchingPad.Draw(shader);
}


void renderTerrain(gps::Shader shader) {
    shader.useShaderProgram();

    glm::mat4 planeModel = glm::mat4(1.0f);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(planeModel));

    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    terrain.Draw(shader);
}

void renderTrees(gps::Shader shader) {

    shader.useShaderProgram();

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    model = glm::translate(glm::mat4(1.0f), glm::vec3(40.0f, 0.0f, -40.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    tree1.Draw(shader);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(55.0f, 0.0f, -42.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    tree2.Draw(shader);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(63.0f, 0.0f, -44.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    tree3.Draw(shader);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(47.0f, 0.0f, -53.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    tree4.Draw(shader);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(75.0f, 0.0f, -52.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    tree5.Draw(shader);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(30.0f, 0.0f, -38.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    tree6.Draw(shader);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(35.0f, 0.0f, -58.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    tree7.Draw(shader);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(53.0f, 0.0f, -68.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    tree8.Draw(shader);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(67.0f, 0.0f, -65.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    tree9.Draw(shader);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(77.0f, 0.0f, -34.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    tree10.Draw(shader);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(63.0f, 0.0f, -70.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    tree11.Draw(shader);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(43.0f, 0.0f, -30.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    tree12.Draw(shader);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(53.0f, 0.0f, -69.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    tree13.Draw(shader);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(68.0f, 0.0f, -76.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    tree14.Draw(shader);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(52.0f, 0.0f, -56.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    tree15.Draw(shader);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(40.0f, 0.0f, 45.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    ancientTree.Draw(shader);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(85.0f, 0.0f, 25.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    ancientTree2.Draw(shader);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(65.0f, 0.0f, 45.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    ancientTree3.Draw(shader);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(55.0f, 0.0f, 25.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    ritual.Draw(shader);
    
}

void renderHouse(gps::Shader shader) {

    shader.useShaderProgram();

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    model = glm::translate(glm::mat4(1.0f), glm::vec3(100.0f, 0.0f, -5.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    house.Draw(shader);
}

void renderGuard(gps::Shader shader) {

    glm::mat4 modelAux = glm::mat4(1.0f);
    shader.useShaderProgram();

    modelAux = glm::translate(modelAux, glm::vec3(80.0f, 0.0f, -5.0f)); 
    modelAux = glm::rotate(modelAux, glm::radians(230.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelAux));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    guard.Draw(shader);

    modelAux = glm::mat4(1.0f);

    modelAux = glm::translate(modelAux, glm::vec3(84.0f, 0.0f, 2.0f));
    modelAux = glm::rotate(modelAux, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelAux));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    guard2.Draw(shader);
}

void renderLantern(gps::Shader shader) {

    glm::mat4 modelAux = glm::mat4(1.0f);
    shader.useShaderProgram();

    modelAux = glm::translate(modelAux, glm::vec3(83.0f, 0.0f, 7.0f));
    modelAux = glm::rotate(modelAux, glm::radians(100.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelAux));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    glCheckError();
    lantern.Draw(shader);
}


void renderScene() {


    if (timeIndex < 2151)
      cameraAnimation();
    
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (pressedKeys[GLFW_KEY_U]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (pressedKeys[GLFW_KEY_P]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (pressedKeys[GLFW_KEY_L]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }
    
	renderGoblin(myBasicShader);
    renderTerrain(myBasicShader);
    renderTrees(myBasicShader);
    renderHouse(myBasicShader);
    renderGuard(myBasicShader); 
    renderLantern(myBasicShader);
    renderPad(myBasicShader);
    mySkyBox.Draw(skyboxShader, view, projection);

}

void cleanup() {

    myWindow.Delete();
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    faces.push_back("textures/skybox/stormydays_rt.tga");
    faces.push_back("textures/skybox/stormydays_lf.tga");
    faces.push_back("textures/skybox/stormydays_up.tga");
    faces.push_back("textures/skybox/stormydays_dn.tga");
    faces.push_back("textures/skybox/stormydays_bk.tga");
    faces.push_back("textures/skybox/stormydays_ft.tga");
   
    initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
    setWindowCallbacks();

	glCheckError();

	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
