
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
        getRenderInterface()->destroyTexture(glyphTex);
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

    RenderInterface * renderer = getRenderInterface();
    renderer->beginDraw();
    currentZ = 0;
}

void GeometryBatch::endDraw()
{
    RenderInterface * renderer = getRenderInterface();

    // Continue anyway if exceeded (assuming the error handled doesn't throw).
    // The result might be a glitchy draw with overlapping elements.
    if (++currentZ >= renderer->getMaxZ())
    {
        errorF("Max frame Z index exceeded! Provide a custom RenderInterface::getMaxZ()!");
        currentZ = renderer->getMaxZ() - 1;
    }

    if (!verts2DBatch.isEmpty() && !tris2DBatch.isEmpty())
    {
        renderer->draw2DTriangles(
            verts2DBatch.getData<VertexPTC>(), verts2DBatch.getSize(),
            tris2DBatch.getData<UInt16>(), tris2DBatch.getSize(),
            nullptr, currentZ); // untextured
    }

    if (!drawClippedInfos.isEmpty() && !vertsClippedBatch.isEmpty() && !trisClippedBatch.isEmpty())
    {
        renderer->drawClipped2DTriangles(
            vertsClippedBatch.getData<VertexPTC>(), vertsClippedBatch.getSize(),
            trisClippedBatch.getData<UInt16>(), trisClippedBatch.getSize(),
            drawClippedInfos.getData<DrawClippedInfo>(), drawClippedInfos.getSize(), currentZ);
    }

    if (!textVertsBatch.isEmpty() && !textTrisBatch.isEmpty())
    {
        renderer->draw2DTriangles(
            textVertsBatch.getData<VertexPTC>(), textVertsBatch.getSize(),
            textTrisBatch.getData<UInt16>(), textTrisBatch.getSize(),
            glyphTex, currentZ); // textured
    }

    if (!linesBatch.isEmpty())
    {
        renderer->draw2DLines(linesBatch.getData<VertexPC>(), linesBatch.getSize(), currentZ);
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

    renderer->endDraw();
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

    glyphTex = getRenderInterface()->createTexture(
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

static bool leftClick(const MouseButton button, const int clicks)
{
    return clicks > 0 && button == MouseButton::Left;
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
            if (widget->testFlag(FlagNeedDeleting))
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
    setFlag(FlagVisible, visible);
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
    setFlag(FlagMouseDragEnabled, enabled);

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
        packColor(255, 255, 255, 255),
        }
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
        packColor(255, 255, 255, 255),
        }
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
    if (myColors.shadow.dark != 0 && myColors.shadow.offset != 0 && !testFlag(FlagNoRectShadow))
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

void ButtonWidget::init(GUI * myGUI, Widget * myParent, const Rectangle & myRect,
                        bool visible, Icon myIcon, EventListener * myListener)
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

    if (isCheckBoxButton())
    {
        //TODO special case
    }

    // Draw the box background and outline, if any:
    Widget::onDraw(geoBatch);

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
        " "  // CheckMark (unused)
    };
    static_assert(lengthOfArray(buttonIcons) == static_cast<int>(Icon::Count),
                  "Keep size in sync with Icon enum!");

    Rectangle charBox = rect;
    charBox.moveBy(0, Widget::uiScaleBy(4, textScaling)); // top offset
    geoBatch.drawTextConstrained(buttonIcons[static_cast<int>(icon)], 1, charBox, charBox,
                                 textScaling, getColors().text.normal, TextAlign::Center);
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

} // namespace ntb {}
