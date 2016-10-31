
// ================================================================================================
// -*- C++ -*-
// File: ntb_widgets.cpp
// Author: Guilherme R. Lampert
// Created on: 25/04/16
// Brief: Widgets are the back-end UI elements/components of NTB.
// ================================================================================================

#include "ntb_widgets.hpp"
#include "ntb_tables.hpp"
#include "ntb_font.hpp"

//TODO add macro switch to optionally sort primitives in the batch by Z

namespace ntb
{

// ========================================================
// class GeometryBatch:
// ========================================================

GeometryBatch::GeometryBatch()
    : glyphTex(nullptr)
    , currentZ(0)
    , baseVertex2D(0)
    , baseVertexText(0)
    , baseVertexClipped(0)
    , linesBatch(sizeof(VertexPC))
    , verts2DBatch(sizeof(VertexPTC))
    , tris2DBatch(sizeof(UInt16))
    , textVertsBatch(sizeof(VertexPTC))
    , textTrisBatch(sizeof(UInt16))
    , drawClippedInfos(sizeof(DrawClippedInfo))
    , vertsClippedBatch(sizeof(VertexPTC))
    , trisClippedBatch(sizeof(UInt16))
{
    createGlyphTexture();
}

GeometryBatch::~GeometryBatch()
{
    if (glyphTex != nullptr)
    {
        getRenderInterface().destroyTexture(glyphTex);
        glyphTex = nullptr;
    }
}

void GeometryBatch::preallocateBatches(const int lines, const int quads,
                                       const int textGlyphs, const int drawClipped)
{
    linesBatch.allocate(lines * 2);          // 2 vertexes per line

    verts2DBatch.allocate(quads * 4);        // 4 vertexes per 2D quad
    tris2DBatch.allocate(quads  * 6);        // 6 indexes per 2D quad (2 tris)

    textVertsBatch.allocate(textGlyphs * 4); // 4 vertexes per glyph quadrilateral
    textTrisBatch.allocate(textGlyphs  * 6); // 6 indexes per glyph (2 tris)

    drawClippedInfos.allocate(drawClipped);
    vertsClippedBatch.allocate(drawClipped * 4);
    trisClippedBatch.allocate(drawClipped  * 6);
}

void GeometryBatch::beginDraw()
{
    // Ensure we are self consistent with begin/end calls!
    NTB_ASSERT(baseVertex2D == 0 && baseVertexText == 0 && baseVertexClipped == 0);
    NTB_ASSERT(linesBatch.isEmpty());
    NTB_ASSERT(verts2DBatch.isEmpty());
    NTB_ASSERT(tris2DBatch.isEmpty());
    NTB_ASSERT(textVertsBatch.isEmpty());
    NTB_ASSERT(textTrisBatch.isEmpty());
    NTB_ASSERT(drawClippedInfos.isEmpty());
    NTB_ASSERT(vertsClippedBatch.isEmpty());
    NTB_ASSERT(trisClippedBatch.isEmpty());

    RenderInterface & renderer = getRenderInterface();
    renderer.beginDraw();
    currentZ = 0;
}

void GeometryBatch::endDraw()
{
    RenderInterface & renderer = getRenderInterface();

    // Continue anyway if exceeded (assuming the error handled doesn't throw).
    // The result might be a glitchy draw with overlapping elements.
    if (++currentZ >= renderer.getMaxZ())
    {
        errorF("Max frame Z index exceeded! Provide a custom RenderInterface::getMaxZ()!");
        currentZ = renderer.getMaxZ() - 1;
    }

    if (!verts2DBatch.isEmpty() && !tris2DBatch.isEmpty())
    {
        renderer.draw2DTriangles(
            verts2DBatch.getData<VertexPTC>(), verts2DBatch.getSize(),
            tris2DBatch.getData<UInt16>(), tris2DBatch.getSize(),
            nullptr, currentZ); // untextured
    }

    if (!drawClippedInfos.isEmpty() && !vertsClippedBatch.isEmpty() && !trisClippedBatch.isEmpty())
    {
        renderer.drawClipped2DTriangles(
            vertsClippedBatch.getData<VertexPTC>(), vertsClippedBatch.getSize(),
            trisClippedBatch.getData<UInt16>(), trisClippedBatch.getSize(),
            drawClippedInfos.getData<DrawClippedInfo>(), drawClippedInfos.getSize(), currentZ);
    }

    if (!textVertsBatch.isEmpty() && !textTrisBatch.isEmpty())
    {
        renderer.draw2DTriangles(
            textVertsBatch.getData<VertexPTC>(), textVertsBatch.getSize(),
            textTrisBatch.getData<UInt16>(), textTrisBatch.getSize(),
            glyphTex, currentZ); // textured
    }

    if (!linesBatch.isEmpty())
    {
        renderer.draw2DLines(linesBatch.getData<VertexPC>(), linesBatch.getSize(), currentZ);
    }

    // Reset the batches:
    linesBatch.clear();
    verts2DBatch.clear();
    tris2DBatch.clear();
    textVertsBatch.clear();
    textTrisBatch.clear();
    drawClippedInfos.clear();
    vertsClippedBatch.clear();
    trisClippedBatch.clear();

    // And the base vertex offsets:
    baseVertex2D      = 0;
    baseVertexText    = 0;
    baseVertexClipped = 0;

    renderer.endDraw();
}

void GeometryBatch::drawClipped2DTriangles(const VertexPTC * verts, const int vertCount,
                                           const UInt16 * indexes, const int indexCount,
                                           const Rectangle & viewport, const Rectangle & clipBox)
{
    NTB_ASSERT(verts   != nullptr);
    NTB_ASSERT(indexes != nullptr);
    NTB_ASSERT(vertCount  > 0);
    NTB_ASSERT(indexCount > 0);

    DrawClippedInfo drawInfo;
    drawInfo.texture    = nullptr;
    drawInfo.viewportX  = viewport.getX();
    drawInfo.viewportY  = viewport.getY();
    drawInfo.viewportW  = viewport.getWidth();
    drawInfo.viewportH  = viewport.getHeight();
    drawInfo.clipBoxX   = clipBox.getX();
    drawInfo.clipBoxY   = clipBox.getY();
    drawInfo.clipBoxW   = clipBox.getWidth();
    drawInfo.clipBoxH   = clipBox.getHeight();
    drawInfo.firstIndex = trisClippedBatch.getSize();
    drawInfo.indexCount = indexCount;
    drawClippedInfos.pushBack<DrawClippedInfo>(drawInfo);

    for (int i = 0; i < indexCount; ++i)
    {
        NTB_ASSERT(indexes[i] < unsigned(vertCount));
        NTB_ASSERT(indexes[i] + baseVertexClipped <= UINT16_MAX);
        trisClippedBatch.pushBack<UInt16>(indexes[i] + baseVertexClipped);
    }
    baseVertexClipped += vertCount;

    const Float32 z = getNextZ();
    for (int v = 0; v < vertCount; ++v)
    {
        VertexPTC vert = verts[v];
        vert.z += z; // Note that we actually add to the existing Z value.
                     // This is required by the 3D objects being screen protected.

        vertsClippedBatch.pushBack<VertexPTC>(vert);
    }
}

void GeometryBatch::draw2DTriangles(const VertexPTC * verts, const int vertCount,
                                    const UInt16 * indexes, const int indexCount)
{
    NTB_ASSERT(verts   != nullptr);
    NTB_ASSERT(indexes != nullptr);
    NTB_ASSERT(vertCount  > 0);
    NTB_ASSERT(indexCount > 0);

    for (int i = 0; i < indexCount; ++i)
    {
        NTB_ASSERT(indexes[i] < unsigned(vertCount));
        NTB_ASSERT(indexes[i] + baseVertex2D <= UINT16_MAX);
        tris2DBatch.pushBack<UInt16>(indexes[i] + baseVertex2D);
    }
    baseVertex2D += vertCount;

    const Float32 z = getNextZ();
    for (int v = 0; v < vertCount; ++v)
    {
        VertexPTC vert = verts[v];
        vert.z = z; // Just overwriting is fine.
        verts2DBatch.pushBack<VertexPTC>(vert);
    }
}

void GeometryBatch::drawLine(const int xFrom, const int yFrom,
                             const int xTo, const int yTo,
                             const Color32 colorFrom,
                             const Color32 colorTo)
{
    const Float32 z = getNextZ();
    const VertexPC vertFrom =
    {
        static_cast<Float32>(xFrom),
        static_cast<Float32>(yFrom),
        z, colorFrom
    };
    const VertexPC vertTo =
    {
        static_cast<Float32>(xTo),
        static_cast<Float32>(yTo),
        z, colorTo
    };
    linesBatch.pushBack<VertexPC>(vertFrom);
    linesBatch.pushBack<VertexPC>(vertTo);
}

void GeometryBatch::drawLine(const int xFrom, const int yFrom,
                             const int xTo, const int yTo,
                             const Color32 color)
{
    drawLine(xFrom, yFrom, xTo, yTo, color, color);
}

void GeometryBatch::drawRectFilled(const Rectangle & rect, const Color32 c0,
                                   const Color32 c1, const Color32 c2, const Color32 c3)
{
    VertexPTC verts[4];

    verts[0].x = static_cast<Float32>(rect.xMins);
    verts[0].y = static_cast<Float32>(rect.yMins);
    verts[0].u = 0.0f;
    verts[0].v = 0.0f;
    verts[0].color = c0;

    verts[1].x = static_cast<Float32>(rect.xMins);
    verts[1].y = static_cast<Float32>(rect.yMaxs);
    verts[1].u = 0.0f;
    verts[1].v = 0.0f;
    verts[1].color = c1;

    verts[2].x = static_cast<Float32>(rect.xMaxs);
    verts[2].y = static_cast<Float32>(rect.yMins);
    verts[2].u = 0.0f;
    verts[2].v = 0.0f;
    verts[2].color = c2;

    verts[3].x = static_cast<Float32>(rect.xMaxs);
    verts[3].y = static_cast<Float32>(rect.yMaxs);
    verts[3].u = 0.0f;
    verts[3].v = 0.0f;
    verts[3].color = c3;

    static const UInt16 indexes[6] = { 0, 1, 2, 2, 1, 3 };
    draw2DTriangles(verts, lengthOfArray(verts), indexes, lengthOfArray(indexes));
}

void GeometryBatch::drawRectFilled(const Rectangle & rect, const Color32 color)
{
    drawRectFilled(rect, color, color, color, color);
}

void GeometryBatch::drawRectOutline(const Rectangle & rect, const Color32 c0,
                                    const Color32 c1, const Color32 c2, const Color32 c3)
{
    // CCW winding.
    drawLine(rect.xMins, rect.yMins, rect.xMins, rect.yMaxs, c0);
    drawLine(rect.xMins, rect.yMaxs, rect.xMaxs, rect.yMaxs, c1);
    drawLine(rect.xMaxs, rect.yMaxs, rect.xMaxs, rect.yMins, c2);
    drawLine(rect.xMaxs, rect.yMins, rect.xMins, rect.yMins, c3);
}

void GeometryBatch::drawRectOutline(const Rectangle & rect, const Color32 color)
{
    // CCW winding.
    drawLine(rect.xMins, rect.yMins, rect.xMins, rect.yMaxs, color);
    drawLine(rect.xMins, rect.yMaxs, rect.xMaxs, rect.yMaxs, color);
    drawLine(rect.xMaxs, rect.yMaxs, rect.xMaxs, rect.yMins, color);
    drawLine(rect.xMaxs, rect.yMins, rect.xMins, rect.yMins, color);
}

void GeometryBatch::drawRectShadow(const Rectangle & rect, const Color32 shadowColor,
                                   const Color32 penumbraColor, const int shadowOffset)
{
    const int posX = rect.xMins;
    const int posY = rect.yMins;
    const int w = posX + rect.getWidth();
    const int h = posY + rect.getHeight();
    const int wOffs = w + shadowOffset;

    // Shadow is made up of 5 quads, each with one corner for the shadow
    // color and the other 3 for the penumbra, which can be fully transparent.
    // The GPU will properly interpolate the colors and produce a nice gradient
    // effect. Note that the following draw order relies on a CCW polygon winding!

    // bottom-left
    drawRectFilled(Rectangle{ posX, h, posX + shadowOffset, h + shadowOffset },
                   penumbraColor, penumbraColor,
                   shadowColor, penumbraColor);

    // center-left
    drawRectFilled(Rectangle{ posX + shadowOffset, h, w, h + shadowOffset },
                   shadowColor, penumbraColor,
                   shadowColor, penumbraColor);

    // bottom-right
    drawRectFilled(Rectangle{ w, h, wOffs, h + shadowOffset },
                   shadowColor, penumbraColor,
                   penumbraColor, penumbraColor);

    // center-right
    drawRectFilled(Rectangle{ w, posY + shadowOffset, wOffs, h },
                   shadowColor, shadowColor,
                   penumbraColor, penumbraColor);

    // top-right
    drawRectFilled(Rectangle{ w, posY, wOffs, posY + shadowOffset },
                   penumbraColor, shadowColor,
                   penumbraColor, penumbraColor);
}

void GeometryBatch::drawArrowFilled(const Rectangle & rect, const Color32 bgColor,
                                    const Color32 outlineColor, const int direction)
{
    VertexPTC verts[3];
    verts[0].u = 0.0f;
    verts[0].v = 0.0f;
    verts[0].color = bgColor;
    verts[1].u = 0.0f;
    verts[1].v = 0.0f;
    verts[1].color = bgColor;
    verts[2].u = 0.0f;
    verts[2].v = 0.0f;
    verts[2].color = bgColor;

    if (direction == 1) // up
    {
        verts[0].x = static_cast<Float32>(rect.xMins + (rect.getWidth() / 2));
        verts[0].y = static_cast<Float32>(rect.yMins);
        verts[1].x = static_cast<Float32>(rect.xMins);
        verts[1].y = static_cast<Float32>(rect.yMaxs);
        verts[2].x = static_cast<Float32>(rect.xMaxs);
        verts[2].y = static_cast<Float32>(rect.yMaxs);
    }
    else // down
    {
        verts[0].x = static_cast<Float32>(rect.xMins);
        verts[0].y = static_cast<Float32>(rect.yMins);
        verts[1].x = static_cast<Float32>(rect.xMins + (rect.getWidth() / 2));
        verts[1].y = static_cast<Float32>(rect.yMaxs);
        verts[2].x = static_cast<Float32>(rect.xMaxs);
        verts[2].y = static_cast<Float32>(rect.yMins);
    }

    static const UInt16 indexes[3] = { 0, 1, 2 };
    draw2DTriangles(verts, lengthOfArray(verts), indexes, lengthOfArray(indexes));

    // Outline:
    drawLine(verts[0].x, verts[0].y, verts[1].x, verts[1].y, outlineColor);
    drawLine(verts[1].x, verts[1].y, verts[2].x, verts[2].y, outlineColor);
    drawLine(verts[2].x, verts[2].y, verts[0].x, verts[0].y, outlineColor);
}

void GeometryBatch::createGlyphTexture()
{
    UInt8 * decompressedBitmap = detail::decompressFontBitmap();
    if (decompressedBitmap == nullptr)
    {
        errorF("Unable to decompress the built-in font bitmap data!");
        return;
    }

    glyphTex = getRenderInterface().createTexture(
                     detail::getFontCharSet().bitmapWidth,
                     detail::getFontCharSet().bitmapHeight,
                     detail::getFontCharSet().bitmapColorChannels,
                     decompressedBitmap);

    // No longer needed.
    implFree(decompressedBitmap);
}

void GeometryBatch::drawTextConstrained(const char * text, const int textLength, Rectangle alignBox,
                                        const Rectangle & clipBox, const Float32 scaling, const Color32 color,
                                        const TextAlign align)
{
    NTB_ASSERT(text != nullptr);
    if (*text == '\0' || textLength <= 0)
    {
        return;
    }

    const Float32 charHeight   = getCharWidth() * scaling;
    const Float32 clipBoxWidth = clipBox.getWidth();

    Float32 textWidth = calcTextWidth(text, textLength, scaling);
    int clippedLength = textLength;

    while (textWidth > clipBoxWidth)
    {
        textWidth -= charHeight;
        clippedLength--;
    }

    if (clippedLength <= 0)
    {
        return; // The whole string was clipped.
    }

    if (textWidth > clipBoxWidth)
    {
        alignBox = clipBox;
    }

    Float32 x = alignBox.xMins;
    Float32 y = alignBox.yMins;

    for (;;)
    {
        if (align == TextAlign::Center)
        {
            x += alignBox.getWidth() * 0.5f;
            x -= textWidth * 0.5f;
        }
        else if (align == TextAlign::Right)
        {
            x += alignBox.getWidth();
            x -= textWidth;
        }

        if (x >= clipBox.xMins)
        {
            break;
        }
        alignBox = clipBox;
    }

    drawTextImpl(text, clippedLength, x, y, scaling, color);
}

void GeometryBatch::drawTextImpl(const char * text, const int textLength, Float32 x, Float32 y,
                                 const Float32 scaling, const Color32 color)
{
    NTB_ASSERT(text != nullptr);
    NTB_ASSERT(textLength > 0);

    // Invariants for all characters:
    const Float32 initialX      = x;
    const FontCharSet & charSet = detail::getFontCharSet();
    const Float32 charsZ        = getNextZ(); // Assume glyphs in a string never overlap, so share the Z index.
    const Float32 scaleU        = static_cast<Float32>(charSet.bitmapWidth);
    const Float32 scaleV        = static_cast<Float32>(charSet.bitmapHeight);
    const Float32 fixedWidth    = getCharWidth();  // Unscaled
    const Float32 fixedHeight   = getCharHeight(); // Unscaled
    const Float32 tabW          = fixedWidth  * 4.0f * scaling; // TAB = 4 spaces.
    const Float32 chrW          = fixedWidth  * scaling;
    const Float32 chrH          = fixedHeight * scaling;
    const UInt16 indexes[6]     = { 0, 1, 2, 2, 1, 3 };

    // These are necessary to avoid artefacts caused by texture
    // sampling between characters that draw close together. Values
    // tuned for the current font. Might need some tweaking for a new one!
    constexpr Float32 offsetU = +0.5f;
    constexpr Float32 offsetV = -0.5f;

    int increment;
    for (int c = 0; c < textLength; c += increment)
    {
        int charValue = text[c];
        increment = 1;

        if (charValue == ' ')
        {
            x += chrW;
            continue;
        }
        if (charValue == '\t')
        {
            x += tabW;
            continue;
        }
        if (charValue == '\n')
        {
            y += chrH;
            x  = initialX;
            continue;
        }

        // Not in the valid character range? Try to check for a multibyte character;
        // Output a question mark to signal the error if there is no conversion.
        if (charValue < 0 || charValue >= FontCharSet::MaxChars)
        {
            int lengthInBytes = 0;
            const int result  = decodeUtf8(&text[c], &lengthInBytes);
            if (result != -1)
            {
                charValue = result;
                increment = lengthInBytes;
            }
            else
            {
                charValue = '?';
            }
        }

        const FontChar fontChar = charSet.chars[charValue];
        const Float32 u0 = (fontChar.x + offsetU) / scaleU;
        const Float32 v0 = (fontChar.y + offsetV) / scaleV;
        const Float32 u1 = u0 + (fixedWidth  / scaleU);
        const Float32 v1 = v0 + (fixedHeight / scaleV);

        VertexPTC verts[4];
        verts[0].x = x;
        verts[0].y = y;
        verts[0].u = u0;
        verts[0].v = v0;
        verts[0].color = color;
        verts[1].x = x;
        verts[1].y = y + chrH;
        verts[1].u = u0;
        verts[1].v = v1;
        verts[1].color = color;
        verts[2].x = x + chrW;
        verts[2].y = y;
        verts[2].u = u1;
        verts[2].v = v0;
        verts[2].color = color;
        verts[3].x = x + chrW;
        verts[3].y = y + chrH;
        verts[3].u = u1;
        verts[3].v = v1;
        verts[3].color = color;

        for (int i = 0; i < 6; ++i)
        {
            NTB_ASSERT(indexes[i] + baseVertexText <= UINT16_MAX);
            textTrisBatch.pushBack<UInt16>(indexes[i] + baseVertexText);
        }
        for (int v = 0; v < 4; ++v)
        {
            verts[v].z = charsZ;
            textVertsBatch.pushBack<VertexPTC>(verts[v]);
        }

        baseVertexText += 4;
        x += chrW;
    }
}

Float32 GeometryBatch::calcTextWidth(const char * text, const int textLength, const Float32 scaling)
{
    NTB_ASSERT(text != nullptr);

    const Float32 fixedWidth = getCharWidth();
    const Float32 tabW = fixedWidth * 4.0f * scaling; // TAB = 4 spaces.
    const Float32 chrW = fixedWidth * scaling;

    Float32 x    = 0.0f;
    Float32 maxX = 0.0f;

    int increment;
    for (int c = 0; c < textLength; c += increment)
    {
        int charValue = text[c];
        increment = 1;

        // Tabs are handled differently (4 spaces)
        if (charValue == '\t')
        {
            x += tabW;
            continue;
        }
        if (charValue == '\n')
        {
            if (x > maxX)
            {
                maxX = x;
            }
            x = 0.0f;
            continue;
        }

        if (charValue < 0 || charValue >= FontCharSet::MaxChars)
        {
            int lengthInBytes = 0;
            const int result  = decodeUtf8(&text[c], &lengthInBytes);
            if (result != -1)
            {
                increment = lengthInBytes;
            }
        }

        // Non-tab char (including whitespace):
        x += chrW;
    }

    return std::max(x, maxX);
}

Float32 GeometryBatch::getCharWidth()
{
    return static_cast<Float32>(detail::getFontCharSet().charWidth);
}

Float32 GeometryBatch::getCharHeight()
{
    return static_cast<Float32>(detail::getFontCharSet().charHeight);
}

// ========================================================
// Misc local helpers:
// ========================================================

static void drawCheckMark(GeometryBatch & geoBatch, const Rectangle & rect, const Color32 color, const Float32 scaling)
{
    // Invariants for all triangles:
    static const UInt16 indexes[6] = { 0, 1, 2, 2, 1, 3 };
    VertexPTC verts[4];
    verts[0].u = 0.0f;
    verts[0].v = 0.0f;
    verts[0].color = color;
    verts[1].u = 0.0f;
    verts[1].v = 0.0f;
    verts[1].color = color;
    verts[2].u = 0.0f;
    verts[2].v = 0.0f;
    verts[2].color = color;
    verts[3].u = 0.0f;
    verts[3].v = 0.0f;
    verts[3].color = color;

    // Offsets are arbitrary. Decided via visual testing.
    const int halfW   = rect.getWidth() / 2;
    const int offset1 = Widget::uiScaleBy(2, scaling);
    const int offset2 = Widget::uiScaleBy(3, scaling);
    const int offset3 = Widget::uiScaleBy(6, scaling);
    const int offset4 = Widget::uiScaleBy(1, scaling);
    const int offset5 = Widget::uiScaleBy(4, scaling);

    // Large leg of the check mark to the right:
    verts[0].x = rect.xMaxs - offset1;
    verts[0].y = rect.yMins + offset4;
    verts[1].x = rect.xMins + halfW - offset1;
    verts[1].y = rect.yMaxs - offset1;
    verts[2].x = rect.xMaxs;
    verts[2].y = rect.yMins + offset2;
    verts[3].x = rect.xMins + halfW;
    verts[3].y = rect.yMaxs;
    geoBatch.draw2DTriangles(verts, lengthOfArray(verts), indexes, lengthOfArray(indexes));

    // Small leg to the left:
    verts[0].x = rect.xMins;
    verts[0].y = rect.yMins + offset3;
    verts[1].x = rect.xMins + halfW - offset1;
    verts[1].y = rect.yMaxs - offset1;
    verts[2].x = rect.xMins + offset1;
    verts[2].y = rect.yMins + offset5;
    verts[3].x = rect.xMins + halfW;
    verts[3].y = rect.yMaxs - offset5;
    geoBatch.draw2DTriangles(verts, lengthOfArray(verts), indexes, lengthOfArray(indexes));
}

static inline bool leftClick(const MouseButton button, const int clicks)
{
    return clicks > 0 && button == MouseButton::Left;
}

// ========================================================
// class ValueSlider:
// ========================================================

void ValueSlider::drawSelf(GeometryBatch & geoBatch, const Rectangle & displayBox, Color32 borderColor, Color32 fillColor)
{
    // Bring to the [0,1] range so it is simpler to work with the value.
    const Float64 scale = remap(currentVal, minVal, maxVal, 0.0, 1.0);

    sliderRect = displayBox;
    sliderRect.xMaxs = sliderRect.xMins + Widget::uiScaleBy(sliderRect.getWidth(), scale);
    geoBatch.drawRectFilled(sliderRect, fillColor);

    // Optional outline border
    if (borderColor != 0)
    {
        geoBatch.drawRectOutline(displayBox, borderColor);
        geoBatch.drawLine(sliderRect.xMaxs, sliderRect.yMins, sliderRect.xMaxs, sliderRect.yMaxs, borderColor);
    }
}

// ========================================================
// class Widget:
// ========================================================

Widget::Widget()
    : gui(nullptr)
    , parent(nullptr)
    , colors(nullptr)
    , children(sizeof(Widget *))
    , scaling(1.0f)
    , textScaling(1.0f)
    , flags(0)
{
    rect.setZero();
    lastMousePos.setZero();
}

Widget::~Widget()
{
    children.forEach<Widget *>(
        [](Widget * widget, void * /*unused*/)
        {
            if (widget->testFlag(Flag_NeedDeleting))
            {
                destroy(widget);
                implFree(widget);
            }
            return true;
        }, nullptr);
}

void Widget::init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible)
{
    NTB_ASSERT(myGUI != nullptr);

    gui    = myGUI;
    parent = myParent;
    rect   = myRect;

    lastMousePos.setZero();
    setFlag(Flag_Visible, visible);
    setNormalColors();
}

bool Widget::onKeyPressed(KeyCode /*key*/, KeyModFlags /*modifiers*/)
{
    // No key event handling by default. Each Widget defines its own.
    return false;
}

bool Widget::onMouseButton(MouseButton button, int clicks)
{
    // Obviously, hidden elements should not normally respond to mouse clicks.
    if (!isVisible())
    {
        return false;
    }

    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        if (getChild(c)->onMouseButton(button, clicks))
        {
            return true;
        }
    }

    // If the cursor is intersecting this element or any
    // of its children, we consume the mouse click, even
    // if it has no input effect in the UI.
    return isMouseIntersecting();
}

bool Widget::onMouseMotion(int mx, int my)
{
    // First, handle mouse drag:
    if (isMouseDragEnabled())
    {
        onMove(mx - lastMousePos.x, my - lastMousePos.y);
    }

    // Propagate the event to its children,
    // since they might overlap the parent.
    bool intersectingChildWidget = false;
    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        intersectingChildWidget |= getChild(c)->onMouseMotion(mx, my);
    }

    // Even if it intersected a child element, we want
    // to notify the parent as well in case its rect
    // falls under the mouse cursor too.

    if (rect.containsPoint(mx, my))
    {
        setHighlightedColors();
        setMouseIntersecting(true);
    }
    else
    {
        setNormalColors();
        setMouseIntersecting(false);
    }

    // Remember the mouse pointer position so we can
    // compute the displacement for mouse dragging.
    lastMousePos.x = mx;
    lastMousePos.y = my;
    return isMouseIntersecting() | intersectingChildWidget;
}

bool Widget::onMouseScroll(int /*yScroll*/)
{
    // No default scroll event handling.
    // Only the scroll bars / sliders use this.
    return false;
}

void Widget::onScrollContentUp()
{
    // Implemented by scroll bars/var widgets.
}

void Widget::onScrollContentDown()
{
    // Implemented by scroll bars/var widgets.
}

void Widget::onDraw(GeometryBatch & geoBatch) const
{
    drawSelf(geoBatch);
    drawChildren(geoBatch);
}

void Widget::onResize(int /*displacementX*/, int /*displacementY*/, Corner /*corner*/)
{
    // Widget is NOT resizeable by default.
    // Resizeable UI elements have to override this method.
}

void Widget::onMove(int displacementX, int displacementY)
{
    // Displacement may be positive or negative.
    rect.moveBy(displacementX, displacementY);
}

void Widget::onAdjustLayout()
{
    // Nothing done here at this level, just a default placeholder.
}

void Widget::onDisableEditing()
{
    if (parent != nullptr)
    {
        parent->onDisableEditing();
    }
}

void Widget::setMouseDragEnabled(bool enabled)
{
    setFlag(Flag_MouseDragEnabled, enabled);

    // Child elements move with the parent.
    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        getChild(c)->setMouseDragEnabled(enabled);
    }
}

void Widget::setNormalColors()
{
    //TODO this is a temporary for testing only. User should set this instead.
    static const ColorScheme test_colors_normal = {
        // box
        {
        packColor(100, 100, 100, 255),
        packColor(100, 100, 100, 255),
        packColor(100, 100, 100, 255),
        packColor(100, 100, 100, 255),
        packColor(000, 000, 000, 255), //top
        packColor(000, 000, 000, 255), //bottom
        packColor(80, 80, 80, 255),    //left
        packColor(000, 000, 000, 255), //right
        },
        // shadow
        {
        packColor(0, 0, 0, 128),
        packColor(0, 0, 0, 20),
        4,
        },
        // text
        {
        packColor(255, 255, 255, 255),
        packColor(255, 255, 255, 255),
        packColor(255, 255, 128, 255),
        },
        // listItem
        {
        packColor(80, 80, 80),
        packColor(110, 110, 110),
        packColor(0, 0, 0),
        packColor(180, 180, 180),
        },

        //checkmark
        packColor(0, 255, 0),
        packColor(255, 255, 255, 255),

        //scrollbar line
        packColor(50, 50, 50),
        packColor(80, 80, 80),

        // view3d outline / arrow,box:
        packColor(255, 255, 255),
        packColor(255, 255, 0),
        packColor(0, 128, 128),

        //resizeHandle
        packColor(255, 255, 255),
    };
    colors = &test_colors_normal;
}

void Widget::setHighlightedColors()
{
    //TODO this is a temporary for testing only. User should set this instead.
    static const ColorScheme test_colors_mousehover = {
        // box
        {
        packColor(110, 110, 110, 255),
        packColor(110, 110, 110, 255),
        packColor(110, 110, 110, 255),
        packColor(110, 110, 110, 255),
        packColor(000, 000, 000, 255), //top
        packColor(000, 000, 000, 255), //bottom
        packColor(80,  80,  80,  255), //left
        packColor(000, 000, 000, 255), //right
        },
        // shadow
        {
        packColor(0, 0, 0, 128),
        packColor(0, 0, 0, 20),
        4,
        },
        // text
        {
        packColor(255, 255, 255, 255),
        packColor(255, 255, 255, 255),
        packColor(255, 255, 128, 255),
        },
        // listItem
        {
        packColor(80, 80, 80),
        packColor(110, 110, 110),
        packColor(0, 0, 0),
        packColor(180, 180, 180),
        },

        //checkmark
        packColor(0, 255, 0),
        packColor(255, 255, 255, 255),

        //scrollbar line
        packColor(50, 50, 50),
        packColor(80, 80, 80),

        // view3d outline:
        packColor(255, 255, 255),
        packColor(255, 255, 0),
        packColor(0, 128, 128),

        //resizeHandle
        packColor(255, 255, 255),
    };
    colors = &test_colors_mousehover;
}

bool Widget::isChild(const Widget * widget) const
{
    if (widget == nullptr)
    {
        return false;
    }

    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        if (getChild(c) == widget)
        {
            return true;
        }
    }

    return false;
}

void Widget::drawSelf(GeometryBatch & geoBatch) const
{
    if (!isVisible())
    {
        return;
    }

    const ColorScheme & myColors = getColors();

    // Optional drop shadow effect under the element.
    if (myColors.shadow.dark != 0 && myColors.shadow.offset != 0 && !testFlag(Flag_NoRectShadow))
    {
        geoBatch.drawRectShadow(rect, myColors.shadow.dark,
                                      myColors.shadow.light,
                                      myColors.shadow.offset);
    }

    // Body box:
    geoBatch.drawRectFilled(rect, myColors.box.bgTopLeft,
                                  myColors.box.bgBottomLeft,
                                  myColors.box.bgTopRight,
                                  myColors.box.bgBottomRight);

    // Box outline/border:
    geoBatch.drawRectOutline(rect, myColors.box.outlineLeft,
                                   myColors.box.outlineBottom,
                                   myColors.box.outlineRight,
                                   myColors.box.outlineTop);
}

void Widget::drawChildren(GeometryBatch & geoBatch) const
{
    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        getChild(c)->onDraw(geoBatch);
    }
}

#if NEO_TWEAK_BAR_DEBUG
void Widget::printHierarchy(std::ostream & out, const SmallStr & indent) const
{
    out << indent.c_str() << getTypeString().c_str() << "\n";
    out << "|";

    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        SmallStr nextLevel(indent);
        nextLevel += "---";

        getChild(c)->printHierarchy(out, nextLevel);
    }
}
#endif // NEO_TWEAK_BAR_DEBUG

// ========================================================
// class ButtonWidget:
// ========================================================

ButtonWidget::ButtonWidget()
    : eventListener(nullptr)
    , icon(Icon::None)
    , state(false)
{
}

void ButtonWidget::init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible,
                        Icon myIcon, EventListener * myListener)
{
    Widget::init(myGUI, myParent, myRect, visible);
    eventListener = myListener;
    icon = myIcon;
}

void ButtonWidget::onDraw(GeometryBatch & geoBatch) const
{
    // Nothing to display?
    if (icon == Icon::None || !isVisible())
    {
        return;
    }

    // Draw the box background and outline, if any:
    Widget::onDraw(geoBatch);

    // A selected check box fully overrides the widget drawing logic.
    if (isCheckBoxButton() && state == true)
    {
        drawCheckMark(geoBatch, rect, getColors().checkMarkFill, textScaling * 2.0f);
        if (getColors().checkBoxBorder != 0)
        {
            geoBatch.drawRectOutline(rect, getColors().checkBoxBorder);
        }
    }
    else
    {
        // Text-based button icons:
        static const char * buttonIcons[] = {
            " ", // None (unused)
            "+", // Plus
            "-", // Minus
            "<", // LeftArrow
            ">", // RightArrow
            "«", // DblLeftArrow
            "»", // DblRightArrow
            "?", // QuestionMark
            "-"  // CheckMark (unselected)
        };
        static_assert(lengthOfArray(buttonIcons) == static_cast<int>(Icon::Count),
                      "Keep size in sync with Icon enum declaration!");

        Rectangle charBox{ rect };

        // Top/center offset
        const Float32 chrMid = GeometryBatch::getCharHeight() * textScaling * 0.5f;
        const Float32 boxMid = charBox.getHeight() * 0.5f;
        charBox.moveBy(0, boxMid - chrMid);

        geoBatch.drawTextConstrained(buttonIcons[static_cast<int>(icon)], 1, charBox, charBox,
                                     textScaling, getColors().text.normal, TextAlign::Center);

        // Still draw the border for an unselected check box.
        if (isCheckBoxButton() && getColors().checkBoxBorder != 0)
        {
            geoBatch.drawRectOutline(rect, getColors().checkBoxBorder);
        }
    }
}

bool ButtonWidget::onMouseButton(MouseButton button, int clicks)
{
    if (icon != Icon::None && isVisible() && isMouseIntersecting())
    {
        if (leftClick(button, clicks))
        {
            // Always toggle the button state.
            state = !state;

            // Fire the event if we have a listener.
            if (hasEventListener())
            {
                return eventListener->onButtonDown(*this);
            }
        }
    }
    return isMouseIntersecting();
}

// ========================================================
// class ButtonWidget::EventListener:
// ========================================================

bool ButtonWidget::EventListener::onButtonDown(ButtonWidget & /*button*/)
{
    return false; // Button event always ignored.
}

ButtonWidget::EventListener::~EventListener()
{
    // Defined here to anchor the vtable to this file. Do not remove.
}

// ========================================================
// class TitleBarWidget:
// ========================================================

void TitleBarWidget::init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible,
                          const char * myTitle, bool minimizeButton, bool maximizeButton,
                          int buttonOffsX, int buttonOffsY, int buttonSize, int buttonSpacing)
{
    Widget::init(myGUI, myParent, myRect, visible);

    initialHeight = myRect.getHeight();
    if (myTitle != nullptr)
    {
        titleText = myTitle;
    }

    buttonSetup(minimizeButton, maximizeButton, buttonOffsX, buttonOffsY, buttonSize, buttonSpacing);
}

void TitleBarWidget::buttonSetup(bool minimizeButton, bool maximizeButton,
                                 int buttonOffsX, int buttonOffsY,
                                 int buttonSize,  int buttonSpacing)
{
    Rectangle btnRect{
        rect.xMins + buttonOffsX,
        rect.yMins + buttonOffsY,
        rect.xMins + buttonOffsX + buttonSize,
        rect.yMins + buttonOffsY + buttonSize
    };

    if (minimizeButton)
    {
        buttons[BtnMinimize].init(getGUI(), this, btnRect, isVisible(), ButtonWidget::Icon::Minus, this);

        // init() can be called more than once, so check first.
        if (!isChild(&buttons[BtnMinimize]))
        {
            addChild(&buttons[BtnMinimize]);
        }
    }

    if (maximizeButton)
    {
        btnRect.xMins += buttonSize + buttonSpacing;
        btnRect.xMaxs += buttonSize + buttonSpacing;
        buttons[BtnMaximize].init(getGUI(), this, btnRect, isVisible(), ButtonWidget::Icon::Plus, this);

        if (!isChild(&buttons[BtnMaximize]))
        {
            addChild(&buttons[BtnMaximize]);
        }
    }
}

void TitleBarWidget::onDraw(GeometryBatch & geoBatch) const
{
    Widget::onDraw(geoBatch);

    if (titleText.isEmpty() || !isVisible())
    {
        return;
    }

    Rectangle textBox{ rect };

    // Top/center offset
    const Float32 chrMid = GeometryBatch::getCharHeight() * textScaling * 0.5f;
    const Float32 boxMid = textBox.getHeight() * 0.5f;
    textBox.moveBy(0, boxMid - chrMid);

    Rectangle clipBox{ textBox };
    if (buttons[BtnMinimize].getIcon() != ButtonWidget::Icon::None)
    {
        clipBox.xMins = buttons[BtnMinimize].getRect().xMaxs + Widget::uiScaled(4);
    }
    if (buttons[BtnMaximize].getIcon() != ButtonWidget::Icon::None)
    {
        clipBox.xMins = buttons[BtnMaximize].getRect().xMaxs + Widget::uiScaled(4);
    }

    geoBatch.drawTextConstrained(titleText.c_str(), titleText.getLength(), textBox, clipBox,
                                 textScaling, getColors().text.normal, TextAlign::Center);
}

bool TitleBarWidget::onMouseButton(MouseButton button, int clicks)
{
    if (!isVisible())
    {
        return false;
    }

    // A child button handled it first?
    if (buttons[BtnMinimize].onMouseButton(button, clicks) ||
        buttons[BtnMaximize].onMouseButton(button, clicks))
    {
        return true;
    }

    // If the mouse is currently over the title bar...
    if (isMouseIntersecting())
    {
        // And we have a left click, enable dragging.
        if (leftClick(button, clicks))
        {
            if (parent != nullptr)
            {
                parent->setMouseDragEnabled(true);
            }
            else
            {
                setMouseDragEnabled(true);
            }
        }
        else
        {
            if (parent != nullptr)
            {
                parent->setMouseDragEnabled(false);
            }
            else
            {
                setMouseDragEnabled(false);
            }
        }
        return true;
    }

    // Click didn't interact with the bar.
    return false;
}

void TitleBarWidget::onResize(int displacementX, int displacementY, Corner corner)
{
    // Title bar doesn't change height.
    switch (corner)
    {
    case TopLeft :
        rect.xMins += displacementX;
        rect.yMins += displacementY;
        rect.yMaxs = rect.yMins + getBarHeight();
        buttons[BtnMinimize].onMove(displacementX, displacementY);
        buttons[BtnMaximize].onMove(displacementX, displacementY);
        break;

    case BottomLeft :
        rect.xMins += displacementX;
        buttons[BtnMinimize].onMove(displacementX, 0);
        buttons[BtnMaximize].onMove(displacementX, 0);
        break;

    case TopRight :
        rect.xMaxs += displacementX;
        rect.yMins += displacementY;
        rect.yMaxs = rect.yMins + getBarHeight();
        buttons[BtnMinimize].onMove(0, displacementY);
        buttons[BtnMaximize].onMove(0, displacementY);
        break;

    case BottomRight :
        rect.xMaxs += displacementX;
        break;

    default :
        errorF("Bad corner enum in TitleBarWidget!");
        break;
    } // switch (corner)
}

bool TitleBarWidget::onButtonDown(ButtonWidget & button)
{
    if (&buttons[BtnMinimize] == &button)
    {
        //TODO button action!
        return true;
    }
    if (&buttons[BtnMaximize] == &button)
    {
        //TODO button action!
        return true;
    }
    return false;
}

void TitleBarWidget::setButtonTextScaling(Float32 s)
{
    for (int b = 0; b < BtnCount; ++b)
    {
        buttons[b].setTextScaling(s);
    }
}

// ========================================================
// class InfoBarWidget:
// ========================================================

void InfoBarWidget::init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible, const char * myText)
{
    Widget::init(myGUI, myParent, myRect, visible);

    initialHeight = myRect.getHeight();
    if (myText != nullptr)
    {
        setText(myText);
    }

    // Should not cast a shadow because the parent window already does so.
    setFlag(Flag_NoRectShadow, true);
}

void InfoBarWidget::onDraw(GeometryBatch & geoBatch) const
{
    Widget::onDraw(geoBatch);

    if (infoText.isEmpty() || !isVisible())
    {
        return;
    }

    Rectangle textBox{ rect };

    // Top/center offset
    const Float32 chrMid = GeometryBatch::getCharHeight() * textScaling * 0.5f;
    const Float32 boxMid = textBox.getHeight() * 0.5f;
    textBox.moveBy(Widget::uiScaled(2), boxMid - chrMid); // Slightly offset the text so that it doesn't touch the left border.

    geoBatch.drawTextConstrained(infoText.c_str(), infoText.getLength(), textBox, textBox,
                                 textScaling, getColors().text.informational, TextAlign::Left);
}

void InfoBarWidget::onResize(int displacementX, int displacementY, Corner corner)
{
    // Info bar doesn't change height.
    switch (corner)
    {
    case TopLeft :
        rect.xMins += displacementX;
        break;

    case BottomLeft :
        rect.xMins += displacementX;
        rect.yMins += displacementY;
        rect.yMaxs = rect.yMins + getBarHeight();
        break;

    case TopRight :
        rect.xMaxs += displacementX;
        break;

    case BottomRight :
        rect.xMaxs += displacementX;
        rect.yMins += displacementY;
        rect.yMaxs = rect.yMins + getBarHeight();
        break;

    default :
        errorF("Bad corner enum in InfoBarWidget!");
        break;
    } // switch (corner)
}

// ========================================================
// class ScrollBarWidget:
// ========================================================

ScrollBarWidget::ScrollBarWidget()
    : scrollBarOffsetY(0)
    , scrollBarDisplacement(0)
    , scrollBarSizeFactor(0)
    , scrollBarThickness(0)
    , scrollBarButtonSize(0)
    , scrollStartY(0)
    , scrollEndY(0)
    , accumulatedScrollSliderDrag(0)
    , totalLines(0)
    , linesOutOfView(0)
    , linesScrolledOut(0)
{
    upBtnRect.setZero();
    downBtnRect.setZero();
    barSliderRect.setZero();
    sliderClickInitialPos.setZero();
}

void ScrollBarWidget::init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible, int buttonSize)
{
    Widget::init(myGUI, myParent, myRect, visible);

    initialWidth = myRect.getWidth();
    scrollBarButtonSize = buttonSize;

    // Set up the scroll-bar rects:
    onAdjustLayout();

    // Should not cast a shadow because the parent window already does so.
    setFlag(Flag_NoRectShadow, true);
}

void ScrollBarWidget::onDraw(GeometryBatch & geoBatch) const
{
    if (!isVisible())
    {
        return;
    }

    // No child elements attached.
    drawSelf(geoBatch);

    // Window contents are not scrollable. Don't draw a bar slider or buttons.
    if (scrollBarSizeFactor <= 0)
    {
        return;
    }

    const ColorScheme & myColors = getColors();

    // Center lines taking the whole length of the scroll bar slider:
    const int lineX = rect.xMins + (rect.getWidth() / 2);
    geoBatch.drawLine(lineX - 1, scrollStartY, lineX - 1, scrollEndY, myColors.scrollBarCenterLine1);
    geoBatch.drawLine(lineX,     scrollStartY, lineX,     scrollEndY, myColors.scrollBarCenterLine2);
    geoBatch.drawLine(lineX + 1, scrollStartY, lineX + 1, scrollEndY, myColors.scrollBarCenterLine1);

    // Bar body/slider (50% lighter than the background):
    geoBatch.drawRectFilled(barSliderRect,
                            lighthenRGB(myColors.box.bgTopLeft,     50),
                            lighthenRGB(myColors.box.bgBottomLeft,  50),
                            lighthenRGB(myColors.box.bgTopRight,    50),
                            lighthenRGB(myColors.box.bgBottomRight, 50));

    // Bar outline/border (50% darker than the background's):
    geoBatch.drawRectOutline(barSliderRect,
                             darkenRGB(myColors.box.outlineLeft,   50),
                             darkenRGB(myColors.box.outlineBottom, 50),
                             darkenRGB(myColors.box.outlineRight,  50),
                             darkenRGB(myColors.box.outlineTop,    50));

    // Up and down arrow buttons:
    geoBatch.drawArrowFilled(upBtnRect,
                             lighthenRGB(myColors.box.bgTopLeft, 80),
                             darkenRGB(myColors.box.outlineTop,  80), 1); // up

    geoBatch.drawArrowFilled(downBtnRect,
                             lighthenRGB(myColors.box.bgBottomLeft, 80),
                             darkenRGB(myColors.box.outlineBottom,  80), -1); // down
}

bool ScrollBarWidget::onKeyPressed(KeyCode key, KeyModFlags modifiers)
{
    //TODO:
    // [HOME] and [END] keys should scroll all the way to the top and bottom!
    (void)key; (void)modifiers;
    return false;
}

bool ScrollBarWidget::onMouseButton(MouseButton button, int clicks)
{
    if (!isVisible())
    {
        return false;
    }

    setFlag(Flag_HoldingScrollSlider, false);

    if ((scrollBarSizeFactor > 0) && isMouseIntersecting() && leftClick(button, clicks))
    {
        if (barSliderRect.containsPoint(lastMousePos)) // Click, hold and move the slider.
        {
            sliderClickInitialPos = lastMousePos;
            setFlag(Flag_HoldingScrollSlider, true);
        }
        else if (upBtnRect.containsPoint(lastMousePos)) // Scroll up (-Y)
        {
            doScrollUp();
        }
        else if (downBtnRect.containsPoint(lastMousePos)) // Scroll down (+Y)
        {
            doScrollDown();
        }
    }

    return isMouseIntersecting();
}

bool ScrollBarWidget::onMouseMotion(int mx, int my)
{
    bool eventHandled = Widget::onMouseMotion(mx, my);

    if (testFlag(Flag_HoldingScrollSlider))
    {
        // Lower threshold and the scroll bar slider moves faster, but less precise.
        // Higher value makes it much "harder" to move, but gains precision.
        constexpr int threshold = 200;

        accumulatedScrollSliderDrag += my - sliderClickInitialPos.y;
        if (accumulatedScrollSliderDrag < -threshold)
        {
            doScrollUp();
            accumulatedScrollSliderDrag = 0;
        }
        else if (accumulatedScrollSliderDrag > threshold)
        {
            doScrollDown();
            accumulatedScrollSliderDrag = 0;
        }

        eventHandled = true;
    }
    else
    {
        accumulatedScrollSliderDrag = 0;
    }

    return eventHandled;
}

bool ScrollBarWidget::onMouseScroll(int yScroll)
{
    if (scrollBarSizeFactor <= 0)
    {
        return false; // No scrolling enabled.
    }

    if (yScroll > 0)
    {
        if (isMouseScrollInverted())
        {
            doScrollDown();
        }
        else
        {
            doScrollUp();
        }
        return true; // Handled the scroll event.
    }

    if (yScroll < 0)
    {
        if (isMouseScrollInverted())
        {
            doScrollUp();
        }
        else
        {
            doScrollDown();
        }
        return true; // Handled the scroll event.
    }

    return false;
}

void ScrollBarWidget::doScrollUp()
{
    if (barSliderRect.yMins <= scrollStartY)
    {
        return;
    }

    if (parent != nullptr)
    {
        parent->onScrollContentUp();
    }

    if ((barSliderRect.yMins - (scrollBarDisplacement * 2)) < scrollStartY) // Don't go out of bounds
    {
        scrollBarOffsetY -= scrollBarDisplacement;
        // Snap to to the beginning of the scroll area.
        scrollBarOffsetY += scrollStartY - (barSliderRect.yMins - scrollBarDisplacement);
    }
    else
    {
        scrollBarOffsetY -= scrollBarDisplacement;
    }

    --linesScrolledOut;
    barSliderRect = makeInnerBarRect();
}

void ScrollBarWidget::doScrollDown()
{
    if (barSliderRect.yMaxs >= scrollEndY)
    {
        return;
    }

    if (parent != nullptr)
    {
        parent->onScrollContentDown();
    }

    if ((barSliderRect.yMaxs + (scrollBarDisplacement * 2)) > scrollEndY) // Don't go out of bounds
    {
        scrollBarOffsetY += scrollBarDisplacement;
        // Snap to to the end of the scroll area.
        scrollBarOffsetY -= (barSliderRect.yMaxs + scrollBarDisplacement) - scrollEndY;
    }
    else
    {
        scrollBarOffsetY += scrollBarDisplacement;
    }

    ++linesScrolledOut;
    barSliderRect = makeInnerBarRect();
}

void ScrollBarWidget::onResize(int displacementX, int displacementY, Corner corner)
{
    // Scroll bar doesn't change width.
    switch (corner)
    {
    case TopLeft :
        rect.yMins += displacementY;
        break;

    case BottomLeft :
        rect.yMaxs += displacementY;
        break;

    case TopRight :
        rect.yMins += displacementY;
        rect.xMins += displacementX;
        rect.xMaxs = rect.xMins + getBarWidth();
        break;

    case BottomRight :
        rect.yMaxs += displacementY;
        rect.xMins += displacementX;
        rect.xMaxs = rect.xMins + getBarWidth();
        break;

    default :
        errorF("Bad corner enum in ScrollBarWidget!");
        break;
    } // switch (corner)

    onAdjustLayout();
}

void ScrollBarWidget::onAdjustLayout()
{
    if (linesOutOfView > 0)
    {
        // 4 seems to be the magic number here, not quite sure why...
        // If it gets below 4, things start to get, humm, weird...
        if ((totalLines - linesOutOfView) >= 4)
        {
            // map [0,totalLines] to [0,100] range:
            scrollBarSizeFactor = remap(totalLines - linesOutOfView, 0, totalLines, 0, 100);
        }
        else
        {
            scrollBarSizeFactor = remap(4, 0, totalLines, 0, 100);
        }
    }
    else
    {
        scrollBarSizeFactor   = 0;
        scrollBarDisplacement = 0;
    }

    scrollBarOffsetY   = 0;
    scrollBarThickness = Widget::uiScaleBy(rect.getWidth(), 0.6) / 2;

    upBtnRect    = makeUpButtonRect();
    downBtnRect  = makeDownButtonRect();
    scrollStartY = upBtnRect.yMaxs   + Widget::uiScaled(5);
    scrollEndY   = downBtnRect.yMins - Widget::uiScaled(5);

    if (linesOutOfView > 0)
    {
        const int sliderHeight = makeInnerBarRect().getHeight();
        scrollBarDisplacement  = (scrollEndY - scrollStartY - sliderHeight) / linesOutOfView;
        scrollBarOffsetY       = scrollBarDisplacement * linesScrolledOut;
    }

    // Now that we have the correct scrollBarOffsetY, rebuild the slider box:
    barSliderRect = makeInnerBarRect();
}

void ScrollBarWidget::updateLineScrollState(int lineCount, int linesOut)
{
    totalLines     = lineCount;
    linesOutOfView = linesOut;
    onAdjustLayout();
}

void ScrollBarWidget::onMove(int displacementX, int displacementY)
{
    Widget::onMove(displacementX, displacementY);

    upBtnRect.moveBy(displacementX, displacementY);
    downBtnRect.moveBy(displacementX, displacementY);
    barSliderRect.moveBy(displacementX, displacementY);

    scrollStartY = upBtnRect.yMaxs   + Widget::uiScaled(5);
    scrollEndY   = downBtnRect.yMins - Widget::uiScaled(5);
}

Rectangle ScrollBarWidget::makeInnerBarRect() const
{
    const int xMins  = rect.xMins + scrollBarThickness;
    const int xMaxs  = rect.xMaxs - scrollBarThickness;
    const int yMins  = scrollStartY + scrollBarOffsetY;
    const int height = scrollEndY - scrollStartY;

    int yMaxs = yMins + Widget::uiScaleBy(height, scrollBarSizeFactor * 0.01); // map [0,100] to [0,1] range
    if (yMaxs <= yMins)                                                        // So that it doesn't get too small.
    {
        yMaxs = yMins + Widget::uiScaled(4);
    }

    return { xMins, yMins, xMaxs, yMaxs };
}

Rectangle ScrollBarWidget::makeUpButtonRect() const
{
    const int topOffset = Widget::uiScaled(2);
    const int xMins = rect.xMins + scrollBarThickness;
    const int xMaxs = rect.xMaxs - scrollBarThickness;
    const int yMins = rect.yMins + topOffset;
    const int yMaxs = yMins + scrollBarButtonSize;

    return { xMins, yMins, xMaxs, yMaxs };
}

Rectangle ScrollBarWidget::makeDownButtonRect() const
{
    const int bottomOffset = Widget::uiScaled(18);
    const int xMins = rect.xMins + scrollBarThickness;
    const int xMaxs = rect.xMaxs - scrollBarThickness;
    const int yMins = rect.yMaxs - scrollBarButtonSize - bottomOffset;
    const int yMaxs = yMins + scrollBarButtonSize;

    return { xMins, yMins, xMaxs, yMaxs };
}

// ========================================================
// class ListWidget:
// ========================================================

ListWidget::ListWidget()
    : entries(sizeof(Entry))
    , selectedEntry(None)
    , hoveredEntry(None)
{
}

void ListWidget::init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible)
{
    Widget::init(myGUI, myParent, myRect, visible);
}

void ListWidget::onDraw(GeometryBatch & geoBatch) const
{
    Widget::onDraw(geoBatch);

    if (!isVisible())
    {
        return;
    }

    const ColorScheme & myColors = getColors();
    const int entryCount = entries.getSize();

    for (int e = 0; e < entryCount; ++e)
    {
        const Entry & entry = entries.get<Entry>(e);

        geoBatch.drawRectFilled(entry.rect, (e == selectedEntry) ?
                myColors.listItem.fillColorSelected : myColors.listItem.fillColorNormal);

        geoBatch.drawRectOutline(entry.rect, (e == hoveredEntry) ?
                myColors.listItem.outlineColorHovered : myColors.listItem.outlineColorNormal);

        Rectangle textBox{ entry.rect };

        // Top/center offset
        const Float32 chrMid = GeometryBatch::getCharHeight() * textScaling * 0.5f;
        const Float32 boxMid = textBox.getHeight() * 0.5f;
        textBox.moveBy(0, boxMid - chrMid);

        geoBatch.drawTextConstrained(strings.c_str() + entry.firstChar, entry.lengthInChars, textBox,
                                     textBox, textScaling, myColors.text.alternate, TextAlign::Center);
    }
}

void ListWidget::onMove(int displacementX, int displacementY)
{
    Widget::onMove(displacementX, displacementY);

    const int entryCount = entries.getSize();
    for (int e = 0; e < entryCount; ++e)
    {
        Entry & entry = entries.get<Entry>(e);
        entry.rect.moveBy(displacementX, displacementY);
    }
}

bool ListWidget::onMouseButton(MouseButton button, int clicks)
{
    bool eventHandled = Widget::onMouseButton(button, clicks);

    // Find if one of the entries was clicked:
    if (isMouseIntersecting())
    {
        const int index = findEntryForPoint(lastMousePos.x, lastMousePos.y);
        if (index != None)
        {
            selectedEntry = index;
            eventHandled  = true;
        }
    }

    return eventHandled;
}

bool ListWidget::onMouseMotion(int mx, int my)
{
    bool eventHandled = Widget::onMouseMotion(mx, my);

    // Check for intersection with the entries to highlight the hovered item:
    if (isMouseIntersecting())
    {
        hoveredEntry = findEntryForPoint(mx, my);
        if (hoveredEntry != None)
        {
            eventHandled = true;
        }
    }
    else
    {
        hoveredEntry = None;
    }

    return eventHandled;
}

int ListWidget::findEntryForPoint(int x, int y) const
{
    const int entryCount = entries.getSize();
    for (int e = 0; e < entryCount; ++e)
    {
        const Entry & entry = entries.get<Entry>(e);
        if (entry.rect.containsPoint(x, y))
        {
            return e;
        }
    }
    return None;
}

void ListWidget::allocEntries(int count)
{
    strings.clear();
    entries.clear();
    entries.resize(count);
    entries.zeroFill();
    selectedEntry = None;
    hoveredEntry  = None;
}

void ListWidget::addEntryText(int index, const char * value)
{
    Entry & entry       = entries.get<Entry>(index);
    entry.firstChar     = strings.getLength();
    entry.lengthInChars = lengthOfString(value);

    addEntryRect(index, entry.lengthInChars);
    strings.append(value, entry.lengthInChars);
}

void ListWidget::addEntryRect(int entryIndex, int entryLengthInChars)
{
    const int spacing     = Widget::uiScaled(4); // Hardcoded for now.
    const int entryHeight = (GeometryBatch::getCharHeight() * textScaling) + spacing;
    const int entryWidth  = (GeometryBatch::getCharWidth()  * textScaling  * entryLengthInChars) + (spacing * 2);

    Rectangle newRect = rect.shrunk(spacing, spacing);
    newRect.yMins += (entryHeight + spacing) * entryIndex;
    newRect.yMaxs  = newRect.yMins + entryHeight;
    newRect.xMaxs  = newRect.xMins + entryWidth;
    entries.get<Entry>(entryIndex).rect = newRect;

    // Expand the background window if needed:
    if (newRect.xMaxs > rect.xMaxs)
    {
        rect.xMaxs = newRect.xMaxs + spacing;
    }
    if (newRect.yMaxs > rect.yMaxs)
    {
        rect.yMaxs = newRect.yMaxs + spacing;
    }

    // Adjust the remaining rectangles to match the widest one:
    int e, widest = 0;
    const int entryCount = entries.getSize();
    for (e = 0; e < entryCount; ++e)
    {
        const Entry & entry = entries.get<Entry>(e);
        if (entry.rect.xMaxs > widest)
        {
            widest = entry.rect.xMaxs;
        }
    }
    for (e = 0; e < entryCount; ++e)
    {
        entries.get<Entry>(e).rect.xMaxs = widest;
    }
}

// ========================================================
// class ColorPickerWidget:
// ========================================================

ColorPickerWidget::ColorPickerWidget()
    : selectedColorIndex(None)
    , colorButtonLinesScrolledUp(0)
{
    usableRect.setZero();
}

void ColorPickerWidget::init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible,
                             int titleBarHeight, int titleBarButtonSize, int scrollBarWidth,
                             int scrollBarButtonSize, int clrButtonSize)
{
    Widget::init(myGUI, myParent, myRect, visible);
    colorButtonSize = clrButtonSize;

    // Vertical scroll bar (right side):
    Rectangle barRect{ rect.xMaxs - scrollBarWidth, rect.yMins + titleBarHeight + Widget::uiScaled(1), rect.xMaxs, rect.yMaxs };
    scrollBar.init(myGUI, this, barRect, visible, scrollBarButtonSize);

    // Title bar:
    barRect.set(rect.xMins, rect.yMins, rect.xMaxs, rect.yMins + titleBarHeight);
    titleBar.init(myGUI, this, barRect, visible, "Color Picker", true, false, Widget::uiScaled(4), Widget::uiScaled(4), titleBarButtonSize, 0);

    addChild(&scrollBar);
    addChild(&titleBar);
    refreshUsableRect();

    // Adjust the scroll bar bar according to the content window size:
    const int colorButtonCount  = lengthOfArray(detail::g_colorTable);
    const int gapBetweenButtons = Widget::uiScaled(4);
    const int maxButtonsPerLine = 7;

    const int btnSize   = colorButtonSize  + gapBetweenButtons;
    const int lineCount = colorButtonCount / maxButtonsPerLine;

    int linesScrolledOut;
    if ((lineCount * btnSize) > usableRect.getHeight())
    {
        linesScrolledOut = std::ceil(((lineCount * btnSize) - usableRect.getHeight()) / static_cast<Float64>(colorButtonSize));
    }
    else
    {
        linesScrolledOut = 0;
    }

    scrollBar.updateLineScrollState(lineCount, linesScrolledOut);
    colorButtonLinesScrolledUp = 0;
    selectedColorIndex = None;

    // The color table colors are initially sorted by
    // name but grouping similar colors together looks
    // better in the window. This is only done once for
    // when the first ColorPickerWidget is created. You
    // can also disable this code completely if you don't
    // care about that.
    #if NEO_TWEAK_BAR_SORT_COLORTABLE
    auto colorSortPredicate = [](const detail::NamedColor a, const detail::NamedColor b) -> bool
    {
        UInt8 alpha;
        UInt8 aR, aG, aB;
        UInt8 bR, bG, bB;
        unpackColor(a.value, aR, aG, aB, alpha);
        unpackColor(b.value, bR, bG, bB, alpha);

        Float32 aH, aL, aS;
        Float32 bH, bL, bS;
        RGBToHLS(byteToFloat(aR), byteToFloat(aG), byteToFloat(aB), aH, aL, aS);
        RGBToHLS(byteToFloat(bR), byteToFloat(bG), byteToFloat(bB), bH, bL, bS);

        // NOTE: Sorting by Hue is not very accurate, but more or less
        // bunches similar colors together. Combining the other components
        // doesn't provide much better results either.
        return aH > bH;
    };
    if (!detail::g_colorTableSorted)
    {
        std::sort(std::begin(detail::g_colorTable), std::end(detail::g_colorTable), colorSortPredicate);
        detail::g_colorTableSorted = true;
    }
    #endif // NEO_TWEAK_BAR_SORT_COLORTABLE
}

void ColorPickerWidget::onDraw(GeometryBatch & geoBatch) const
{
    if (!isVisible())
    {
        return;
    }

    Widget::onDraw(geoBatch);
    forEachColorButton(&ColorPickerWidget::drawColorButton, &geoBatch);
}

void ColorPickerWidget::onMove(int displacementX, int displacementY)
{
    Widget::onMove(displacementX, displacementY);
    usableRect.moveBy(displacementX, displacementY);
}

bool ColorPickerWidget::onButtonDown(ButtonWidget & /*button*/)
{
    //TODO handle the close button at the top bar
    return false;
}

bool ColorPickerWidget::onMouseButton(MouseButton button, int clicks)
{
    if (isMouseIntersecting() && leftClick(button, clicks))
    {
        if (forEachColorButton(&ColorPickerWidget::testColorButtonClick, nullptr))
        {
            //TODO handle color selection!
            titleBar.setTitle(detail::g_colorTable[selectedColorIndex].name);
            return true; // Got a button click.
        }
    }
    return Widget::onMouseButton(button, clicks);
}

bool ColorPickerWidget::onMouseMotion(int mx, int my)
{
    // Prevent it from being dragged out the top of the screen:
    int clampedY = my;
    if (isMouseDragEnabled())
    {
        const int displacementY = my - lastMousePos.y;
        if ((rect.yMins + displacementY) < 0)
        {
            clampedY = my - (rect.yMins + displacementY);
        }
    }
    return Widget::onMouseMotion(mx, clampedY);
}

bool ColorPickerWidget::onMouseScroll(int yScroll)
{
    // Only scroll if the mouse is hovering this window!
    if (scrollBar.isVisible() && isMouseIntersecting())
    {
        return scrollBar.onMouseScroll(yScroll);
    }
    return false;
}

void ColorPickerWidget::onScrollContentUp()
{
    --colorButtonLinesScrolledUp;
}

void ColorPickerWidget::onScrollContentDown()
{
    ++colorButtonLinesScrolledUp;
}

void ColorPickerWidget::refreshUsableRect()
{
    const int offset = Widget::uiScaled(5); // "Magic" offset for the color picker

    usableRect = rect;
    usableRect.xMins += offset;
    usableRect.xMaxs -= scrollBar.getRect().getWidth();
    usableRect.yMins += titleBar.getRect().getHeight() + offset;
    usableRect.yMaxs -= offset;
}

bool ColorPickerWidget::drawColorButton(Rectangle colorRect, int colorIndex, GeometryBatch * pGeoBatch) const
{
    NTB_ASSERT(pGeoBatch != nullptr);
    const ColorScheme & myColors = getColors();

    // Optional drop shadow effect under the color button:
    if (myColors.shadow.dark != 0 && myColors.shadow.offset != 0)
    {
        const int shadowOffset = ((colorIndex != selectedColorIndex) ?
                                   std::max(myColors.shadow.offset - 1, 0) :
                                   myColors.shadow.offset + 2);

        pGeoBatch->drawRectShadow(colorRect, myColors.shadow.dark,
                                  myColors.shadow.light, shadowOffset);
    }

    // The button box:
    if (detail::g_colorTable[colorIndex].value == 0) // ZeroAlpha (null color), draw an [X]:
    {
        if (colorIndex == selectedColorIndex) // Highlight if selected
        {
            colorRect = colorRect.expanded(Widget::uiScaled(2), Widget::uiScaled(2));
        }

        pGeoBatch->drawRectFilled(colorRect, packColor(0, 0, 0));

        const Color32 outlineColor = packColor(255, 255, 255);
        pGeoBatch->drawLine(colorRect.xMins, colorRect.yMins,
                            colorRect.xMaxs, colorRect.yMaxs,
                            outlineColor);
        pGeoBatch->drawLine(colorRect.xMaxs, colorRect.yMins,
                            colorRect.xMins, colorRect.yMaxs,
                            outlineColor);
        pGeoBatch->drawRectOutline(colorRect, outlineColor);
    }
    else // Opaque color, draw filled:
    {
        if (colorIndex == selectedColorIndex)
        {
            colorRect = colorRect.expanded(Widget::uiScaled(2), Widget::uiScaled(2));
        }
        pGeoBatch->drawRectFilled(colorRect, detail::g_colorTable[colorIndex].value);
    }

    // Continue till the end or window filled with buttons.
    return false;
}

bool ColorPickerWidget::testColorButtonClick(Rectangle colorRect, int colorIndex, GeometryBatch * /*pUnused*/) const
{
    if (colorRect.containsPoint(lastMousePos))
    {
        selectedColorIndex = colorIndex;
        return true;
    }
    return false; // Continue to the next button.
}

bool ColorPickerWidget::forEachColorButton(ButtonFunc pFunc, GeometryBatch * pGeoBatch) const
{
    // We have one small box/button for each color in the table.
    const int colorButtonCount  = lengthOfArray(detail::g_colorTable);
    const int gapBetweenButtons = Widget::uiScaled(4);
    const int maxButtonsPerLine = 7;

    int colorIndex   = colorButtonLinesScrolledUp * maxButtonsPerLine;
    int colorButtonX = usableRect.xMins;
    int colorButtonY = usableRect.yMins;
    int buttonsInCurrLine = 0;

    for (; colorIndex < colorButtonCount; ++colorIndex)
    {
        const Rectangle colorRect{ colorButtonX, colorButtonY,
                                   colorButtonX + colorButtonSize,
                                   colorButtonY + colorButtonSize };

        const bool shouldStop = (this->*pFunc)(colorRect, colorIndex, pGeoBatch);
        if (shouldStop)
        {
            return true;
        }

        colorButtonX += colorButtonSize + gapBetweenButtons;
        ++buttonsInCurrLine;

        if (buttonsInCurrLine == maxButtonsPerLine)
        {
            buttonsInCurrLine = 0;
            colorButtonX = usableRect.xMins;
            colorButtonY += colorButtonSize + gapBetweenButtons;

            if ((colorButtonY + colorButtonSize) > usableRect.yMaxs)
            {
                break; // Already filled the ColorPicker window. Stop.
            }
        }
    }

    return false; // Was not interrupted by the callback.
}

// ========================================================
// class View3DWidget:
// ========================================================

View3DWidget::View3DWidget()
    : mouseSensitivity(0.5f)
    , maxMouseDelta(20)
    , invertMouseY(false)
    , leftMouseButtonDown(false)
    , interactiveControls(true)
    , showXyzLabels(true)
    , object(ObjectType::None)
    , updateScrGeometry(true)
    , resettingAngles(false)
    , prevFrameTimeMs(0)
    , scrProjectedVerts(sizeof(VertexPTC))
    , scrProjectedIndexes(sizeof(UInt16))
    , projParams()
{
    mouseDelta.setZero();
    rotationDegrees.setZero();
    resetAnglesBtnRect.setZero();
}

void View3DWidget::init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible,
                        const char * myTitle, int titleBarHeight, int titleBarButtonSize,
                        int resetAnglesBtnSize, const ProjectionParameters & proj, ObjectType obj)
{
    Widget::init(myGUI, myParent, myRect, visible);

    updateScrGeometry = true;
    projParams = proj;
    object = obj;

    // Title bar is optional in this widget, so we can also use it as a component attached
    // to a WindowWidget or as a standalone popup-like window when a title/top-bar is provided.
    if (myTitle != nullptr)
    {
        const Rectangle barRect{ rect.xMins, rect.yMins,
                                 rect.xMaxs, rect.yMins + titleBarHeight };

        titleBar.init(myGUI, this, barRect, visible, myTitle, true, false,
                      Widget::uiScaled(4), Widget::uiScaled(4), titleBarButtonSize, Widget::uiScaled(4));
    }
    else
    {
        titleBar.init(myGUI, this, Rectangle{ 0, 0, 0, 0 }, false, nullptr, false, false, 0, 0, 0, 0);
    }

    addChild(&titleBar);
    refreshProjectionViewport();

    // Need to be cached for the click tests.
    const Float32 chrW = GeometryBatch::getCharWidth()  * textScaling;
    const Float32 chrH = GeometryBatch::getCharHeight() * textScaling;
    resetAnglesBtnRect.xMins = projParams.viewport.xMins + resetAnglesBtnSize;
    resetAnglesBtnRect.yMins = projParams.viewport.yMaxs - resetAnglesBtnSize - chrH;
    resetAnglesBtnRect.xMaxs = resetAnglesBtnRect.xMins + chrW + resetAnglesBtnSize;
    resetAnglesBtnRect.yMaxs = resetAnglesBtnRect.yMins + chrH + resetAnglesBtnSize;

    // Preallocate the vertex caches:
    int vertCount;
    switch (object)
    {
    case ObjectType::Sphere :
        vertCount  = lengthOfArray(detail::g_sphereVerts);
        vertCount += lengthOfArray(detail::g_arrowVerts) * 3;
        scrProjectedVerts.allocateExact(vertCount);
        scrProjectedIndexes.allocateExact(vertCount);
        break;

    case ObjectType::Arrow :
        vertCount = lengthOfArray(detail::g_arrowVerts);
        scrProjectedVerts.allocateExact(vertCount);
        scrProjectedIndexes.allocateExact(vertCount);
        break;

    case ObjectType::Box :
        scrProjectedVerts.allocateExact(24);
        scrProjectedIndexes.allocateExact(36);
        break;

    default:
        break;
    } // switch (object)
}

void View3DWidget::onDraw(GeometryBatch & geoBatch) const
{
    const Int64 currentTimeMs = getShellInterface().getTimeMilliseconds();
    const Int64 deltaTimeMs   = currentTimeMs - prevFrameTimeMs;
    prevFrameTimeMs = currentTimeMs;

    if (resettingAngles)
    {
        const Float32 resetSpeed = 2.0f;
        const Float32 deltaTimeSeconds = deltaTimeMs * 0.001;

        rotationDegrees.x = lerpAngles(rotationDegrees.x, 0.0f, resetSpeed * deltaTimeSeconds);
        rotationDegrees.y = lerpAngles(rotationDegrees.y, 0.0f, resetSpeed * deltaTimeSeconds);
        rotationDegrees.z = lerpAngles(rotationDegrees.z, 0.0f, resetSpeed * deltaTimeSeconds);
        updateScrGeometry = true;

        if (angleNearZero(rotationDegrees.x) &&
            angleNearZero(rotationDegrees.y) &&
            angleNearZero(rotationDegrees.z))
        {
            rotationDegrees.setZero();
            resettingAngles = false;
        }
    }

    if (!isVisible())
    {
        return;
    }

    const Color32 vpOutlineColor = getColors().view3dOutline;
    const Color32 resetBtnColor  = getColors().text.normal;
    const Color32 xAxisColor     = packColor(225, 0, 0); // X=red
    const Color32 yAxisColor     = packColor(0, 225, 0); // Y=green
    const Color32 zAxisColor     = packColor(0, 0, 225); // Z=blue

    Widget::onDraw(geoBatch);
    geoBatch.drawRectOutline(projParams.viewport, vpOutlineColor);

    if (interactiveControls)
    {
        Rectangle textBox = resetAnglesBtnRect;
        textBox.moveBy(Widget::uiScaled(2), Widget::uiScaled(-4)); // Better center the "R"
        geoBatch.drawTextConstrained("R", 1, textBox, textBox, textScaling, resetBtnColor, TextAlign::Left);
    }

    if (showXyzLabels)
    {
        const Float32 chrW = GeometryBatch::getCharWidth()  * textScaling;
        const Float32 chrH = GeometryBatch::getCharHeight() * textScaling;

        Rectangle textBox;
        textBox.xMins = projParams.viewport.xMaxs - chrW - Widget::uiScaled(4);
        textBox.yMins = projParams.viewport.yMaxs - (chrH * 3) - Widget::uiScaled(4);
        textBox.xMaxs = textBox.xMins + chrW + Widget::uiScaled(2);
        textBox.yMaxs = textBox.yMins + chrH * 3;

        geoBatch.drawTextConstrained("x", 1, textBox, textBox, textScaling, xAxisColor, TextAlign::Right);
        textBox = textBox.shrunk(0, chrH);
        geoBatch.drawTextConstrained("y", 1, textBox, textBox, textScaling, yAxisColor, TextAlign::Right);
        textBox = textBox.shrunk(0, chrH);
        geoBatch.drawTextConstrained("z", 1, textBox, textBox, textScaling, zAxisColor, TextAlign::Right);
    }

    if (updateScrGeometry)
    {
        const Mat4x4 matRx = Mat4x4::rotationX(degToRad(rotationDegrees.x));
        const Mat4x4 matRy = Mat4x4::rotationY(degToRad(rotationDegrees.y));
        const Mat4x4 matRz = Mat4x4::rotationZ(degToRad(rotationDegrees.z));
        const Mat4x4 modelToWorldMatrix = Mat4x4::multiply(Mat4x4::multiply(matRz, matRx), matRy);

        clearScreenVertexCaches();

        switch (object)
        {
        case ObjectType::Sphere :
            addScreenProjectedArrow(modelToWorldMatrix,  0.28f, xAxisColor, ArrowDirX);
            addScreenProjectedArrow(modelToWorldMatrix,  0.28f, yAxisColor, ArrowDirY);
            addScreenProjectedArrow(modelToWorldMatrix,  0.28f, zAxisColor, ArrowDirZ);
            addScreenProjectedSphere(modelToWorldMatrix, 0.20f);
            break;

        case ObjectType::Arrow :
            addScreenProjectedArrow(modelToWorldMatrix, 0.35f, getColors().view3dArrowObj, ArrowDirZ);
            break;

        case ObjectType::Box :
            addScreenProjectedBox(modelToWorldMatrix, 0.4f, 0.4f, 0.4f, getColors().view3dBoxObj);
            break;

        default :
            break;
        } // switch (object)

        updateScrGeometry = false;
    }

    submitScreenVertexCaches(geoBatch);
}

void View3DWidget::onMove(int displacementX, int displacementY)
{
    Widget::onMove(displacementX, displacementY);

    resetAnglesBtnRect.moveBy(displacementX, displacementY);
    refreshProjectionViewport();
}

bool View3DWidget::onMouseButton(MouseButton button, int clicks)
{
    const bool eventHandled = Widget::onMouseButton(button, clicks);

    if (interactiveControls && isMouseIntersecting())
    {
        if (leftClick(button, clicks))
        {
            // Clicking the "R" button resets the angles.
            if (resetAnglesBtnRect.containsPoint(lastMousePos))
            {
                resettingAngles   = true;
                updateScrGeometry = true;
            }
            else
            {
                leftMouseButtonDown = true;
            }
        }
        else if (clicks <= 0)
        {
            leftMouseButtonDown = false;
            mouseDelta.setZero();
        }
    }

    return eventHandled | leftMouseButtonDown;
}

bool View3DWidget::onMouseMotion(int mx, int my)
{
    mouseDelta.x = mx - lastMousePos.x;
    mouseDelta.y = my - lastMousePos.y;
    mouseDelta.x = clamp(mouseDelta.x, -maxMouseDelta, maxMouseDelta);
    mouseDelta.y = clamp(mouseDelta.y, -maxMouseDelta, maxMouseDelta);

    // Prevent it from being dragged out the top of the screen:
    int clampedY = my;
    if (isMouseDragEnabled())
    {
        const int displacementY = my - lastMousePos.y;
        if ((rect.yMins + displacementY) < 0)
        {
            clampedY = my - (rect.yMins + displacementY);
        }
    }

    bool eventHandled = Widget::onMouseMotion(mx, clampedY);

    if (interactiveControls && leftMouseButtonDown && isMouseIntersecting() && projParams.viewport.containsPoint(mx, my))
    {
        const Float32 dirY = invertMouseY ? -1.0f : 1.0f;
        rotationDegrees.x -= mouseDelta.y * mouseSensitivity * dirY;
        rotationDegrees.y += mouseDelta.x * mouseSensitivity;
        rotationDegrees.x  = normalizeAngle360(rotationDegrees.x);
        rotationDegrees.y  = normalizeAngle360(rotationDegrees.y);

        mouseDelta.setZero();
        resettingAngles   = false;
        updateScrGeometry = true;
        eventHandled      = true;
    }

    return eventHandled;
}

bool View3DWidget::onMouseScroll(int yScroll)
{
    if (isVisible() && isMouseIntersecting() && interactiveControls && leftMouseButtonDown)
    {
        // Zoom
        resettingAngles   = false;
        updateScrGeometry = true;
        rotationDegrees.z = normalizeAngle360(rotationDegrees.z + (yScroll * mouseSensitivity));
        return true;
    }
    return false;
}

void View3DWidget::setMouseIntersecting(bool intersect)
{
    Widget::setMouseIntersecting(intersect);

    if (intersect)
    {
        titleBar.setHighlightedColors();
    }
    else // If we lost mouse focus just cancel the last button down event.
    {
        leftMouseButtonDown = false;
    }
}

void View3DWidget::clearScreenVertexCaches() const
{
    scrProjectedVerts.clear();
    scrProjectedIndexes.clear();
}

void View3DWidget::submitScreenVertexCaches(GeometryBatch & geoBatch) const
{
    geoBatch.drawClipped2DTriangles(scrProjectedVerts.getData<VertexPTC>(), scrProjectedVerts.getSize(),
                                    scrProjectedIndexes.getData<UInt16>(), scrProjectedIndexes.getSize(),
                                    projParams.viewport, projParams.viewport);
}

void View3DWidget::addScreenProjectedSphere(const Mat4x4 & modelToWorldMatrix, Float32 scaleXYZ) const
{
    const RenderInterface & renderer = getRenderInterface();

    int vp[4];
    renderer.getViewport(&vp[0], &vp[1], &vp[2], &vp[3]);
    const Rectangle scrViewport{ vp[0], vp[1], vp[0] + vp[2], vp[1] + vp[3] };

    const bool highlighted   = isMouseIntersecting();
    const Color32 brightness = highlighted ? packColor(255, 255, 255) : packColor(200, 200, 200);
    const Color32 shadeColor = packColor(0, 0, 0, 255);
    const SphereVert * pVert = detail::g_sphereVerts;
    const int vertCount      = lengthOfArray(detail::g_sphereVerts);
    UInt16 nextVertexIndex   = scrProjectedVerts.getSize();

    // Should have been preallocated.
    NTB_ASSERT(scrProjectedVerts.getCapacity()   >= vertCount);
    NTB_ASSERT(scrProjectedIndexes.getCapacity() >= vertCount);

    for (int v = 0; v < vertCount; ++v, ++pVert)
    {
        const Vec3 wp = Mat4x4::transformPointAffine(pVert->position, modelToWorldMatrix);
        const Color32 vertColor = blendColors(shadeColor, pVert->color & brightness,
                                              std::fabs(clamp(wp.z, -1.0f, 1.0f)));

        VertexPTC scrVert = { wp.x * scaleXYZ, wp.y * scaleXYZ, wp.z * scaleXYZ, 0.0f, 0.0f, vertColor };
        screenProjectionXY(scrVert, scrVert, projParams.viewProjMatrix, scrViewport);

        scrProjectedVerts.pushBack<VertexPTC>(scrVert);
        scrProjectedIndexes.pushBack<UInt16>(nextVertexIndex++);
    }
}

void View3DWidget::addScreenProjectedArrow(const Mat4x4 & modelToWorldMatrix, Float32 scaleXYZ, Color32 color, ArrowDir dir) const
{
    const RenderInterface & renderer = getRenderInterface();

    int vp[4];
    renderer.getViewport(&vp[0], &vp[1], &vp[2], &vp[3]);
    const Rectangle scrViewport{ vp[0], vp[1], vp[0] + vp[2], vp[1] + vp[3] };

    const bool highlighted   = isMouseIntersecting();
    const Color32 brightness = highlighted ? packColor(255, 255, 255) : packColor(200, 200, 200);
    const Color32 shadeColor = packColor(0, 0, 0, 255);
    const ArrowVert * pVert  = detail::g_arrowVerts;
    const int vertCount      = lengthOfArray(detail::g_arrowVerts);
    UInt16 nextVertexIndex   = scrProjectedVerts.getSize();

    // Should have been preallocated.
    NTB_ASSERT(scrProjectedVerts.getCapacity()   >= vertCount);
    NTB_ASSERT(scrProjectedIndexes.getCapacity() >= vertCount);

    for (int v = 0; v < vertCount; ++v, ++pVert)
    {
        ArrowVert av = *pVert;
        switch (dir)
        {
        case ArrowDirX :
            std::swap(av.position.x, av.position.z);
            std::swap(av.normal.x,   av.normal.z);
            break;

        case ArrowDirY :
            std::swap(av.position.y, av.position.z);
            std::swap(av.normal.y,   av.normal.z);
            break;

        default :
            // ArrowDirZ is the default direction.
            break;
        } // switch (dir)

        const Vec3 wp = Mat4x4::transformPointAffine(av.position, modelToWorldMatrix);
        const Vec3 wn = Mat4x4::transformPointAffine(av.normal,   modelToWorldMatrix);

        const Color32 vertColor = blendColors(shadeColor, color & brightness,
                                              std::fabs(clamp(wn.z, -1.0f, 1.0f)));

        VertexPTC scrVert = { wp.x * scaleXYZ, wp.y * scaleXYZ, wp.z * scaleXYZ, 0.0f, 0.0f, vertColor };
        screenProjectionXY(scrVert, scrVert, projParams.viewProjMatrix, scrViewport);

        scrProjectedVerts.pushBack<VertexPTC>(scrVert);
        scrProjectedIndexes.pushBack<UInt16>(nextVertexIndex++);
    }
}

void View3DWidget::addScreenProjectedBox(const Mat4x4 & modelToWorldMatrix, Float32 w, Float32 h, Float32 d, Color32 color) const
{
    const RenderInterface & renderer = getRenderInterface();

    int vp[4];
    renderer.getViewport(&vp[0], &vp[1], &vp[2], &vp[3]);
    const Rectangle scrViewport{ vp[0], vp[1], vp[0] + vp[2], vp[1] + vp[3] };

    BoxVert tempBoxVerts[24];
    UInt16  tempBoxIndexes[36];

    const bool highlighted   = isMouseIntersecting();
    const Color32 brightness = highlighted ? packColor(255, 255, 255) : packColor(200, 200, 200);
    const Color32 shadeColor = packColor(0, 0, 0, 255);
    const BoxVert * pVert    = tempBoxVerts;
    const UInt16  * pIndex   = tempBoxIndexes;
    const int vertCount      = lengthOfArray(tempBoxVerts);
    const int indexCount     = lengthOfArray(tempBoxIndexes);

    // Each face could be colored independently, but we aren't using this at the moment.
    const Color32 tempFaceColors[6] = { color, color, color, color, color, color };
    makeTexturedBoxGeometry(tempBoxVerts, tempBoxIndexes, tempFaceColors, w, h, d);

    // Should have been preallocated.
    NTB_ASSERT(scrProjectedVerts.getCapacity()   >= vertCount);
    NTB_ASSERT(scrProjectedIndexes.getCapacity() >= indexCount);

    for (int v = 0; v < vertCount; ++v, ++pVert)
    {
        const Vec3 wp = Mat4x4::transformPointAffine(pVert->position, modelToWorldMatrix);
        const Vec3 wn = Mat4x4::transformPointAffine(pVert->normal,   modelToWorldMatrix);

        const Color32 vertColor = blendColors(shadeColor, pVert->color & brightness,
                                              std::fabs(clamp(wn.z, -1.0f, 1.0f)));

        VertexPTC scrVert = { wp.x, wp.y, wp.z, pVert->u, pVert->v, vertColor };
        screenProjectionXY(scrVert, scrVert, projParams.viewProjMatrix, scrViewport);

        scrProjectedVerts.pushBack<VertexPTC>(scrVert);
    }

    // Unlike the others, the box geometry is actually indexed!
    for (int i = 0; i < indexCount; ++i, ++pIndex)
    {
        scrProjectedIndexes.pushBack<UInt16>(*pIndex);
    }
}

void View3DWidget::refreshProjectionViewport()
{
    const int vpOffset = Widget::uiScaled(4);
    const Float32 oldAspectRatio = projParams.viewport.getAspect();

    // Update the viewport rect:
    projParams.viewport = rect;
    projParams.viewport.xMins += vpOffset;
    projParams.viewport.xMaxs -= vpOffset;
    projParams.viewport.yMins += titleBar.getRect().getHeight() + vpOffset;
    projParams.viewport.yMaxs -= vpOffset;

    // Might also have to recompute the projection/view,
    // since the aspect-ratio might have changed.
    if (projParams.autoAdjustAspect && oldAspectRatio != projParams.viewport.getAspect())
    {
        projParams.aspectRatio = projParams.viewport.getAspect();

        const Mat4x4 projMatrix = Mat4x4::perspective(projParams.fovYRadians,
                                                      projParams.aspectRatio,
                                                      projParams.zNear,
                                                      projParams.zFar);

        const Mat4x4 viewMatrix = Mat4x4::lookAt(Vec3{ 0.0f, 0.0f, +1.0f },
                                                 Vec3{ 0.0f, 0.0f, -1.0f },
                                                 Vec3{ 0.0f, 1.0f,  0.0f });

        projParams.viewProjMatrix = Mat4x4::multiply(viewMatrix, projMatrix);
    }
}

// ========================================================
// class VarDisplayWidget:
// ========================================================

VarDisplayWidget::VarDisplayWidget()
    : parentWindow(nullptr)
    , customTextColor(0)
    , initialHeight(0)
{
    incrButton.setZero();
    decrButton.setZero();
    editPopupButton.setZero();
    dataDisplayRect.setZero();
}

VarDisplayWidget::~VarDisplayWidget()
{
    // In case we are being deleted before the parent WindowWidget.
    if (parentWindow != nullptr && editField.isLinked())
    {
        parentWindow->removeEditField(&editField);
    }
}

void VarDisplayWidget::init(GUI * myGUI, VarDisplayWidget * myParent, const Rectangle & myRect,
                            bool visible, WindowWidget * myWindow, const char * name)
{
    auto parentWidget = (myParent != nullptr) ? static_cast<Widget *>(myParent) : static_cast<Widget *>(myWindow);
    Widget::init(myGUI, parentWidget, myRect, visible);

    parentWindow  = myWindow;
    initialHeight = myRect.getHeight();

    if (name != nullptr && *name != '\0')
    {
        varName = name;
    }

    if (myParent != nullptr)
    {
        // Parent VarDisplayWidget gets an expand collapse button [+]/[-]
        myParent->addExpandCollapseButton();
        myParent->addChild(this);
    }
    else if (myWindow != nullptr)
    {
        // Direct child of the window/panel.
        myWindow->addChild(this);
    }

    dataDisplayRect = makeDataDisplayAndButtonRects(false);

    if (parentWindow != nullptr)
    {
        parentWindow->addEditField(&editField);
    }
}

void VarDisplayWidget::onDraw(GeometryBatch & geoBatch) const
{
    if (isVisible())
    {
        drawSelf(geoBatch);             // Border rectangle
        drawVarName(geoBatch);          // Var name drawing (left side)
        drawVarValue(geoBatch);         // Displayed value (right side)
        drawValueEditButtons(geoBatch); // [+],[-] buttons (if enabled)
    }

    // Still go over the children even if this is not visible.
    // The var might be scrolled out, but its children might
    // still be visible.

    if (!isHierarchyCollapsed())
    {
        drawChildren(geoBatch);
    }
}

void VarDisplayWidget::drawVarName(GeometryBatch & geoBatch) const
{
    Rectangle textBox{ rect };

    // Top/center offset
    const Float32 chrMid = GeometryBatch::getCharHeight() * textScaling * 0.5f;
    const Float32 boxMid = textBox.getHeight() * 0.5f;
    textBox.moveBy(Widget::uiScaled(2), boxMid - chrMid); // Also move slightly on the X to avoid touching the border.

    geoBatch.drawTextConstrained(varName.c_str(), varName.getLength(), textBox, textBox,
                                 textScaling, getColors().text.informational, TextAlign::Left);
}

void VarDisplayWidget::drawVarValue(GeometryBatch & /*geoBatch*/) const
{
    // Unimplemented in the base class.
}

void VarDisplayWidget::drawValueEditButtons(GeometryBatch & geoBatch) const
{
    //TODO
    (void)geoBatch;
}

void VarDisplayWidget::onMove(int displacementX, int displacementY)
{
    Widget::onMove(displacementX, displacementY);

    incrButton.moveBy(displacementX, displacementY);
    decrButton.moveBy(displacementX, displacementY);
    editPopupButton.moveBy(displacementX, displacementY);
    dataDisplayRect.moveBy(displacementX, displacementY);
}

void VarDisplayWidget::onResize(int displacementX, int displacementY, Corner corner)
{
    switch (corner)
    {
    case TopLeft :
        rect.xMins += displacementX;
        rect.yMins += displacementY;
        rect.yMaxs = rect.yMins + initialHeight;
        dataDisplayRect.xMins += displacementX;
        break;

    case BottomLeft :
        rect.xMins += displacementX;
        dataDisplayRect.xMins += displacementX;
        break;

    case TopRight :
        rect.xMaxs += displacementX;
        rect.yMins += displacementY;
        rect.yMaxs = rect.yMins + initialHeight;
        break;

    case BottomRight :
        rect.xMaxs += displacementX;
        break;

    default :
        errorF("Bad corner enum in VarDisplayWidget!");
        break;
    } // switch (corner)

    // Propagate to the var hierarchy:
    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        getChild(c)->onResize(displacementX, displacementY, corner);
    }
}

void VarDisplayWidget::onAdjustLayout()
{
    dataDisplayRect = makeDataDisplayAndButtonRects(testFlag(Flag_ValueEditButtonsEnabled));

    // Only if edit buttons are allowed for this variable type.
    if (testFlag(Flag_WithValueEditButtons))
    {
        // See if we need to disable the value edit buttons and reset the data display rect:
        if (dataDisplayRect.getWidth() <= getMinDataDisplayRectWidth())
        {
            setFlag(Flag_ValueEditButtonsEnabled, false);
            dataDisplayRect = makeDataDisplayAndButtonRects(false);
        }
        // Or if the width is now big enough, restore the buttons:
        else if (!testFlag(Flag_ValueEditButtonsEnabled))
        {
            setFlag(Flag_ValueEditButtonsEnabled, true);
            const Rectangle newRect = makeDataDisplayAndButtonRects(true);

            // If re-enabling the buttons violates the min width. Don't do it.
            if (newRect.getWidth() <= getMinDataDisplayRectWidth())
            {
                setFlag(Flag_ValueEditButtonsEnabled, false);
            }
            else
            {
                dataDisplayRect = newRect;
            }
        }
    }

    // This button is only present on variable hierarchies.
    if (hasExpandCollapseButton())
    {
        expandCollapseButton.setRect(getExpandCollapseButtonRect());
    }
}

void VarDisplayWidget::onDisableEditing()
{
    // Sine we have EditFields, do nothing.
    // Only WindowWidgets should handle this event.
}

void VarDisplayWidget::setVisible(bool visible)
{
    Widget::setVisible(visible);
    expandCollapseButton.setVisible(visible);
}

bool VarDisplayWidget::onMouseButton(MouseButton button, int clicks)
{
    //TODO
    return Widget::onMouseButton(button, clicks);
}

bool VarDisplayWidget::onMouseMotion(int mx, int my)
{
    //TODO
    return Widget::onMouseMotion(mx, my);
}

bool VarDisplayWidget::onMouseScroll(int yScroll)
{
    //TODO
    return Widget::onMouseScroll(yScroll);
}

bool VarDisplayWidget::onKeyPressed(KeyCode key, KeyModFlags modifiers)
{
    //TODO
    return Widget::onKeyPressed(key, modifiers);
}

bool VarDisplayWidget::onButtonDown(ButtonWidget & button)
{
    if (hasExpandCollapseButton() && (&expandCollapseButton == &button))
    {
        setExpandCollapseState(expandCollapseButton.getState());
        return true;
    }
    return false;
}

void VarDisplayWidget::addExpandCollapseButton()
{
    if (hasExpandCollapseButton())
    {
        return; // Already has it.
    }

    const Rectangle btnRect = getExpandCollapseButtonRect();
    expandCollapseButton.init(getGUI(), this, btnRect, isVisible(), ButtonWidget::Icon::Minus, this);
    expandCollapseButton.setState(true); // Hierarchy initially open.

    // Note: Button is a child of the parent widget (WindowWidget), so that we
    // keep this widget's children list reserved for nested VarDisplayWidgets.
    if (parentWindow != nullptr)
    {
        parentWindow->addChild(&expandCollapseButton);
    }
}

void VarDisplayWidget::setHierarchyVisibility(VarDisplayWidget * child, bool visible) const
{
    NTB_ASSERT(child != nullptr);

    child->setVisible(visible);
    child->setMinimized(!visible);

    const int childCount = child->getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        if (!child->isHierarchyCollapsed())
        {
            setHierarchyVisibility(static_cast<VarDisplayWidget *>(child->getChild(c)), visible);
        }
    }
}

void VarDisplayWidget::setExpandCollapseState(bool expanded)
{
    // Mark each child as hidden or visible, recursively:
    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        // Children of VarDisplayWidget should only be other VarDisplayWidgets.
        // The button is a child of the parent WindowWidget/Panel.
        setHierarchyVisibility(static_cast<VarDisplayWidget *>(getChild(c)), expanded);
    }

    // Change the icon appropriately:
    expandCollapseButton.setIcon(expanded ? ButtonWidget::Icon::Minus : ButtonWidget::Icon::Plus);
    expandCollapseButton.setState(expanded);

    // Collapse the hidden variables in the window to fill the gaps.
    if (parentWindow != nullptr)
    {
        parentWindow->onAdjustLayout();
    }
}

int VarDisplayWidget::getMinDataDisplayRectWidth() const
{
    // Reserve space for about 3 characters plus some arbitrary extra...
    return (GeometryBatch::getCharWidth() * 3 * textScaling) + Widget::uiScaled(4);
}

Rectangle VarDisplayWidget::makeDataDisplayAndButtonRects(bool editButtons)
{
    const int buttonWidth = Widget::uiScaled(8);
    int xMins = rect.xMins + (rect.getWidth() / 2) + Widget::uiScaled(10);
    int yMins = rect.yMins;
    int xMaxs = rect.xMaxs;
    int yMaxs = rect.yMaxs;

    // The offsets are really just one pixel, I didn't forget to use uiScaled()!
    editPopupButton.xMins = xMaxs - buttonWidth;
    editPopupButton.yMins = yMins + 1;
    editPopupButton.xMaxs = xMaxs - 1;
    editPopupButton.yMaxs = yMaxs - 1;

    decrButton = editPopupButton;
    decrButton.xMins -= buttonWidth + 1;
    decrButton.xMaxs -= buttonWidth + 1;

    incrButton = decrButton;
    incrButton.xMins -= buttonWidth + 1;
    incrButton.xMaxs -= buttonWidth + 1;

    if (editButtons)
    {
        const int buttonsWidthTotal = incrButton.getWidth() +
                                      decrButton.getWidth() +
                                      editPopupButton.getWidth();
        xMaxs -= buttonsWidthTotal;
        xMaxs -= Widget::uiScaled(4);
    }

    return { xMins, yMins, xMaxs, yMaxs };
}

Rectangle VarDisplayWidget::getExpandCollapseButtonRect() const
{
    const int buttonSize = getExpandCollapseButtonSize();
    const int gapX = Widget::uiScaleBy(buttonSize, 0.2);
    const int gapY = Widget::uiScaleBy(buttonSize, 0.1);

    const int xMins = rect.xMins - buttonSize - gapX;
    const int yMins = rect.yMins + gapY;
    const int xMaxs = xMins + buttonSize;
    const int yMaxs = yMins + buttonSize;

    return { xMins, yMins, xMaxs, yMaxs };
}

#if NEO_TWEAK_BAR_DEBUG
SmallStr VarDisplayWidget::getTypeString() const
{
    SmallStr varStr = "VarDisplayWidget ";
    varStr += "\'";
    varStr += varName;
    varStr += "\'";
    return varStr;
}
#endif // NEO_TWEAK_BAR_DEBUG

// ========================================================
// class WindowWidget:
// ========================================================

WindowWidget::WindowWidget()
    : resizingCorner(CornerNone)
    , popupWidget(nullptr)
    , titleBarButtonSize(0)
    , titleBarHeight(0)
    , scrollBarButtonSize(0)
    , scrollBarWidth(0)
{
    usableRect.setZero();
}

WindowWidget::~WindowWidget()
{
    //TODO might need to flag dynamic elements for deletion! (like the popupWidget)

    // Edit fields are never dynamically allocated. So just unlink.
    editFieldsList.unlinkAll();
}

void WindowWidget::init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible, const char * title,
                        int titleBarH, int titleBarBtnSize, int scrollBarW, int scrollBarBtnSize)
{
    Widget::init(myGUI, myParent, myRect, visible);

    titleBarButtonSize  = static_cast<Int16>(titleBarBtnSize);
    titleBarHeight      = static_cast<Int16>(titleBarH);
    scrollBarButtonSize = static_cast<Int16>(scrollBarBtnSize);
    scrollBarWidth      = static_cast<Int16>(scrollBarW);

    refreshBarRects(title, nullptr);
    addChild(&scrollBar);
    addChild(&titleBar);
    addChild(&infoBar);
    refreshUsableRect();
}

void WindowWidget::onDraw(GeometryBatch & geoBatch) const
{
    if (!isVisible())
    {
        return;
    }

    // First draw the window itself:
    drawSelf(geoBatch);

    // If we have an open popup, it must draw last, at the top.
    // We have to override Widget::onDraw to draw the children
    // first and them the popup (which is also a window child).
    if (popupWidget != nullptr)
    {
        const int childCount = getChildCount();
        for (int c = 0; c < childCount; ++c)
        {
            const Widget * child = getChild(c);
            if (child != popupWidget)
            {
                child->onDraw(geoBatch);
            }
        }
    }
    else // Same logic of Widget::onDraw
    {
        drawChildren(geoBatch);
    }

    // Has to be on top of everything...
    drawResizeHandles(geoBatch);

    // The popup still has to be above the resize handles.
    if (popupWidget != nullptr)
    {
        popupWidget->onDraw(geoBatch);
    }
}

void WindowWidget::onMove(int displacementX, int displacementY)
{
    Widget::onMove(displacementX, displacementY);
    refreshUsableRect(); // Keep the effective usable area up-to-date.
}

void WindowWidget::onAdjustLayout()
{
    // Keep the sub-rects up-to-date.
    refreshBarRects(nullptr, nullptr);
    refreshUsableRect();
}

void WindowWidget::onDisableEditing()
{
    EditField * pEdit = editFieldsList.getFirst();
    if (pEdit != nullptr)
    {
        pEdit->setActive(false);
    }
}

void WindowWidget::setMouseIntersecting(bool intersect)
{
    Widget::setMouseIntersecting(intersect);

    // We want to highlight these when the parent window gains focus.
    if (intersect)
    {
        scrollBar.setHighlightedColors();
        titleBar.setHighlightedColors();
        infoBar.setHighlightedColors();
    }
}

bool WindowWidget::onMouseButton(MouseButton button, int clicks)
{
    if (!isVisible())
    {
        return false;
    }

    resizingCorner = CornerNone;

    // Check for intersection with the resize handles in the corners.
    // We can resize the window if there's a left click over one.
    if (isMouseIntersecting() && leftClick(button, clicks))
    {
        const int xMins = rect.xMins;
        const int xMaxs = rect.xMaxs;
        const int yMins = rect.yMins;
        const int yMaxs = rect.yMaxs;
        const int handleSize = Widget::uiScaleBy(titleBarButtonSize, 0.8);

        Rectangle resizeHandles[CornerCount];
        resizeHandles[ TopLeft     ].set(xMins, yMins, xMins + handleSize, yMins + handleSize); // top-left
        resizeHandles[ BottomLeft  ].set(xMins, yMaxs - handleSize, xMins + handleSize, yMaxs); // bottom-left
        resizeHandles[ TopRight    ].set(xMaxs - handleSize, yMins, xMaxs, yMins + handleSize); // top-right
        resizeHandles[ BottomRight ].set(xMaxs - handleSize, yMaxs - handleSize, xMaxs, yMaxs); // bottom-right

        for (int c = 0; c < CornerCount; ++c)
        {
            if (resizeHandles[c].containsPoint(lastMousePos))
            {
                resizingCorner = Corner(c); // Save for onMouseMotion()
                onDisableEditing();         // All edit fields lose focus
                setMouseDragEnabled(false); // Cancel any mouse drag
                return true;
            }
        }
    }

    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        Widget * child = getChild(c);
        if (child->onMouseButton(button, clicks))
        {
            // Bubble this message up to the parent (this)
            // or break the chain if one of the Widgets is
            // the active/focused edit field. Simply calling
            // onDisableEditing() wouldn't work.
            child->onDisableEditing();
            return true;
        }
    }

    // If the cursor is intersecting this element or any
    // of its children, we consume the mouse click, even
    // if it has no input effect in the UI.
    if (isMouseIntersecting())
    {
        onDisableEditing();
        return true;
    }
    return false;
}

bool WindowWidget::onMouseMotion(int mx, int my)
{
    if (!isVisible())
    {
        return false;
    }

    // Window can go out from the sides and bottom, but not
    // out the top of the screen. We preempt movement at the
    // window level if that's the case.
    int clampedY = my;
    if (isMouseDragEnabled())
    {
        const int displacementY = my - lastMousePos.y;
        if ((rect.yMins + displacementY) < 0)
        {
            clampedY = my - (rect.yMins + displacementY);
        }
    }

    // Handle resizing each corner:
    switch (resizingCorner)
    {
    case TopLeft :
        resizeWithMin(resizingCorner, rect.xMins, rect.yMins,
                      mx - lastMousePos.x, clampedY - lastMousePos.y);
        break;

    case BottomLeft :
        resizeWithMin(resizingCorner, rect.xMins, rect.yMaxs,
                      mx - lastMousePos.x, clampedY - lastMousePos.y);
        break;

    case TopRight :
        resizeWithMin(resizingCorner, rect.xMaxs, rect.yMins,
                      mx - lastMousePos.x, clampedY - lastMousePos.y);
        break;

    case BottomRight :
        resizeWithMin(resizingCorner, rect.xMaxs, rect.yMaxs,
                      mx - lastMousePos.x, clampedY - lastMousePos.y);
        break;

    default :
        // Not an error, just means we are not resizing.
        break;
    } // switch (resizingCorner)

    return Widget::onMouseMotion(mx, clampedY);
}

bool WindowWidget::onMouseScroll(int yScroll)
{
    // Allow child element to respond the event first (like a ColorPicker window)
    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        if (getChild(c)->isMouseIntersecting() &&
            getChild(c)->onMouseScroll(yScroll))
        {
            return true;
        }
    }

    // Only scroll if the mouse is hovering this window!
    if (isMouseIntersecting())
    {
        return scrollBar.onMouseScroll(yScroll);
    }

    return false;
}

bool WindowWidget::onKeyPressed(KeyCode key, KeyModFlags modifiers)
{
    if (!isVisible())
    {
        return false;
    }

    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        if (getChild(c)->onKeyPressed(key, modifiers))
        {
            return true;
        }
    }
    return false;
}

void WindowWidget::drawResizeHandles(GeometryBatch & geoBatch) const
{
    // This messy block of code draws the wedges in each
    // corner of the window to indicate it is resizeable.
    //
    // Each wedge is a main colored line and
    // a shade line offset by one pixel.

    const Color32 wedgeColor      = getColors().resizeHandle;
    const Color32 shadeColor      = packColor(0, 0, 0);
    const int cornerWedgeLineSize = Widget::uiScaleBy(titleBarButtonSize, 0.8);
    const int cornerWedgeLineOffs = Widget::uiScaled(4);

    const int xMins = rect.xMins;
    const int xMaxs = rect.xMaxs;
    const int yMins = rect.yMins;
    const int yMaxs = rect.yMaxs;
    int xFrom, yFrom, xTo, yTo;

    // Top-left corner, horizontal line width shade:
    xFrom = xMins + cornerWedgeLineOffs;
    yFrom = yMins + cornerWedgeLineOffs;
    xTo = xMins + cornerWedgeLineSize;
    yTo = yFrom;
    geoBatch.drawLine(xFrom, yFrom, xTo, yTo, wedgeColor);
    geoBatch.drawLine(xFrom, yFrom + 1, xTo, yTo + 1, shadeColor);

    // Top-left corner, vertical line with shade:
    xFrom = xMins + cornerWedgeLineOffs;
    yFrom = yMins + cornerWedgeLineOffs;
    xTo = xFrom;
    yTo = yMins + cornerWedgeLineSize;
    geoBatch.drawLine(xFrom, yFrom, xTo, yTo, wedgeColor);
    geoBatch.drawLine(xFrom + 1, yFrom + 1, xTo + 1, yTo, shadeColor);

    // Top-right corner, horizontal line width shade:
    xFrom = xMaxs - cornerWedgeLineSize;
    yFrom = yMins + cornerWedgeLineOffs;
    xTo = xMaxs - cornerWedgeLineOffs;
    yTo = yFrom;
    geoBatch.drawLine(xFrom, yFrom, xTo, yTo, wedgeColor);
    geoBatch.drawLine(xFrom, yFrom + 1, xTo, yTo + 1, shadeColor);

    // Top-right corner, vertical line with shade:
    xFrom = xMaxs - cornerWedgeLineOffs;
    yFrom = yMins + cornerWedgeLineOffs;
    xTo = xFrom;
    yTo = yMins + cornerWedgeLineSize;
    geoBatch.drawLine(xFrom, yFrom, xTo, yTo, wedgeColor);
    geoBatch.drawLine(xFrom + 1, yFrom + 1, xTo + 1, yTo, shadeColor);

    // Bottom-left corner, horizontal line width shade:
    xFrom = xMins + cornerWedgeLineOffs;
    yFrom = yMaxs - cornerWedgeLineOffs;
    xTo = xFrom + cornerWedgeLineSize;
    yTo = yFrom;
    geoBatch.drawLine(xFrom, yFrom, xTo, yTo, wedgeColor);
    geoBatch.drawLine(xFrom, yFrom + 1, xTo, yTo + 1, shadeColor);

    // Bottom-left corner, vertical line with shade:
    xFrom = xMins + cornerWedgeLineOffs;
    yFrom = yMaxs - cornerWedgeLineOffs;
    xTo = xFrom;
    yTo = yFrom - cornerWedgeLineSize;
    geoBatch.drawLine(xFrom, yFrom, xTo, yTo, wedgeColor);
    geoBatch.drawLine(xFrom + 1, yFrom, xTo + 1, yTo, shadeColor);

    // Bottom-right corner, horizontal line width shade:
    xFrom = xMaxs - cornerWedgeLineOffs;
    yFrom = yMaxs - cornerWedgeLineOffs;
    xTo = xMaxs - cornerWedgeLineSize;
    yTo = yFrom;
    geoBatch.drawLine(xFrom, yFrom, xTo, yTo, wedgeColor);
    geoBatch.drawLine(xFrom + 1, yFrom + 1, xTo, yTo + 1, shadeColor);

    // Bottom-right corner, vertical line with shade:
    xFrom = xMaxs - cornerWedgeLineOffs;
    yFrom = yMaxs - cornerWedgeLineOffs;
    xTo = xFrom;
    yTo = yMaxs - cornerWedgeLineSize;
    geoBatch.drawLine(xFrom, yFrom, xTo, yTo, wedgeColor);
    geoBatch.drawLine(xFrom + 1, yFrom + 1, xTo + 1, yTo, shadeColor);
}

void WindowWidget::resizeWithMin(Corner corner, int & x, int & y, int offsetX, int offsetY)
{
    const int minWindowWidth  = getMinWindowWidthScaled();
    const int minWindowHeight = getMinWindowHeightScaled();
    const Rectangle old = rect;

    // x/y are refs to 'this->rect'.
    x += offsetX;
    y += offsetY;

    // Rollback if we are below the size minimum.
    if (rect.getWidth() < minWindowWidth)
    {
        rect.xMins = old.xMins;
        rect.xMaxs = old.xMaxs;
        offsetX = 0;
    }
    if (rect.getHeight() < minWindowHeight)
    {
        rect.yMins = old.yMins;
        rect.yMaxs = old.yMaxs;
        offsetY = 0;
    }

    // Never go out the top of the screen.
    if (rect.yMins < 0)
    {
        rect.yMins = old.yMins;
        offsetY = 0;
    }

    // Only notify the children and perform adjustments if there's a change in size.
    // We might have snapped them to zero if already at the min size.
    if (offsetX != 0 || offsetY != 0)
    {
        const int childCount = getChildCount();
        for (int c = 0; c < childCount; ++c)
        {
            getChild(c)->onResize(offsetX, offsetY, corner);
        }
        onAdjustLayout();
    }
}

void WindowWidget::refreshBarRects(const char * newTitle, const char * newInfoString)
{
    // Title bar stuff:
    const int  gapBetweenButtons = Widget::uiScaled(10);
    const int  btnOffsX          = titleBarButtonSize + Widget::uiScaled(2);
    const int  btnOffsY          = Widget::uiScaled(4);
    const bool minimizeBtn       = true;
    const bool maximizeBtn       = true;

    // Vertical scroll bar (right-hand side):
    const Rectangle scrollBarRect{ rect.xMaxs - scrollBarWidth,
                                   rect.yMins + titleBarHeight + Widget::uiScaled(1),
                                   rect.xMaxs,
                                   rect.yMaxs };
    // Title bar (top):
    const Rectangle titleBarRect{ rect.xMins,
                                  rect.yMins,
                                  rect.xMaxs,
                                  rect.yMins + titleBarHeight };
    // Info bar at the bottom:
    const Rectangle infoBarRect{ rect.xMins + scrollBarWidth,
                                 rect.yMaxs - titleBarHeight,
                                 rect.xMaxs - scrollBarWidth - Widget::uiScaled(1),
                                 rect.yMaxs };

    infoBar.init(gui, this, infoBarRect, isVisible(), newInfoString);
    scrollBar.init(gui, this, scrollBarRect, isVisible(), scrollBarButtonSize);
    titleBar.init(gui, this, titleBarRect, isVisible(), newTitle, minimizeBtn, maximizeBtn,
                  btnOffsX, btnOffsY, titleBarButtonSize, gapBetweenButtons);

    // This is necessary to make sure we keep the highlighted colors on the bars.
    // init() will always reset to the normal colors.
    setMouseIntersecting(isMouseIntersecting());
}

void WindowWidget::refreshUsableRect()
{
    // Need to refresh the usable sub-rect taking into account
    // the space occupied by the top/bottom/side bars.
    usableRect = rect;

    // Add a small hand-tuned offset to the top/bottom.
    const int offset = Widget::uiScaled(4);
    usableRect.xMaxs -= scrollBar.getRect().getWidth();
    usableRect.yMins += titleBar.getRect().getHeight() + offset;
    usableRect.yMaxs -= infoBar.getRect().getHeight()  + offset;
}

#if NEO_TWEAK_BAR_DEBUG
SmallStr WindowWidget::getTypeString() const
{
    SmallStr windowStr = "WindowWidget ";
    windowStr += "\'";
    windowStr += titleBar.getTitle();
    windowStr += "\'";
    return windowStr;
}
#endif // NEO_TWEAK_BAR_DEBUG

} // namespace ntb {}
