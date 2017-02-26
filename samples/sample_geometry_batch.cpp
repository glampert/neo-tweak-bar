
// ================================================================================================
// -*- C++ -*-
// File: sample_geometry_batch.cpp
// Author: Guilherme R. Lampert
// Created on: 29/08/16
//
// Brief:
//  Various tests for the underlaying NTB RenderInterfaces and GeometryBatch, using OpenGL.
//  --gl-core:   Runs in OpenGL Core Profile mode (GL 3+);
//  --gl-legacy: Runs in Legacy mode (OpenGL 2.0 or lower);
//  If no command line arguments are given, defaults to legacy mode.
// ================================================================================================

#include "ntb.hpp"
#include "ntb_widgets.hpp"
#include "sample_app_lib.hpp"

#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#if !defined(NEO_TWEAK_BAR_STD_STRING_INTEROP)
    #error "NEO_TWEAK_BAR_STD_STRING_INTEROP is required for this sample!"
#endif // NEO_TWEAK_BAR_STD_STRING_INTEROP

// ========================================================

static void makeScreenProjectedBox(ntb::PODArray * scrProjectedVerts, ntb::PODArray * scrProjectedIndexes,
                                   const ntb::Mat4x4 & modelToWorldMatrix, const ntb::Mat4x4 & viewProjMatrix)
{
    ntb::BoxVert  tempBoxVerts[24];
    std::uint16_t tempBoxIndexes[36];

    int viewport[4];
    ntb::Rectangle scrViewport;

    const ntb::RenderInterface & renderer = ntb::getRenderInterface();
    renderer.getViewport(&viewport[0], &viewport[1], &viewport[2], &viewport[3]);
    scrViewport.set(viewport);

    const ntb::Float32 w          = 0.4f;
    const ntb::Float32 h          = 0.4f;
    const ntb::Float32 d          = 0.4f;
    const ntb::Color32 shadeColor = ntb::packColor(0, 0, 0, 255);
    const ntb::BoxVert  * pVert   = tempBoxVerts;
    const std::uint16_t * pIndex  = tempBoxIndexes;
    const std::int32_t vertCount  = ntb::lengthOfArray(tempBoxVerts);
    const std::int32_t indexCount = ntb::lengthOfArray(tempBoxIndexes);

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

    for (std::int32_t v = 0; v < vertCount; ++v, ++pVert)
    {
        const ntb::Vec3 wp = ntb::Mat4x4::transformPointAffine(pVert->position, modelToWorldMatrix);
        const ntb::Vec3 wn = ntb::Mat4x4::transformPointAffine(pVert->normal,   modelToWorldMatrix);
        const ntb::Color32 vertColor = ntb::blendColors(shadeColor, pVert->color, std::fabs(ntb::clamp(wn.z, -1.0f, 1.0f)));

        ntb::VertexPTC scrVert = { wp.x, wp.y, wp.z, pVert->u, pVert->v, vertColor };
        ntb::screenProjectionXY(scrVert, scrVert, viewProjMatrix, scrViewport);
        scrProjectedVerts->pushBack<ntb::VertexPTC>(scrVert);
    }

    for (std::int32_t i = 0; i < indexCount; ++i, ++pIndex)
    {
        scrProjectedIndexes->pushBack<std::uint16_t>(*pIndex);
    }
}

// ========================================================

int main(const int argc, const char * argv[])
{
    AppContext ctx;
    if (!appInit(argc, argv, "NTB GeometryBatch Tests", 1024, 768, &ctx))
    {
        std::fprintf(stderr, "[APP_ERROR]: Failed to initialize sample app!\n");
        return EXIT_FAILURE;
    }

    ntb::initialize(ctx.shellInterface, ctx.renderInterface);
    {
        bool done = false;
        ntb::GeometryBatch geoBatch;
        ntb::TextureHandle sampleTex = ctx.renderInterface->createCheckerboardTexture(64, 64, 4);

        ntb::PODArray scrProjectedVerts{ sizeof(ntb::VertexPTC) };
        ntb::PODArray scrProjectedIndexes{ sizeof(std::uint16_t) };

        ntb::Float32 rotationDegreesX = 0.0f;
        ntb::Float32 rotationDegreesZ = 0.0f;

        ntb::Mat4x4 modelToWorldMatrix;
        modelToWorldMatrix.setIdentity();

        while (!done)
        {
            ctx.frameUpdate(&ctx, &done);
            geoBatch.beginDraw();

            //
            // Draw a textured quad without batching:
            //
            const ntb::Float32 batchZ     = geoBatch.getNextZ();
            const std::uint16_t indexes[] = { 0, 1, 2, 2, 3, 0 };
            const ntb::VertexPTC verts[]  =
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
            geoBatch.drawRectOutline(ntb::Rectangle{ 10, 250, 210, 450 }, ntb::packColor(255, 0, 0));
            geoBatch.drawRectFilled(ntb::Rectangle{ 10, 500, 210, 700 },  ntb::packColor(0, 255, 0));

            // Simple text string with a background box and outline:
            const char * hello = "Hello World!";
            const int helloLength = ntb::lengthOfString(hello);
            ntb::Rectangle textAlignBox{ 10, 850, 500, 950 };

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
            textAlignBox = ntb::Rectangle{ 550, 50, 1500, 1000 };

            // Large block of text:
            geoBatch.drawTextConstrained(allChars, allCharsLength, textAlignBox, textAlignBox,
                                         2.0f, ntb::packColor(255, 255, 255), ntb::TextAlign::Center);

            // Small block of text:
            geoBatch.drawTextConstrained(allChars, allCharsLength, textAlignBox.moveBy(0, 600), textAlignBox.moveBy(0, 600),
                                         1.0f, ntb::packColor(0, 200, 200), ntb::TextAlign::Center);

            // Text outline box:
            textAlignBox = ntb::Rectangle{ 550, 50, 1500, 1000 };
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
                ntb::Mat4x4::lookAt(ntb::Vec3{ 0.0f, 0.0f, +1.0f },
                                    ntb::Vec3{ 0.0f, 0.0f, -1.0f },
                                    ntb::Vec3{ 0.0f, 1.0f,  0.0f });
            const ntb::Mat4x4 viewProjMatrix = ntb::Mat4x4::multiply(viewMatrix, projMatrix);

            scrProjectedVerts.clear();
            scrProjectedIndexes.clear();
            makeScreenProjectedBox(&scrProjectedVerts, &scrProjectedIndexes, modelToWorldMatrix, viewProjMatrix);

            // Rotate it:
            rotationDegreesX = ntb::normalizeAngle360(rotationDegreesX + 0.07f);
            rotationDegreesZ = ntb::normalizeAngle360(rotationDegreesZ + 0.07f);
            const ntb::Mat4x4 matRx = ntb::Mat4x4::rotationX(ntb::degToRad(rotationDegreesX));
            const ntb::Mat4x4 matRz = ntb::Mat4x4::rotationZ(ntb::degToRad(rotationDegreesZ));
            modelToWorldMatrix = ntb::Mat4x4::multiply(matRz, matRx);

            geoBatch.drawRectFilled(clipViewport, ntb::packColor(200, 200, 200));
            geoBatch.drawClipped2DTriangles(scrProjectedVerts.getData<ntb::VertexPTC>(), scrProjectedVerts.getSize(),
                                            scrProjectedIndexes.getData<std::uint16_t>(), scrProjectedIndexes.getSize(),
                                            clipViewport, clipViewport);
            geoBatch.drawRectOutline(clipViewport.expanded(10, 10), ntb::packColor(255, 0, 0));

            // Finally, test some overlapping draws to make sure depth testing is working as expected.
            ntb::Rectangle box = ntb::Rectangle{ 1200, 1000, 1400, 1200 };
            geoBatch.drawRectFilled(box, ntb::packColor(255, 0, 0));
            geoBatch.drawRectFilled(box.moveBy(40, 40), ntb::packColor(0, 255, 0));
            geoBatch.drawRectFilled(box.moveBy(40, 40), ntb::packColor(0, 0, 255));
            geoBatch.drawRectFilled(box.moveBy(40, 40), ntb::packColor(255, 255, 255));
            geoBatch.drawRectOutline(box.shrunk(50, 50), ntb::packColor(0, 0, 0));
            geoBatch.drawArrowFilled(box.shrunk(80, 80), ntb::packColor(0, 200, 0), ntb::packColor(0, 0, 0), 1);

            geoBatch.endDraw();
            ctx.framePresent(&ctx);
        }
    }
    ctx.shutdown(&ctx);
    ntb::shutdown();
}

