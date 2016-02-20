
#include "neo_tweak_bar.hpp"

namespace ntb
{

//TODO
//
// Macros with initial memory allocation hints for the batches???
//
// PROBABLY this the best place to add checks to avoid drawing
// rects and text chars that fall outside the window. Easier
// than cluttering the Widgets with checks!
//

//FIXME TEMP
#include "test_font.hpp"

// ========================================================
// class GeometryBatch:
// ========================================================

GeometryBatch::GeometryBatch()
    : glyphTex(NTB_NULL)
    , currZ(0)
    , maxZ(0)
    , baseVertex2d(0)
    , baseVertexText(0)
    , linesBatch(sizeof(VertexPC))
    , verts2dBatch(sizeof(VertexPTC))
    , tris2dBatch(sizeof(UInt16))
    , textVertsBatch(sizeof(VertexPTC))
    , textTrisBatch(sizeof(UInt16))
{
    createGlyphTexture();

    //TODO
    // SHOULD PROBABLY PREALLOCATE SOME MEMORY UP FRONT FOR THE BATCHES
    // LET THE USER DEFINE AMOUNT VIA MACROS? OR MAYBE PARAMETERS?
}

GeometryBatch::~GeometryBatch()
{
    if (glyphTex)
    {
        getRenderInterface()->destroyTexture(glyphTex);
        glyphTex = NTB_NULL;
    }
}

void GeometryBatch::beginDraw()
{
    // Ensure we are self consistent with begin/end calls!
    NTB_ASSERT(baseVertex2d == 0 && baseVertexText == 0);
    NTB_ASSERT(linesBatch.isEmpty());

    NTB_ASSERT(verts2dBatch.isEmpty());
    NTB_ASSERT(tris2dBatch.isEmpty());

    NTB_ASSERT(textVertsBatch.isEmpty());
    NTB_ASSERT(textTrisBatch.isEmpty());

    getRenderInterface()->beginDraw();

    // MaxZ is assumed to remain constant for at
    // least a complete begin/end draw sequence!
    currZ = 1;
    maxZ  = getRenderInterface()->getMaxZ();
}

void GeometryBatch::endDraw()
{
    if (currZ >= maxZ)
    {
        NTB_ERROR("Max Z value exceeded! Provide a custom RenderInterface::getMaxZ()!");
        // Continue anyway. The result might be a glitchy draw with overlapping elements.
    }

    if (!verts2dBatch.isEmpty() && !tris2dBatch.isEmpty())
    {
        getRenderInterface()->draw2DTriangles(
            verts2dBatch.getData<VertexPTC>(), verts2dBatch.getSize(),
            tris2dBatch.getData<UInt16>(), tris2dBatch.getSize(),
            NTB_NULL); // untextured
    }

    if (!textVertsBatch.isEmpty() && !textTrisBatch.isEmpty())
    {
        getRenderInterface()->draw2DTriangles(
            textVertsBatch.getData<VertexPTC>(), textVertsBatch.getSize(),
            textTrisBatch.getData<UInt16>(), textTrisBatch.getSize(),
            glyphTex); // textured
    }

    if (!linesBatch.isEmpty())
    {
        getRenderInterface()->draw2DLines(linesBatch.getData<VertexPC>(), linesBatch.getSize());
    }

    // Reset them:
    linesBatch.clear();
    verts2dBatch.clear();
    tris2dBatch.clear();
    textVertsBatch.clear();
    textTrisBatch.clear();

    // And the base vertex offsets:
    baseVertex2d   = 0;
    baseVertexText = 0;

    getRenderInterface()->endDraw();
}

void GeometryBatch::draw2DTriangles(const VertexPTC * verts, const int vertCount,
                                    const UInt16 * indexes, const int indexCount)
{
    NTB_ASSERT(verts   != NTB_NULL);
    NTB_ASSERT(indexes != NTB_NULL);
    NTB_ASSERT(vertCount  > 0);
    NTB_ASSERT(indexCount > 0);

    for (int i = 0; i < indexCount; ++i)
    {
        NTB_ASSERT(indexes[i] < unsigned(vertCount));
        tris2dBatch.pushBack<UInt16>(indexes[i] + baseVertex2d);
    }
    baseVertex2d += vertCount;

    const float z = static_cast<float>(currZ);
    for (int v = 0; v < vertCount; ++v)
    {
        VertexPTC vert = verts[v];
        vert.z = z;
        verts2dBatch.pushBack<VertexPTC>(vert);
    }
    ++currZ;
}

void GeometryBatch::drawLine(const int xFrom, const int yFrom,
                             const int xTo, const int yTo,
                             const Color32 colorFrom,
                             const Color32 colorTo)
{
    const float z = static_cast<float>(currZ);
    const VertexPC vertFrom =
    {
        static_cast<float>(xFrom),
        static_cast<float>(yFrom),
        z, colorFrom
    };
    const VertexPC vertTo =
    {
        static_cast<float>(xTo),
        static_cast<float>(yTo),
        z, colorTo
    };
    linesBatch.pushBack<VertexPC>(vertFrom);
    linesBatch.pushBack<VertexPC>(vertTo);
    ++currZ;
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

    verts[0].x = static_cast<float>(rect.xMins);
    verts[0].y = static_cast<float>(rect.yMins);
    verts[0].u = 0.0f;
    verts[0].v = 0.0f;
    verts[0].color = c0;

    verts[1].x = static_cast<float>(rect.xMins);
    verts[1].y = static_cast<float>(rect.yMaxs);
    verts[1].u = 0.0f;
    verts[1].v = 0.0f;
    verts[1].color = c1;

    verts[2].x = static_cast<float>(rect.xMaxs);
    verts[2].y = static_cast<float>(rect.yMins);
    verts[2].u = 0.0f;
    verts[2].v = 0.0f;
    verts[2].color = c2;

    verts[3].x = static_cast<float>(rect.xMaxs);
    verts[3].y = static_cast<float>(rect.yMaxs);
    verts[3].u = 0.0f;
    verts[3].v = 0.0f;
    verts[3].color = c3;

    static const UInt16 indexes[6] = { 0, 1, 2, 2, 1, 3 }; // CCW winding
    draw2DTriangles(verts, lengthOf(verts), indexes, lengthOf(indexes));
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
    const int posX = rect.getPosX();
    const int posY = rect.getPosY();
    const int w = posX + rect.getWidth();
    const int h = posY + rect.getHeight();
    const int wOffs = w + shadowOffset;

    // Shadow is made up of 5 quads, each with one corner for the shadow
    // color and the other 3 for the penumbra, which can be fully transparent.
    // The GPU will properly interpolate the colors and produce a nice gradient
    // effect. Note that the following draw order relies on a CCW polygon winding!

    // bottom-left
    drawRectFilled(makeRect(posX, h, posX + shadowOffset, h + shadowOffset),
                   penumbraColor, penumbraColor,
                   shadowColor, penumbraColor);

    // center-left
    drawRectFilled(makeRect(posX + shadowOffset, h, w, h + shadowOffset),
                   shadowColor, penumbraColor,
                   shadowColor, penumbraColor);

    // bottom-right
    drawRectFilled(makeRect(w, h, wOffs, h + shadowOffset),
                   shadowColor, penumbraColor,
                   penumbraColor, penumbraColor);

    // center-right
    drawRectFilled(makeRect(w, posY + shadowOffset, wOffs, h),
                   shadowColor, shadowColor,
                   penumbraColor, penumbraColor);

    // top-right
    drawRectFilled(makeRect(w, posY, wOffs, posY + shadowOffset),
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
        verts[0].x = static_cast<float>(rect.xMins + (rect.getWidth() / 2));
        verts[0].y = static_cast<float>(rect.yMins);
        verts[1].x = static_cast<float>(rect.xMins);
        verts[1].y = static_cast<float>(rect.yMaxs);
        verts[2].x = static_cast<float>(rect.xMaxs);
        verts[2].y = static_cast<float>(rect.yMaxs);
    }
    else // down
    {
        verts[0].x = static_cast<float>(rect.xMins);
        verts[0].y = static_cast<float>(rect.yMins);
        verts[1].x = static_cast<float>(rect.xMins + (rect.getWidth() / 2));
        verts[1].y = static_cast<float>(rect.yMaxs);
        verts[2].x = static_cast<float>(rect.xMaxs);
        verts[2].y = static_cast<float>(rect.yMins);
    }

    static const UInt16 indexes[3] = { 0, 1, 2 }; // CCW winding
    draw2DTriangles(verts, lengthOf(verts), indexes, lengthOf(indexes));

    // Outline:
    drawLine(verts[0].x, verts[0].y, verts[1].x, verts[1].y, outlineColor);
    drawLine(verts[1].x, verts[1].y, verts[2].x, verts[2].y, outlineColor);
    drawLine(verts[2].x, verts[2].y, verts[0].x, verts[0].y, outlineColor);
}

void GeometryBatch::createGlyphTexture()
{
    const int decompressedSize = getFontCharSet().bitmapDecompressSize;
    UByte * decompressedBitmap = detail::memAlloc<UByte>(decompressedSize);

    // The glyph bitmap is always assumed to be RLE encoded in the format
    // outputted by the font-tool (see: https://github.com/glampert/font-tool)
    if (rleDecode(decompressedBitmap, decompressedSize,
                  getFontBitmapPixels(), getFontBitmapSizeBytes()) <= 0)
    {
        detail::memFree(decompressedBitmap);
        NTB_ERROR("Unable to decompress RLE font bitmap data!");
        return;
    }

    glyphTex = getRenderInterface()->createTexture(
                    getFontCharSet().bitmapWidth,
                    getFontCharSet().bitmapHeight,
                    1, decompressedBitmap);

    // No longer needed.
    detail::memFree(decompressedBitmap);
}

void GeometryBatch::drawTextConstrained(const char * text, const int textLength, Rectangle alignBox,
                                        const Rectangle & clipBox, const float scaling, const Color32 color,
                                        const TextAlign::Enum align)
{
    NTB_ASSERT(text != NTB_NULL);
    if (*text == '\0' || textLength <= 0)
    {
        return;
    }

    const float charWidth = getFontCharSet().charWidth * scaling;
    const float clipBoxWidth = clipBox.getWidth();

    float textWidth = calcTextWidth(text, textLength, scaling);
    int clippedLength = textLength;

    while (textWidth > clipBoxWidth)
    {
        textWidth -= charWidth;
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

    float x = alignBox.getPosX();
    float y = alignBox.getPosY();

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

        if (x >= clipBox.getPosX())
        {
            break;
        }
        alignBox = clipBox;
    }

    drawTextImpl(text, clippedLength, x, y, scaling, color);
}

void GeometryBatch::drawTextImpl(const char * text, const int textLength, float x, float y,
                                 const float scaling, const Color32 color)
{
    NTB_ASSERT(text != NTB_NULL);
    NTB_ASSERT(textLength > 0);

    static const UInt16 indexes[6] = { 0, 1, 2, 2, 1, 3 };

    // Invariants for all characters:
    const float initialX    = x;
    const float charsZ      = static_cast<float>(currZ);
    const float scaleU      = getFontCharSet().bitmapWidth;
    const float scaleV      = getFontCharSet().bitmapHeight;
    const float fixedWidth  = getFontCharSet().charWidth;
    const float fixedHeight = getFontCharSet().charHeight;
    const float tabW        = fixedWidth  * 4.0f * scaling; // TAB = 4 spaces.
    const float chrW        = fixedWidth  * scaling;
    const float chrH        = fixedHeight * scaling;

    for (int c = 0; c < textLength; ++c)
    {
        const int charValue = text[c];
        if (charValue >= FontCharSet::MaxChars)
        {
            continue;
        }
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

        const FontChar fontChar = getFontCharSet().chars[charValue];
        const float u0 = (fontChar.x + 0.5f) / scaleU;
        const float v0 = (fontChar.y + 0.5f) / scaleV;
        const float u1 = u0 + (fixedWidth  / scaleU);
        const float v1 = v0 + (fixedHeight / scaleV);

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

    // It's fine to increment once per string,
    // since chars in a string cannot overlap.
    ++currZ;
}

float GeometryBatch::calcTextWidth(const char * text, const int textLength, const float scaling)
{
    NTB_ASSERT(text != NTB_NULL);

    const float fixedWidth = getFontCharSet().charWidth;
    const float tabW = fixedWidth * 4.0f * scaling; // TAB = 4 spaces.
    const float chrW = fixedWidth * scaling;

    float x = 0.0f;
    for (int c = 0; c < textLength; ++c)
    {
        const int charValue = text[c];
        if (charValue >= FontCharSet::MaxChars)
        {
            continue;
        }

        // Tabs are handled differently (4 spaces)
        if (charValue == '\t')
        {
            x += tabW;
        }
        else // Non-tab char (including whitespace)
        {
            x += chrW;
        }
    }
    return x;
}

} // namespace ntb {}
