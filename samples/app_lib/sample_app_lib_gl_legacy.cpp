
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
static AppEventCallback        g_eventCb;
static AppContext *            g_eventAppCtx;
static void *                  g_eventCbUserData;

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

    g_eventCb         = nullptr;
    g_eventAppCtx     = nullptr;
    g_eventCbUserData = nullptr;
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

static void appGlLegacyMousePositionCallbackGLFW(GLFWwindow * /*window*/, double xPos, double yPos)
{
    NTB_ASSERT(g_eventAppCtx != nullptr);
    NTB_ASSERT(g_eventCb != nullptr);

    int mx = static_cast<int>(xPos);
    int my = static_cast<int>(yPos);

    // Clamp to window bounds:
    if (mx > g_eventAppCtx->windowWidth)
    {
        mx = g_eventAppCtx->windowWidth;
    }
    else if (mx < 0)
    {
        mx = 0;
    }

    if (my > g_eventAppCtx->windowHeight)
    {
        my = g_eventAppCtx->windowHeight;
    }
    else if (my < 0)
    {
        my = 0;
    }

    int fbScaleX = 1;
    int fbScaleY = 1;
    const int diffW = g_eventAppCtx->framebufferWidth  - g_eventAppCtx->windowWidth;
    const int diffH = g_eventAppCtx->framebufferHeight - g_eventAppCtx->windowHeight;
    if (diffW > 0) { fbScaleX = g_eventAppCtx->framebufferWidth  / g_eventAppCtx->windowWidth;  }
    if (diffH > 0) { fbScaleY = g_eventAppCtx->framebufferHeight / g_eventAppCtx->windowHeight; }

    AppEvent event{};
    event.type = AppEvent::MouseMotion;
    event.data.pos[0] = mx * fbScaleX;
    event.data.pos[1] = my * fbScaleY;
    g_eventCb(event, g_eventCbUserData);
}

static void appGlLegacyMouseScrollCallbackGLFW(GLFWwindow * /*window*/, double xOffset, double yOffset)
{
    NTB_ASSERT(g_eventAppCtx != nullptr);
    NTB_ASSERT(g_eventCb != nullptr);

    AppEvent event{};
    event.type = AppEvent::MouseScroll;
    event.data.scroll[0] = static_cast<int>(xOffset);
    event.data.scroll[1] = static_cast<int>(yOffset);
    g_eventCb(event, g_eventCbUserData);
}

static void appGlLegacyMouseButtonCallbackGLFW(GLFWwindow * /*window*/, const int button, const int action, int /*mods*/)
{
    NTB_ASSERT(g_eventAppCtx != nullptr);
    NTB_ASSERT(g_eventCb != nullptr);

    AppEvent event{};
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        const  ntb::Int64 doubleClickTimeMs = 350; // Milliseconds between clicks for a double click
        static ntb::Int64 lastClickTimeMs   = 0;   // Milliseconds of last mouse click

        int clicks;
        if (action == GLFW_PRESS)
        {
            if ((g_ntbShell.getTimeMilliseconds() - lastClickTimeMs) <= doubleClickTimeMs)
            {
                clicks = 2;
                lastClickTimeMs = 0;
            }
            else
            {
                clicks = 1;
                lastClickTimeMs = g_ntbShell.getTimeMilliseconds();
            }
        }
        else // Button released.
        {
            clicks = -1;
        }

        event.type = AppEvent::MouseClickLeft;
        event.data.clicks = clicks;
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        event.type = AppEvent::MouseClickRight;
        event.data.clicks = (action == GLFW_PRESS) ? 1 : -1;
    }

    g_eventCb(event, g_eventCbUserData);
}

static void appGlLegacySetAppCallback(AppContext * ctx, AppEventCallback cb, void * userContext)
{
    g_eventCb         = cb;
    g_eventAppCtx     = ctx;
    g_eventCbUserData = userContext;

    // GLFW input callbacks:
    glfwSetScrollCallback(reinterpret_cast<GLFWwindow *>(ctx->windowHandle),      &appGlLegacyMouseScrollCallbackGLFW);
    glfwSetCursorPosCallback(reinterpret_cast<GLFWwindow *>(ctx->windowHandle),   &appGlLegacyMousePositionCallbackGLFW);
    glfwSetMouseButtonCallback(reinterpret_cast<GLFWwindow *>(ctx->windowHandle), &appGlLegacyMouseButtonCallbackGLFW);
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
    outCtx->setAppCallback  = &appGlLegacySetAppCallback;
    outCtx->frameUpdate     = &appGlLegacyFrameUpdate;
    outCtx->framePresent    = &appGlLegacyFramePresent;
    outCtx->shutdown        = &appGlLegacyShutdown;
    outCtx->windowWidth     = windowWidth;
    outCtx->windowHeight    = windowHeight;
    outCtx->isGlCoreProfile = false;

    glfwGetFramebufferSize(reinterpret_cast<GLFWwindow *>(windowHandle),
                           &outCtx->framebufferWidth, &outCtx->framebufferHeight);

    return true;
}

