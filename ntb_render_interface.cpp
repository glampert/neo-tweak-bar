
#include "neo_tweak_bar.hpp"

namespace ntb
{

ShellInterface::~ShellInterface() { }

// ========================================================
// class RenderInterface:
// ========================================================

RenderInterface::~RenderInterface()
{
    // We don't want Clang to give us a '-Wweak-vtables' warning,
    // so defining the destructor here avoids that by anchoring
    // the vtable to this file.
}

int RenderInterface::getMaxZ() const
{
    // We can have this much -1 layers of 2D elements.
    // This is a very reasonable default, so the user
    // probably won't have to change it, but if this
    // is not enough, then the user implementation of
    // RenderInterface can override this method with
    // a larger value.
    return 999999;
}

Rectangle RenderInterface::getViewport() const
{
    // A default arbitrary screen size.
    return makeRect(0, 0, 1024, 768);
}

TextureHandle RenderInterface::createTexture(int, int, int, const void *)
{
    return NTB_NULL;
}

void RenderInterface::destroyTexture(TextureHandle)
{
}

void RenderInterface::beginDraw()
{
}

void RenderInterface::endDraw()
{
}

void RenderInterface::draw2DLines(const VertexPC *, int, int)
{
}

void RenderInterface::draw2DTriangles(const VertexPTC *, int, const UInt16 *, int,
                                      TextureHandle, int)
{
}

void RenderInterface::drawClipped2DTriangles(const VertexPTC *, int, const UInt16 *, int,
                                             const DrawClippedInfo *, int, int)
{
}

// ========================================================
// Color conversions / manipulation helpers:
// ========================================================

void unpackColor(const Color32 color, UByte & r, UByte & g, UByte & b, UByte & a)
{
    b = static_cast<UByte>((color & 0x000000FF) >> 0);
    g = static_cast<UByte>((color & 0x0000FF00) >> 8);
    r = static_cast<UByte>((color & 0x00FF0000) >> 16);
    a = static_cast<UByte>((color & 0xFF000000) >> 24);
}

Color32 lighthenRGB(const Color32 color, const Float32 percent)
{
    UByte bR, bG, bB, bA;
    unpackColor(color, bR, bG, bB, bA);

    const Float32 scale = percent / 100.0f;

    Float32 fR = byteToFloat(bR);
    fR += fR * scale;
    fR = std::min(fR, 1.0f);

    Float32 fG = byteToFloat(bG);
    fG += fG * scale;
    fG = std::min(fG, 1.0f);

    Float32 fB = byteToFloat(bB);
    fB += fB * scale;
    fB = std::min(fB, 1.0f);

    return packColor(floatToByte(fR), floatToByte(fG), floatToByte(fB), bA);
}

Color32 darkenRGB(const Color32 color, const Float32 percent)
{
    UByte bR, bG, bB, bA;
    unpackColor(color, bR, bG, bB, bA);

    const Float32 scale = percent / 100.0f;

    Float32 fR = byteToFloat(bR);
    fR -= fR * scale;
    fR = std::max(fR, 0.0f);

    Float32 fG = byteToFloat(bG);
    fG -= fG * scale;
    fG = std::max(fG, 0.0f);

    Float32 fB = byteToFloat(bB);
    fB -= fB * scale;
    fB = std::max(fB, 0.0f);

    return packColor(floatToByte(fR), floatToByte(fG), floatToByte(fB), bA);
}

Color32 blendColors(const Float32 color1[], const Float32 color2[], const Float32 percent)
{
    const Float32 t = 1.0f - percent;
    const Float32 fR = (t * color1[0]) + (percent * color2[0]);
    const Float32 fG = (t * color1[1]) + (percent * color2[1]);
    const Float32 fB = (t * color1[2]) + (percent * color2[2]);
    const Float32 fA = (t * color1[3]) + (percent * color2[3]);

    return packColor(floatToByte(fR), floatToByte(fG),
                     floatToByte(fB), floatToByte(fA));
}

Color32 blendColors(const Color32 color1, const Color32 color2, const Float32 percent)
{
    UByte r1, g1, b1, a1;
    unpackColor(color1, r1, g1, b1, a1);
    const Float32 fR1 = byteToFloat(r1);
    const Float32 fG1 = byteToFloat(g1);
    const Float32 fB1 = byteToFloat(b1);
    const Float32 fA1 = byteToFloat(a1);

    UByte r2, g2, b2, a2;
    unpackColor(color2, r2, g2, b2, a2);
    const Float32 fR2 = byteToFloat(r2);
    const Float32 fG2 = byteToFloat(g2);
    const Float32 fB2 = byteToFloat(b2);
    const Float32 fA2 = byteToFloat(a2);

    const Float32 t = 1.0f - percent;
    const Float32 finalR = (t * fR1) + (percent * fR2);
    const Float32 finalG = (t * fG1) + (percent * fG2);
    const Float32 finalB = (t * fB1) + (percent * fB2);
    const Float32 finalA = (t * fA1) + (percent * fA2);

    return packColor(floatToByte(finalR), floatToByte(finalG),
                     floatToByte(finalB), floatToByte(finalA));
}

void RGBToHLS(const Float32 fR, const Float32 fG, const Float32 fB, Float32 & hue, Float32 & light, Float32 & saturation)
{
    // Compute HLS from RGB. The R,G,B triplet is between [0,1],
    // hue is between [0,360], light and saturation are [0,1].

    Float32 r = 0.0f;
    Float32 g = 0.0f;
    Float32 b = 0.0f;

    if (fR > 0.0f) { r = fR;   }
    if (r  > 1.0f) { r = 1.0f; }
    if (fG > 0.0f) { g = fG;   }
    if (g  > 1.0f) { g = 1.0f; }
    if (fB > 0.0f) { b = fB;   }
    if (b  > 1.0f) { b = 1.0f; }

    Float32 minVal = r;
    if (g < minVal) { minVal = g; }
    if (b < minVal) { minVal = b; }

    Float32 maxVal = r;
    if (g > maxVal) { maxVal = g; }
    if (b > maxVal) { maxVal = b; }

    const Float32 mDiff = maxVal - minVal;
    const Float32 mSum  = maxVal + minVal;
    const Float32 l     = 0.5f * mSum;

    Float32 rNorm = 0.0f;
    Float32 gNorm = 0.0f;
    Float32 bNorm = 0.0f;

    light = l;

    if (maxVal != minVal)
    {
        rNorm = (maxVal - r) / mDiff;
        gNorm = (maxVal - g) / mDiff;
        bNorm = (maxVal - b) / mDiff;
    }
    else
    {
        saturation = hue = 0.0f;
        return;
    }

    if (l < 0.5f)
    {
        saturation = mDiff / mSum;
    }
    else
    {
        saturation = mDiff / (2.0f - mSum);
    }

    if (r == maxVal)
    {
        hue = 60.0f * (6.0f + bNorm - gNorm);
    }
    else if (g == maxVal)
    {
        hue = 60.0f * (2.0f + rNorm - bNorm);
    }
    else
    {
        hue = 60.0f * (4.0f + gNorm - rNorm);
    }

    if (hue > 360.0f)
    {
        hue -= 360.0f;
    }
}

void HLSToRGB(const Float32 hue, const Float32 light, const Float32 saturation, Float32 & fR, Float32 & fG, Float32 & fB)
{
    // Compute RGB from HLS. The light and saturation are between [0,1]
    // and hue is between [0,360]. The returned R,G,B triplet is between [0,1].

    Float32 rh = 0.0f;
    Float32 rl = 0.0f;
    Float32 rs = 0.0f;

    if (hue > 0.0f)
    {
        rh = hue;
    }
    if (rh > 360.0f)
    {
        rh = 360.0f;
    }

    if (light > 0.0f)
    {
        rl = light;
    }
    if (rl > 1.0f)
    {
        rl = 1.0f;
    }

    if (saturation > 0.0f)
    {
        rs = saturation;
    }
    if (rs > 1.0f)
    {
        rs = 1.0f;
    }

    Float32 rm2;
    if (rl <= 0.5f)
    {
        rm2 = rl * (1.0f + rs);
    }
    else
    {
        rm2 = rl + rs - rl * rs;
    }

    const Float32 rm1 = 2.0f * rl - rm2;

    if (!rs)
    {
        fR = rl;
        fG = rl;
        fB = rl;
    }
    else
    {
        struct Helper
        {
            static Float32 HLS2RGB(Float32 a, Float32 b, Float32 h)
            {
                if (h > 360.0f) { h = h - 360.0f; }
                if (h < 0.0f  ) { h = h + 360.0f; }
                if (h < 60.0f ) { return a + (b - a) * h / 60.0f; }
                if (h < 180.0f) { return b; }
                if (h < 240.0f) { return a + (b - a) * (240.0f - h) / 60.0f; }
                return a;
            }
        };
        fR = Helper::HLS2RGB(rm1, rm2, rh + 120.0f);
        fG = Helper::HLS2RGB(rm1, rm2, rh);
        fB = Helper::HLS2RGB(rm1, rm2, rh - 120.0f);
    }
}

} // namespace ntb {}
