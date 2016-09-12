
// ================================================================================================
// -*- C++ -*-
// File: ntb_widgets.hpp
// Author: Guilherme R. Lampert
// Created on: 25/04/16
// Brief: Widgets are the back-end UI elements/components of NTB.
// ================================================================================================

#ifndef NTB_WIDGETS_HPP
#define NTB_WIDGETS_HPP

#include "ntb_utils.hpp"

namespace ntb
{

struct TextAlign
{
    enum
    {
        Left,
        Right,
        Center
    };
    typedef Int8 Enum;
};

// ========================================================
// class GeometryBatch:
// ========================================================

class GeometryBatch NTB_FINAL_CLASS
{
    NTB_DISABLE_COPY_ASSIGN(GeometryBatch);

public:

     GeometryBatch();
    ~GeometryBatch();

    // Preallocate memory for the draw batches. This is optional since the batches
    // will resize as needed. You can hint the number of lines, quadrilaterals, text
    // glyphs/chars and clipped draw calls needed in each frame.
    void preallocateBatches(int lines, int quads, int textGlyphs, int drawClipped);

    // Draw batch setup. Forwards to RenderInterface::beginDraw/endDraw.
    void beginDraw();
    void endDraw();

    // Filled triangles with clipping (used by the 3D widgets).
    void drawClipped2DTriangles(const VertexPTC * verts, int vertCount,
                                const UInt16 * indexes, int indexCount,
                                const Rectangle & viewport, const Rectangle & clipBox);

    // Filled indexed triangles, without texture:
    void draw2DTriangles(const VertexPTC * verts, int vertCount, const UInt16 * indexes, int indexCount);

    // Lines:
    void drawLine(int xFrom, int yFrom, int xTo, int yTo, Color32 color);
    void drawLine(int xFrom, int yFrom, int xTo, int yTo, Color32 colorFrom, Color32 colorTo);

    // Filled rectangle (possibly translucent):
    void drawRectFilled(const Rectangle & rect, Color32 color);
    void drawRectFilled(const Rectangle & rect, Color32 c0, Color32 c1, Color32 c2, Color32 c3);

    // Rectangle outlines:
    void drawRectOutline(const Rectangle & rect, Color32 color);
    void drawRectOutline(const Rectangle & rect, Color32 c0, Color32 c1, Color32 c2, Color32 c3);

    // Simulates a drop-shadow for the given rectangle by drawing
    // alpha-blended borders to the right and bottom.
    void drawRectShadow(const Rectangle & rect, Color32 shadowColor, Color32 penumbraColor, int shadowOffset);

    // Arrowhead direction: 1=up, -1=down.
    void drawArrowFilled(const Rectangle & rect, Color32 bgColor, Color32 outlineColor, int direction);

    // Handles newlines, spaces and tabs, etc.
    // For efficiency reasons, clipping is done per character, so partly occluded chars will not draw.
    void drawTextConstrained(const char * text, int textLength, Rectangle alignBox, const Rectangle & clipBox,
                             Float32 scaling, Color32 color, TextAlign::Enum align);

    // Width in pixels of a text string using the given font. Doesn't actually draw anything.
    static Float32 calcTextWidth(const char * text, int textLength, Float32 scaling);

    // Width/height in pixels of a single character/glyph of the font in use.
    // Note: We always assume a fixed width and height font.
    static Float32 getCharWidth();
    static Float32 getCharHeight();

    // Next Z layer for the frame. Z layers are reset at beginDraw() and incremented
    // every time this method gets called, so no two draw call will have overlapping Z.
    Float32 getNextZ() { return static_cast<Float32>(currentZ++); }

private:

    // Calls in the RenderInterface to allocate the glyph bitmap.
    void createGlyphTexture();

    // Handles newlines, spaces and tabs. String doesn't have to be NUL-terminated, we rely on textLength instead.
    void drawTextImpl(const char * text, int textLength, Float32 x, Float32 y, Float32 scaling, Color32 color);

    // The glyph bitmap decompressed and copied into a RenderInterface texture object.
    TextureHandle glyphTex;

    // Z layer/index for all 2D elements. Starts at 0 in beginDraw(),
    // incremented for each line/triangle that is added to the batch.
    int currentZ;

    // Current offsets for the 2D/text index buffers.
    UInt16 baseVertex2D;
    UInt16 baseVertexText;
    UInt16 baseVertexClipped;

    // Batch for 2D colored lines.
    PODArray linesBatch;        // [VertexPC]

    // Batch for all untextured 2D triangles (indexed).
    PODArray verts2DBatch;      // [VertexPTC] Miscellaneous 2D elements.
    PODArray tris2DBatch;       // [UInt16]    Triangle indexes for the 2D elements.

    // Batch for all 2D text glyphs (indexed).
    PODArray textVertsBatch;    // [VertexPTC] Vertexes for 2D text glyphs.
    PODArray textTrisBatch;     // [UInt16]    Indexes for the 2D text triangles.

    // Separate batch for the clipped 2D vertexes
    // (normally sent from the 3D widgets).
    PODArray drawClippedInfos;  // [DrawClippedInfo]
    PODArray vertsClippedBatch; // [VertexPTC]
    PODArray trisClippedBatch;  // [UInt16]
};

} // namespace ntb {}

#endif // NTB_WIDGETS_HPP
