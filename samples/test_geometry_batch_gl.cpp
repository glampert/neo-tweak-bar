
// ================================================================================================
// -*- C++ -*-
// File: test_geometry_batch_gl.cpp
// Author: Guilherme R. Lampert
// Created on: 29/08/16
//
// Brief:
//  Various tests for the underlaying NTB RenderInterfaces and GeometryBatch, using OpenGL.
//  --gl-core:   Runs in OpenGL Core Profile mode (GL 3+);
//  --gl-legacy: Runs in Legacy mode (OpenGL 2.0 or lower);
//  If no command line arguments are given, defaults to legacy mode.
// ================================================================================================

//FIXME: gl3w and legacy GL can't live side-by-side in the same file!
//       need to restructure. Maybe add a libSamples.a or something...

#include "ntb.hpp"
#include "ntb_widgets.hpp"

#include <GL/gl3w.h> // An OpenGL extension wrangler (https://github.com/skaslev/gl3w).
#include <GLFW/glfw3.h>
#include <vectormath.h>

#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define NTB_DEFAULT_RENDERER_GL_CORE
#include "ntb_renderer_gl_core.hpp"

//TODO temp
//#define NTB_DEFAULT_RENDERER_GL_LEGACY
//#include "ntb_renderer_gl_legacy.hpp"

#if !defined(NEO_TWEAK_BAR_STD_STRING_INTEROP)
    #error "NEO_TWEAK_BAR_STD_STRING_INTEROP is required for this sample!"
#endif // NEO_TWEAK_BAR_STD_STRING_INTEROP

// ========================================================
// MyNTBShellInterfaceGLFW:
// ========================================================

class MyNTBShellInterfaceGLFW : public ntb::ShellInterface
{
public:

    ~MyNTBShellInterfaceGLFW();
    ntb::Int64 getTimeMilliseconds() const NTB_OVERRIDE;
};

MyNTBShellInterfaceGLFW::~MyNTBShellInterfaceGLFW()
{ }

ntb::Int64 MyNTBShellInterfaceGLFW::getTimeMilliseconds() const
{
    const ntb::Float64 seconds = glfwGetTime();
    return static_cast<ntb::Int64>(seconds * 1000.0);
}

// ========================================================

static const int AppWindowWidth  = 1024;
static const int AppWindowHeight = 768;

struct AppContext
{
    GLFWwindow           * window;
    ntb::RenderInterface * renderInterface;
    ntb::ShellInterface  * shellInterface;
    bool                   coreProfile;
};

static GLFWwindow * appInitInternal(const int glVersionMajor, const int glVersionMinor,
                                    const bool coreProfile, const char * const title)
{
    std::printf("\nNTB sample \"%s\" starting up...\n", title);

    if (!glfwInit())
    {
        std::fprintf(stderr, "[APP_ERROR]: Failed to initialize GLFW!\n");
        return NTB_NULL;
    }

    // Things we need for the window / GL render context:
    glfwWindowHint(GLFW_RESIZABLE, false);
    glfwWindowHint(GLFW_DEPTH_BITS, 32);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glVersionMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glVersionMinor);

    if (coreProfile)
    {
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    GLFWwindow * window = glfwCreateWindow(AppWindowWidth, AppWindowHeight, title, NTB_NULL, NTB_NULL);
    if (window == NTB_NULL)
    {
        std::fprintf(stderr, "[APP_ERROR]: Failed to create GLFW window!\n");
        return NTB_NULL;
    }

    glfwMakeContextCurrent(window);

    if (coreProfile)
    {
        if (!gl3wInit())
        {
            std::fprintf(stderr, "[APP_WARNING]: Failed to initialize GL3W extensions library!\n");
        }
        if (!gl3wIsSupported(3, 2))
        {
            std::fprintf(stderr, "[APP_WARNING]: This sample application requires at least OpenGL version 3.2 to run!\n");
        }
    }

    std::printf("GL_VENDOR:    %s\n", glGetString(GL_VERSION));
    std::printf("GL_VERSION:   %s\n", glGetString(GL_VENDOR));
    std::printf("GLSL_VERSION: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    return window;
}

static AppContext appInit(const int argc, const char * argv[], const char * const title)
{
    AppContext ctx;
    std::memset(&ctx, 0, sizeof(ctx));

    int glVersionMajor = 2;
    int glVersionMinor = 0;
    std::string fullTitle = title;

    for (int i = 0; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--gl-core") == 0)
        {
            ctx.coreProfile = true;
            glVersionMajor  = 3;
            glVersionMinor  = 2;
            fullTitle      += " - Core OpenGL";
        }
        else if (std::strcmp(argv[i], "--gl-legacy") == 0)
        {
            ctx.coreProfile = false;
            glVersionMajor  = 2;
            glVersionMinor  = 0;
            fullTitle      += " - Legacy OpenGL";
        }
        else if (std::strcmp(argv[i], "--help") == 0)
        {
            std::printf("\nUsage:\n  $ %s [--gl-code | --gl-legacy | --help]\n", argv[0]);
        }
    }

    ctx.window = appInitInternal(glVersionMajor, glVersionMinor, ctx.coreProfile, fullTitle.c_str());
    if (ctx.window != NTB_NULL)
    {
        if (ctx.coreProfile)
        {
            std::printf("Attempting to initialize sample renderer with GL Core profile...\n");
            ctx.renderInterface = new ntb::RenderInterfaceDefaultGLCore(AppWindowWidth, AppWindowHeight);
        }
        else
        {
            std::printf("Attempting to initialize sample renderer with GL Legacy profile...\n");
            ctx.renderInterface = nullptr;//new ntb::RenderInterfaceDefaultGLLegacy(AppWindowWidth, AppWindowHeight); //TODO
        }

        ctx.shellInterface = new MyNTBShellInterfaceGLFW();
        std::printf("Done!\n\n");
    }

    return ctx;
}

static void appShutdown(AppContext & ctx)
{
    delete ctx.renderInterface;
    delete ctx.shellInterface;

    ctx.renderInterface = NTB_NULL;
    ctx.shellInterface  = NTB_NULL;
    ctx.window          = NTB_NULL;

    if (ctx.coreProfile)
    {
        gl3wShutdown();
    }

    glfwTerminate();
}

static void appFrameUpdate(AppContext & ctx, bool * outIsDone)
{
    // NTB starts writing at Z=0 and increases for each primitive.
    // Since we decide to draw without sorting, then the depth buffer
    // must be cleared to zero before drawing the UI.
    glClearDepth(0);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (outIsDone != NTB_NULL)
    {
        (*outIsDone) = glfwWindowShouldClose(ctx.window);
    }
}

static void appFramePresent(AppContext & ctx)
{
    glfwSwapBuffers(ctx.window);
    glfwPollEvents();
}

static void appMakeScreenProjectedBox(ntb::PODArray * scrProjectedVerts, ntb::PODArray * scrProjectedIndexes,
                                      const ntb::Mat4x4 & modelToWorldMatrix, const ntb::Mat4x4 & viewProjMatrix)
{
    ntb::BoxVert tempBoxVerts[24];
    ntb::UInt16  tempBoxIndexes[36];

    int viewport[4];
    ntb::Rectangle scrViewport;

    ntb::RenderInterface * renderer = ntb::getRenderInterface();
    renderer->getViewport(&viewport[0], &viewport[1], &viewport[2], &viewport[3]);
    scrViewport.set(viewport);

    const ntb::Float32 w          = 0.4f;
    const ntb::Float32 h          = 0.4f;
    const ntb::Float32 d          = 0.4f;
    const ntb::Color32 shadeColor = ntb::packColor(0, 0, 0, 255);
    const ntb::BoxVert * pVert    = tempBoxVerts;
    const ntb::UInt16  * pIndex   = tempBoxIndexes;
    const ntb::Int32 vertCount    = ntb::lengthOfArray(tempBoxVerts);
    const ntb::Int32 indexCount   = ntb::lengthOfArray(tempBoxIndexes);

    // Each face can be colored independently.
    static const ntb::Color32 tempFaceColors[6] = {
        ntb::packColor(255, 0,   0  ),
        ntb::packColor(0,   255, 0  ),
        ntb::packColor(0,   0,   255),
        ntb::packColor(255, 255, 0  ),
        ntb::packColor(0,   255, 255),
        ntb::packColor(255, 0,   255)
    };
    ntb::makeTexturedBoxGeometry(tempBoxVerts, tempBoxIndexes, tempFaceColors, w, h, d);

    scrProjectedVerts->allocateExact(vertCount);
    scrProjectedIndexes->allocateExact(indexCount);

    for (ntb::Int32 v = 0; v < vertCount; ++v, ++pVert)
    {
        const ntb::Vec3 wp = ntb::Mat4x4::transformPointAffine(pVert->position, modelToWorldMatrix);
        const ntb::Vec3 wn = ntb::Mat4x4::transformPointAffine(pVert->normal,   modelToWorldMatrix);
        const ntb::Color32 vertColor = ntb::blendColors(shadeColor, pVert->color, std::fabs(ntb::clamp(wn.z, -1.0f, 1.0f)));

        ntb::VertexPTC scrVert = { wp.x, wp.y, wp.z, pVert->u, pVert->v, vertColor };
        ntb::screenProjectionXY(scrVert, scrVert, viewProjMatrix, scrViewport);
        scrProjectedVerts->pushBack<ntb::VertexPTC>(scrVert);
    }

    for (ntb::Int32 i = 0; i < indexCount; ++i, ++pIndex)
    {
        scrProjectedIndexes->pushBack<ntb::UInt16>(*pIndex);
    }
}

// ========================================================

int main(const int argc, const char * argv[])
{
    AppContext ctx = appInit(argc, argv, "NTB GeometryBatch Test");
    if (ctx.window == NTB_NULL)
    {
        std::fprintf(stderr, "[APP_ERROR]: Failed to initialize sample app!\n");
        return EXIT_FAILURE;
    }

    ntb::initialize(ctx.shellInterface, ctx.renderInterface);
    {
        bool done = false;
        ntb::GeometryBatch geoBatch;
        ntb::TextureHandle sampleTex = ctx.renderInterface->createCheckerboardTexture(64, 64, 4);

        ntb::PODArray scrProjectedVerts(sizeof(ntb::VertexPTC));
        ntb::PODArray scrProjectedIndexes(sizeof(ntb::UInt16));

        ntb::Float32 rotationDegreesX = 0.0f;
        ntb::Float32 rotationDegreesZ = 0.0f;

        ntb::Mat4x4 modelToWorldMatrix;
        modelToWorldMatrix.setIdentity();

        while (!done)
        {
            appFrameUpdate(ctx, &done);
            geoBatch.beginDraw();

            //
            // Draw a textured quad without batching:
            //
            const ntb::Float32 batchZ    = geoBatch.getNextZ();
            const ntb::UInt16 indexes[]  = { 0, 1, 2, 2, 3, 0 };
            const ntb::VertexPTC verts[] =
            {
                { 10,  10,  batchZ, 0.0f, 0.0f, ntb::packColor(255, 0,   0)   },
                { 10,  200, batchZ, 0.0f, 1.0f, ntb::packColor(0,   255, 0)   },
                { 200, 200, batchZ, 1.0f, 1.0f, ntb::packColor(0,   0,   255) },
                { 200, 10,  batchZ, 0.0f, 1.0f, ntb::packColor(255, 255, 255) }
            };
            ctx.renderInterface->draw2DTriangles(verts, ntb::lengthOfArray(verts),
                                             indexes, ntb::lengthOfArray(indexes),
                                             sampleTex, ctx.renderInterface->getMaxZ());

            //
            // Now add some items to the GeometryBatch:
            //

            // Simple rectangles:
            geoBatch.drawRectOutline(ntb::makeRect(10, 250, 210, 450), ntb::packColor(255, 0, 0));
            geoBatch.drawRectFilled(ntb::makeRect(10, 500, 210, 700),  ntb::packColor(0, 255, 0));

            // Simple text string with a background box and outline:
            const char * hello = "Hello World!";
            const int helloLength = ntb::lengthOfString(hello);
            ntb::Rectangle textAlignBox = ntb::makeRect(10, 850, 500, 950);

            geoBatch.drawRectOutline(textAlignBox, ntb::packColor(255, 255, 0));
            geoBatch.drawRectFilled(textAlignBox.shrunk(10, 10), ntb::packColor(128, 200, 0));

            textAlignBox.moveBy(0, 25);
            geoBatch.drawTextConstrained(hello, helloLength, textAlignBox, textAlignBox, 2.0f,
                                         ntb::packColor(255, 255, 255), ntb::TextAlign::Center);

            // Block with all available characters in the built-in font:
            static const char allChars[] =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
                "abcdefghijklmnopqrstuvwxyz\n"
                "1234567890\n"
                "\"!`?\'.,;:()[]{}<>|/@\\^$-%+=#_&~*\n"
                "¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»\n"
                "¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙ\n"
                "ÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷\n"
                "øùúûüýþÿ\n";

            const int allCharsLength = ntb::lengthOfString(allChars);
            textAlignBox = ntb::makeRect(550, 50, 1500, 1000);

            // Large block of text:
            geoBatch.drawTextConstrained(allChars, allCharsLength, textAlignBox, textAlignBox,
                                         2.0f, ntb::packColor(255, 255, 255), ntb::TextAlign::Center);

            // Small block of text:
            geoBatch.drawTextConstrained(allChars, allCharsLength, textAlignBox.moveBy(0, 600), textAlignBox.moveBy(0, 600),
                                         1.0f, ntb::packColor(0, 200, 200), ntb::TextAlign::Center);

            // Text outline box:
            textAlignBox = ntb::makeRect(550, 50, 1500, 1000);
            geoBatch.drawRectOutline(textAlignBox.moveBy(0, -25), ntb::packColor(255, 255, 0));

            // Some screen-projected 3D geometry:
            ntb::Rectangle clipViewport;
            clipViewport.xMins = textAlignBox.xMins + 20;
            clipViewport.yMins = textAlignBox.yMaxs + 30;
            clipViewport.xMaxs = clipViewport.xMins + 500;
            clipViewport.yMaxs = clipViewport.yMins + 500;

            const ntb::Mat4x4 projMatrix =
                ntb::Mat4x4::perspective(ntb::degToRad(60.0f),
                                         clipViewport.getAspect(),
                                         0.5f, 100.0f);
            const ntb::Mat4x4 viewMatrix =
                ntb::Mat4x4::lookAt(ntb::makeVec3(0.0f, 0.0f, +1.0f),
                                    ntb::makeVec3(0.0f, 0.0f, -1.0f),
                                    ntb::makeVec3(0.0f, 1.0f,  0.0f));
            const ntb::Mat4x4 viewProjMatrix = ntb::Mat4x4::multiply(viewMatrix, projMatrix);

            scrProjectedVerts.clear();
            scrProjectedIndexes.clear();
            appMakeScreenProjectedBox(&scrProjectedVerts, &scrProjectedIndexes, modelToWorldMatrix, viewProjMatrix);

            // Rotate it:
            rotationDegreesX = ntb::normalizeAngle360(rotationDegreesX + 0.07f);
            rotationDegreesZ = ntb::normalizeAngle360(rotationDegreesZ + 0.07f);
            const ntb::Mat4x4 matRx = ntb::Mat4x4::rotationX(ntb::degToRad(rotationDegreesX));
            const ntb::Mat4x4 matRz = ntb::Mat4x4::rotationZ(ntb::degToRad(rotationDegreesZ));
            modelToWorldMatrix = ntb::Mat4x4::multiply(matRz, matRx);

            geoBatch.drawRectFilled(clipViewport, ntb::packColor(200, 200, 200));
            geoBatch.drawClipped2DTriangles(scrProjectedVerts.getData<ntb::VertexPTC>(), scrProjectedVerts.getSize(),
                                            scrProjectedIndexes.getData<ntb::UInt16>(), scrProjectedIndexes.getSize(),
                                            clipViewport, clipViewport);
            geoBatch.drawRectOutline(clipViewport.expanded(10, 10), ntb::packColor(255, 0, 0));

            // Finally, test some overlapping draws to make sure depth testing is working as expected.
            ntb::Rectangle box = ntb::makeRect(1200, 1000, 1400, 1200);
            geoBatch.drawRectFilled(box, ntb::packColor(255, 0, 0));
            geoBatch.drawRectFilled(box.moveBy(40, 40), ntb::packColor(0, 255, 0));
            geoBatch.drawRectFilled(box.moveBy(40, 40), ntb::packColor(0, 0, 255));
            geoBatch.drawRectFilled(box.moveBy(40, 40), ntb::packColor(255, 255, 255));
            geoBatch.drawRectOutline(box.shrunk(50, 50), ntb::packColor(0, 0, 0));
            geoBatch.drawArrowFilled(box.shrunk(80, 80), ntb::packColor(0, 200, 0), ntb::packColor(0, 0, 0), 1);

            geoBatch.endDraw();
            appFramePresent(ctx);
        }
    }

    appShutdown(ctx);
    ntb::shutdown();
}

// ================================================================================================
// GL3W is an OpenGL extension wrangler (https://github.com/skaslev/gl3w).
// This would ideally be built separately as a source file in the project, but to
// simplify things in this demo app, I have just included the .cpp file directly in here.
#include "gl3w.cpp"
// ================================================================================================

