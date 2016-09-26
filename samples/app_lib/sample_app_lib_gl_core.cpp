
// ================================================================================================
// -*- C++ -*-
// File: sample_app_lib_gl_core.cpp
// Author: Guilherme R. Lampert
// Created on: 25/09/16
// Brief: Core OpenGL initialization for the samples library.
// ================================================================================================

#include "sample_app_lib.hpp"

#include <GL/gl3w.h> // An OpenGL extension wrangler (https://github.com/skaslev/gl3w).
#include <GLFW/glfw3.h>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define NTB_DEFAULT_RENDERER_GL_CORE
#include "ntb_renderer_gl_core.hpp"

// ========================================================

namespace
{

class MyNTBShellInterfaceGLFW : public ntb::ShellInterface
{
public:

    ~MyNTBShellInterfaceGLFW();
    ntb::Int64 getTimeMilliseconds() const override;
};

MyNTBShellInterfaceGLFW::~MyNTBShellInterfaceGLFW()
{ }

ntb::Int64 MyNTBShellInterfaceGLFW::getTimeMilliseconds() const
{
    const ntb::Float64 seconds = glfwGetTime();
    return static_cast<ntb::Int64>(seconds * 1000.0);
}

static MyNTBShellInterfaceGLFW g_ntbShell;

} // namespace

// ========================================================

static AppWindowHandle * appGlCoreInitInternal(const int glVersionMajor, const int glVersionMinor, const char * const windowTitle,
                                               const int windowWidth, const int windowHeight, ntb::RenderInterface ** outRenderInterface)
{
    if (!glfwInit())
    {
        std::fprintf(stderr, "[APP_ERROR]: Failed to initialize GLFW!\n");
        return nullptr;
    }

    // Things we need for the window / GL render context:
    glfwWindowHint(GLFW_RESIZABLE, false);
    glfwWindowHint(GLFW_DEPTH_BITS, 32);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glVersionMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glVersionMinor);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow * window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, nullptr, nullptr);
    if (window == nullptr)
    {
        std::fprintf(stderr, "[APP_ERROR]: Failed to create GLFW window!\n");
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);

    if (!gl3wInit())
    {
        std::fprintf(stderr, "[APP_ERROR]: Failed to initialize GL3W extensions library!\n");
        glfwTerminate();
        return nullptr;
    }

    if (!gl3wIsSupported(3, 2))
    {
        std::fprintf(stderr, "[APP_ERROR]: This sample application requires at least OpenGL version 3.2 to run!\n");
        gl3wShutdown();
        glfwTerminate();
        return nullptr;
    }

    std::printf("GL_VENDOR:    %s\n", glGetString(GL_VERSION));
    std::printf("GL_VERSION:   %s\n", glGetString(GL_VENDOR));
    std::printf("GLSL_VERSION: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    std::printf("Attempting to initialize sample renderer with GL Core profile...\n");
    (*outRenderInterface) = new ntb::RenderInterfaceDefaultGLCore(windowWidth, windowHeight);

    return reinterpret_cast<AppWindowHandle *>(window);
}

static void appGlCoreShutdown(AppContext * ctx)
{
    delete ctx->renderInterface;
    std::memset(ctx, 0, sizeof(*ctx));

    gl3wShutdown();
    glfwTerminate();
}

static void appGlCoreFrameUpdate(AppContext * ctx, bool * outIsDone)
{
    // NTB starts writing at Z=0 and increases for each primitive.
    // Since we decide to draw without sorting, then the depth buffer
    // must be cleared to zero before drawing the UI.
    glClearDepth(0);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (outIsDone != nullptr)
    {
        (*outIsDone) = glfwWindowShouldClose(reinterpret_cast<GLFWwindow *>(ctx->windowHandle));
    }
}

static void appGlCoreFramePresent(AppContext * ctx)
{
    glfwSwapBuffers(reinterpret_cast<GLFWwindow *>(ctx->windowHandle));
    glfwPollEvents();
}

bool appGlCoreInit(const int glVersionMajor, const int glVersionMinor, const char * const windowTitle,
                   const int windowWidth, const int windowHeight, AppContext * outCtx)
{
    ntb::RenderInterface * renderInterface = nullptr;
    AppWindowHandle * windowHandle = appGlCoreInitInternal(glVersionMajor, glVersionMinor, windowTitle,
                                                           windowWidth, windowHeight, &renderInterface);

    if (windowHandle == nullptr || renderInterface == nullptr)
    {
        return false;
    }

    outCtx->windowHandle    = windowHandle;
    outCtx->renderInterface = renderInterface;
    outCtx->shellInterface  = &g_ntbShell;
    outCtx->frameUpdate     = &appGlCoreFrameUpdate;
    outCtx->framePresent    = &appGlCoreFramePresent;
    outCtx->shutdown        = &appGlCoreShutdown;
    outCtx->windowWidth     = windowWidth;
    outCtx->windowHeight    = windowHeight;
    outCtx->isGlCoreProfile = true;

    return true;
}

// ================================================================================================
// GL3W is an OpenGL extension wrangler (https://github.com/skaslev/gl3w).
// This would ideally be built separately as a source file in the project, but to
// simplify things in this demo app, I have just included the .cpp file directly in here.
#include "gl3w.cpp"
// ================================================================================================

