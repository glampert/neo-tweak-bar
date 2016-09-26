
// ================================================================================================
// -*- C++ -*-
// File: sample_app_lib_gl_legacy.cpp
// Author: Guilherme R. Lampert
// Created on: 25/09/16
// Brief: Legacy OpenGL initialization for the samples library.
// ================================================================================================

#include "sample_app_lib.hpp"

#include <GLFW/glfw3.h>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define NTB_DEFAULT_RENDERER_GL_LEGACY
#include "ntb_renderer_gl_legacy.hpp"

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

static AppWindowHandle * appGlLegacyInitInternal(const int glVersionMajor, const int glVersionMinor, const char * const windowTitle,
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

    GLFWwindow * window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, nullptr, nullptr);
    if (window == nullptr)
    {
        std::fprintf(stderr, "[APP_ERROR]: Failed to create GLFW window!\n");
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);

    std::printf("GL_VENDOR:    %s\n", glGetString(GL_VERSION));
    std::printf("GL_VERSION:   %s\n", glGetString(GL_VENDOR));
    std::printf("GLSL_VERSION: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    std::printf("Attempting to initialize sample renderer with GL Legacy profile...\n");
    (*outRenderInterface) = new ntb::RenderInterfaceDefaultGLLegacy(windowWidth, windowHeight);

    return reinterpret_cast<AppWindowHandle *>(window);
}

static void appGlLegacyShutdown(AppContext * ctx)
{
    delete ctx->renderInterface;
    std::memset(ctx, 0, sizeof(*ctx));

    glfwTerminate();
}

static void appGlLegacyFrameUpdate(AppContext * ctx, bool * outIsDone)
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

static void appGlLegacyFramePresent(AppContext * ctx)
{
    glfwSwapBuffers(reinterpret_cast<GLFWwindow *>(ctx->windowHandle));
    glfwPollEvents();
}

bool appGlLegacyInit(const int glVersionMajor, const int glVersionMinor, const char * const windowTitle,
                     const int windowWidth, const int windowHeight, AppContext * outCtx)
{
    ntb::RenderInterface * renderInterface = nullptr;
    AppWindowHandle * windowHandle = appGlLegacyInitInternal(glVersionMajor, glVersionMinor, windowTitle,
                                                             windowWidth, windowHeight, &renderInterface);

    if (windowHandle == nullptr || renderInterface == nullptr)
    {
        return false;
    }

    outCtx->windowHandle    = windowHandle;
    outCtx->renderInterface = renderInterface;
    outCtx->shellInterface  = &g_ntbShell;
    outCtx->frameUpdate     = &appGlLegacyFrameUpdate;
    outCtx->framePresent    = &appGlLegacyFramePresent;
    outCtx->shutdown        = &appGlLegacyShutdown;
    outCtx->windowWidth     = windowWidth;
    outCtx->windowHeight    = windowHeight;
    outCtx->isGlCoreProfile = false;

    return true;
}

