
// ================================================================================================
// -*- C++ -*-
// File: sample_app_lib_gl_core.cpp
// Author: Guilherme R. Lampert
// Created on: 25/09/16
// Brief: Core OpenGL initialization for the samples library.
// ================================================================================================

#include "sample_app_lib.hpp"

// min/max macros on Windows.h wreak havoc...
#if defined(_WIN32) || defined(WIN32)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif // WIN32_LEAN_AND_MEAN
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif // NOMINMAX
#endif // WIN32

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
    std::int64_t getTimeMilliseconds() const override;
};

MyNTBShellInterfaceGLFW::~MyNTBShellInterfaceGLFW()
{ }

std::int64_t MyNTBShellInterfaceGLFW::getTimeMilliseconds() const
{
    const ntb::Float64 seconds = glfwGetTime();
    return static_cast<std::int64_t>(seconds * 1000.0);
}

static MyNTBShellInterfaceGLFW g_ntbShell;
static AppEventCallback        g_eventCb;
static AppContext *            g_eventAppCtx;
static void *                  g_eventCbUserData;

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

    std::printf("GL_VENDOR:    %s\n", glGetString(GL_VENDOR));
    std::printf("GL_VERSION:   %s\n", glGetString(GL_VERSION));
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

    g_eventCb         = nullptr;
    g_eventAppCtx     = nullptr;
    g_eventCbUserData = nullptr;
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

static void appGlCoreMousePositionCallbackGLFW(GLFWwindow * /*window*/, double xPos, double yPos)
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

static void appGlCoreMouseScrollCallbackGLFW(GLFWwindow * /*window*/, double xOffset, double yOffset)
{
    NTB_ASSERT(g_eventAppCtx != nullptr);
    NTB_ASSERT(g_eventCb != nullptr);

    AppEvent event{};
    event.type = AppEvent::MouseScroll;
    event.data.scroll[0] = static_cast<int>(xOffset);
    event.data.scroll[1] = static_cast<int>(yOffset);
    g_eventCb(event, g_eventCbUserData);
}

static void appGlCoreMouseButtonCallbackGLFW(GLFWwindow * /*window*/, const int button, const int action, int /*mods*/)
{
    NTB_ASSERT(g_eventAppCtx != nullptr);
    NTB_ASSERT(g_eventCb != nullptr);

    AppEvent event{};
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        const  std::int64_t doubleClickTimeMs = 350; // Milliseconds between clicks for a double click
        static std::int64_t lastClickTimeMs   = 0;   // Milliseconds of last mouse click

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

static void appGlCoreSetAppCallback(AppContext * ctx, AppEventCallback cb, void * userContext)
{
    g_eventCb         = cb;
    g_eventAppCtx     = ctx;
    g_eventCbUserData = userContext;

    // GLFW input callbacks:
    glfwSetScrollCallback(reinterpret_cast<GLFWwindow *>(ctx->windowHandle),      &appGlCoreMouseScrollCallbackGLFW);
    glfwSetCursorPosCallback(reinterpret_cast<GLFWwindow *>(ctx->windowHandle),   &appGlCoreMousePositionCallbackGLFW);
    glfwSetMouseButtonCallback(reinterpret_cast<GLFWwindow *>(ctx->windowHandle), &appGlCoreMouseButtonCallbackGLFW);
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
    outCtx->setAppCallback  = &appGlCoreSetAppCallback;
    outCtx->frameUpdate     = &appGlCoreFrameUpdate;
    outCtx->framePresent    = &appGlCoreFramePresent;
    outCtx->shutdown        = &appGlCoreShutdown;
    outCtx->windowWidth     = windowWidth;
    outCtx->windowHeight    = windowHeight;
    outCtx->isGlCoreProfile = true;

    glfwGetFramebufferSize(reinterpret_cast<GLFWwindow *>(windowHandle),
                           &outCtx->framebufferWidth, &outCtx->framebufferHeight);

    return true;
}

// ================================================================================================
// GL3W is an OpenGL extension wrangler (https://github.com/skaslev/gl3w).
// This would ideally be built separately as a source file in the project, but to
// simplify things in this demo app, I have just included the .cpp file directly in here.
#include "gl3w.cpp"
// ================================================================================================
