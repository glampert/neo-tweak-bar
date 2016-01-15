
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

namespace detail { class ColorEx; }

// ========================================================
// Text rendering helpers / bitmap fonts:
// ========================================================

struct TextAlign
{
    enum Enum
    {
        Left,
        Right,
        Center
    };
};

//TODO TEMP -------------------------

struct FontChar
{
    unsigned short x;
    unsigned short y;
};

struct FontCharSet
{
    enum { MaxChars = 256 };
    const unsigned char * bitmap;
    int bitmapWidth;
    int bitmapHeight;
    int bitmapColorChannels;
    int bitmapDecompressSize;
    int charBaseHeight;
    int charWidth;
    int charHeight;
    int charCount;
    FontChar chars[MaxChars];
};

// These are defined at the end of the file. Data for the arrays
// was generated with font-tool (https://github.com/glampert/font-tool)
// from the Monoid font face (https://github.com/larsenwork/monoid)
extern const FontCharSet fontMonoid18CharSet;
extern const int fontMonoid18BitmapSizeBytes;
extern const unsigned char fontMonoid18Bitmap[];

// If you decide to change the font, these are the only things that
// need to be updated. The fontXYZCharSet variables are never
// referenced directly in the code, these functions are used instead.
inline const FontCharSet & getFontCharSet() { return fontMonoid18CharSet; }
inline const UByte * getFontBitmapPixels() { return fontMonoid18Bitmap; }
inline int getFontBitmapSizeBytes() { return fontMonoid18BitmapSizeBytes; }

inline
int rleDecode(UByte * output, const int outSizeBytes, const UByte * input, const int inSizeBytes)
{
    if (output == NTB_NULL || input == NTB_NULL)
    {
        return -1;
    }
    if (outSizeBytes <= 0 || inSizeBytes <= 0)
    {
        return -1;
    }

    int bytesWritten = 0;
    for (int i = 0; i < inSizeBytes; i += 2)
    {
        int rleCount = *input++;
        const UByte rleByte = *input++;

        // Replicate the RLE packet.
        while (rleCount--)
        {
            *output++ = rleByte;
            if (++bytesWritten == outSizeBytes && rleCount > 0)
            {
                // Reached end of output and we are not done yet, stop with an error.
                return -1;
            }
        }
    }
    return bytesWritten;
}

//TODO TEMP -------------------------

// ========================================================
// class GeometryBatch:
// ========================================================

//TODO
class GeometryBatch NTB_FINAL_CLASS
{
    NTB_DISABLE_COPY_ASSIGN(GeometryBatch);

public:

    GeometryBatch();
    ~GeometryBatch();

    void beginDraw();
    void endDraw();

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

private:

    // Calls in the RenderInterface to allocate the glyph bitmap.
    void createGlyphTexture();

    // Handles newlines, spaces and tabs. String doesn't have to be NUL-terminated, we rely on textLength instead.
    void drawTextImpl(const char * text, int textLength, float x, float y, float scaling, Color32 color);

    // The glyph bitmap decompressed and copied into a RenderInterface texture object.
    TextureHandle glyphTex;

    // Z index for all 2D elements. Starts at 1 in beginDraw(),
    // incremented for each line/triangle that is added to the batch.
    int currZ;

    // Max value given by RenderInterface::getMaxZ().
    // We assert at endDraw() that currZ is below
    // this limit. Value is cached on beginDraw(), so
    // getMaxZ() should not change during a draw sequence!
    int maxZ;

    // Current offsets for the 2D/text index buffers.
    UInt16 baseVertex2d;
    UInt16 baseVertexText;

    // Batch for 2D colored lines.
    PODArray linesBatch;     // [VertexPC]

    // Batch for all untextured 2D triangles (indexed).
    PODArray verts2dBatch;   // [VertexPTC] Misc 2D elements.
    PODArray tris2dBatch;    // [UInt16]    Triangle indexes for the 2D elements.

    // Batch for all 2D text glyphs (indexed).
    PODArray textVertsBatch; // [VertexPTC] Vertexes for 2D text glyphs.
    PODArray textTrisBatch;  // [UInt16]    Indexes for the 2D text triangles.
};

} // namespace ntb {}

#endif // NEO_TWEAK_BAR_GEOMETRY_BATCH_HPP
