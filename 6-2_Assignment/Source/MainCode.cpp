#include <iostream>
#include <cstdlib>
#include <memory>

#include <GL/glew.h>
#include "GLFW/glfw3.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SceneManager.h"
#include "ViewManager.h"
#include "ShapeMeshes.h"
#include "ShaderManager.h"

// Namespace for declaring global variables
namespace
{
    // Macro for window title
    const char* const WINDOW_TITLE = "6-2 Assignment";

    // Main GLFW window
    GLFWwindow* g_Window = nullptr;

    // scene manager object for managing the 3D scene prepare and render
    std::unique_ptr<SceneManager> g_SceneManager = nullptr;
    // shader manager object for dynamic interaction with the shader code
    std::unique_ptr<ShaderManager> g_ShaderManager = nullptr;
    // view manager object for managing the 3D view setup and projection to 2D
    std::unique_ptr<ViewManager> g_ViewManager = nullptr;
}

// Function declarations
bool InitializeGLFW();
bool InitializeGLEW();
void GLFWErrorCallback(int error, const char* description);

int main(int argc, char* argv[])
{
    // Set the error callback for GLFW
    glfwSetErrorCallback(GLFWErrorCallback);

    // If GLFW fails initialization, then terminate the application
    if (!InitializeGLFW())
    {
        return(EXIT_FAILURE);
    }

    // Create the shader manager object
    g_ShaderManager = std::make_unique<ShaderManager>();

    // Create the view manager object
    g_ViewManager = std::make_unique<ViewManager>(g_ShaderManager.get());

    // Try to create the main display window
    g_Window = g_ViewManager->CreateDisplayWindow(WINDOW_TITLE);
    if (!g_Window)
    {
        std::cerr << "ERROR: Failed to create GLFW window." << std::endl;
        return(EXIT_FAILURE);
    }

    // If GLEW fails initialization, then terminate the application
    if (!InitializeGLEW())
    {
        return(EXIT_FAILURE);
    }

    // Load the shader code from the external GLSL files
    if (!g_ShaderManager->LoadShaders(
        "../../Utilities/shaders/vertexShader.glsl",
        "../../Utilities/shaders/fragmentShader.glsl"))
    {
        std::cerr << "ERROR: Failed to load shaders." << std::endl;
        return(EXIT_FAILURE);
    }
    g_ShaderManager->use();

    // Create the scene manager object and prepare the 3D scene
    g_SceneManager = std::make_unique<SceneManager>(g_ShaderManager.get());
    g_SceneManager->PrepareScene();

    // Loop will keep running until the application is closed 
    // or until an error has occurred
    while (!glfwWindowShouldClose(g_Window))
    {
        // Enable z-depth
        glEnable(GL_DEPTH_TEST);

        // Clear the frame and z buffers
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Convert from 3D object space to 2D view
        g_ViewManager->PrepareSceneView();

        // Refresh the 3D scene
        g_SceneManager->RenderScene();

        // Flip the back buffer with the front buffer every frame
        glfwSwapBuffers(g_Window);

        // Query the latest GLFW events
        glfwPollEvents();
    }

    // Clear the allocated manager objects from memory
    g_SceneManager.reset();
    g_ViewManager.reset();
    g_ShaderManager.reset();

    // Terminate GLFW
    glfwTerminate();

    // Terminates the program successfully
    exit(EXIT_SUCCESS);
}

/***********************************************************
 *  InitializeGLFW()
 *
 *  This function is used to initialize the GLFW library.
 ***********************************************************/
bool InitializeGLFW()
{
    // GLFW: initialize and configure library
    if (!glfwInit())
    {
        std::cerr << "ERROR: Failed to initialize GLFW." << std::endl;
        return false;
    }

#ifdef __APPLE__
    // set the version of OpenGL and profile to use
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    // set the version of OpenGL and profile to use
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    // GLFW: end -------------------------------

    return true;
}

/***********************************************************
 *  InitializeGLEW()
 *
 *  This function is used to initialize the GLEW library.
 ***********************************************************/
bool InitializeGLEW()
{
    // GLEW: initialize
    GLenum GLEWInitResult = GLEW_OK;

    // try to initialize the GLEW library
    GLEWInitResult = glewInit();
    if (GLEW_OK != GLEWInitResult)
    {
        std::cerr << glewGetErrorString(GLEWInitResult) << std::endl;
        return false;
    }
    // GLEW: end -------------------------------

    // Displays a successful OpenGL initialization message
    std::cout << "INFO: OpenGL Successfully Initialized\n";
    std::cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << "\n" << std::endl;

    return true;
}

/***********************************************************
 *  GLFWErrorCallback()
 *
 *  This function is used as an error callback for GLFW.
 ***********************************************************/
void GLFWErrorCallback(int error, const char* description)
{
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}
