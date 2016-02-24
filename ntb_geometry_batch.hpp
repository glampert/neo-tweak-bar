
// ================================================================================================
// -*- C++ -*-
// File: ntb_geometry_batch.hpp
// Author: Guilherme R. Lampert
// Created on: 21/09/15
// Brief: Batched geometry renderer helper (internal library use).
// ================================================================================================

#ifndef NEO_TWEAK_BAR_GEOMETRY_BATCH_HPP
#define NEO_TWEAK_BAR_GEOMETRY_BATCH_HPP

namespace ntb
{

struct TextAlign
{
    enum Enum
    {
        Left,
        Right,
        Center
    };
};

// ========================================================
// class GeometryBatch:
// ========================================================

//TODO Maybe we make this a member of the detail{} namespace as well?
// Then it will be possible to hide the implementation completely
// if we just use pointers/references.
class GeometryBatch NTB_FINAL_CLASS
{
    NTB_DISABLE_COPY_ASSIGN(GeometryBatch);

public:

    GeometryBatch();
    ~GeometryBatch();

    float getNextZ() { return static_cast<float>(currentZ++); }

    void beginDraw();
    void endDraw();

    void drawClipped2DTriangles(const VertexPTC * verts, int vertCount,
                                const UInt16 * indexes, int indexCount,
                                const Rectangle & viewport, const Rectangle & clipBox);

    void draw2DTriangles(const VertexPTC * verts, int vertCount,
                         const UInt16 * indexes, int indexCount);

    void drawLine(int xFrom, int yFrom, int xTo, int yTo,
                  Color32 colorFrom, Color32 colorTo);

    void drawLine(int xFrom, int yFrom, int xTo, int yTo, Color32 color);

    void drawRectFilled(const Rectangle & rect, Color32 c0,
                        Color32 c1, Color32 c2, Color32 c3);

    void drawRectFilled(const Rectangle & rect, Color32 color);

    void drawRectOutline(const Rectangle & rect, Color32 c0,
                         Color32 c1, Color32 c2, Color32 c3);

    void drawRectOutline(const Rectangle & rect, Color32 color);

    void drawRectShadow(const Rectangle & rect, Color32 shadowColor,
                        Color32 penumbraColor, int shadowOffset);

    // Direction: 1=up, -1=down.
    void drawArrowFilled(const Rectangle & rect, Color32 bgColor,
                         Color32 outlineColor, int direction);

    // Handles newlines, spaces and tabs.
    void drawTextConstrained(const char * text, int textLength, Rectangle alignBox,
                             const Rectangle & clipBox, float scaling, Color32 color,
                             TextAlign::Enum align);

    // Width in pixels of a text string using the given font. Doesn't actually draw anything.
    static float calcTextWidth(const char * text, int textLength, float scaling);

    // Width/height in pixels of a single character/glyph of the font in use.
    // Note: We always assume a fixed width and height font.
    static float getCharWidth();
    static float getCharHeight();

private:

    // Calls in the RenderInterface to allocate the glyph bitmap.
    void createGlyphTexture();

    // Handles newlines, spaces and tabs. String doesn't have to be NUL-terminated, we rely on textLength instead.
    void drawTextImpl(const char * text, int textLength, float x, float y, float scaling, Color32 color);

    // The glyph bitmap decompressed and copied into a RenderInterface texture object.
    TextureHandle glyphTex;

    // Z index for all 2D elements. Starts at 0 in beginDraw(),
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

#endif // NEO_TWEAK_BAR_GEOMETRY_BATCH_HPP
