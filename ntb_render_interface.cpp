
#include "neo_tweak_bar.hpp"

namespace ntb
{

ShellInterface::~ShellInterface() { }

// ========================================================
// class RenderInterface:
// ========================================================

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

RenderInterface::~RenderInterface()
{
    // We don't want Clang to give us a '-Wweak-vtables' warning,
    // so defining the destructor here avoids that by anchoring
    // the vtable to this file.
}

// Default stubs:
void RenderInterface::beginDraw() { }
void RenderInterface::endDraw()   { }

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

Color32 lighthenRGB(const Color32 color, const float percent)
{
    UByte bR, bG, bB, bA;
    unpackColor(color, bR, bG, bB, bA);

    const float scale = percent / 100.0f;

    float fR = byteToFloat(bR);
    fR += fR * scale;
    fR = std::min(fR, 1.0f);

    float fG = byteToFloat(bG);
    fG += fG * scale;
    fG = std::min(fG, 1.0f);

    float fB = byteToFloat(bB);
    fB += fB * scale;
    fB = std::min(fB, 1.0f);

    return packColor(floatToByte(fR), floatToByte(fG), floatToByte(fB), bA);
}

Color32 darkenRGB(const Color32 color, const float percent)
{
    UByte bR, bG, bB, bA;
    unpackColor(color, bR, bG, bB, bA);

    const float scale = percent / 100.0f;

    float fR = byteToFloat(bR);
    fR -= fR * scale;
    fR = std::max(fR, 0.0f);

    float fG = byteToFloat(bG);
    fG -= fG * scale;
    fG = std::max(fG, 0.0f);

    float fB = byteToFloat(bB);
    fB -= fB * scale;
    fB = std::max(fB, 0.0f);

    return packColor(floatToByte(fR), floatToByte(fG), floatToByte(fB), bA);
}

Color32 blendColors(const float color1[], const float color2[], const float percent)
{
    const float t = 1.0f - percent;
    const float fR = (t * color1[0]) + (percent * color2[0]);
    const float fG = (t * color1[1]) + (percent * color2[1]);
    const float fB = (t * color1[2]) + (percent * color2[2]);
    const float fA = (t * color1[3]) + (percent * color2[3]);

    return packColor(floatToByte(fR), floatToByte(fG),
                     floatToByte(fB), floatToByte(fA));
}

void RGBToHLS(const float fR, const float fG, const float fB, float & hue, float & light, float & saturation)
{
    // Compute HLS from RGB. The R,G,B triplet is between [0,1],
    // hue is between [0,360], light and saturation are [0,1].

    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;

    if (fR > 0.0f) { r = fR;   }
    if (r  > 1.0f) { r = 1.0f; }
    if (fG > 0.0f) { g = fG;   }
    if (g  > 1.0f) { g = 1.0f; }
    if (fB > 0.0f) { b = fB;   }
    if (b  > 1.0f) { b = 1.0f; }

    float minVal = r;
    if (g < minVal) { minVal = g; }
    if (b < minVal) { minVal = b; }

    float maxVal = r;
    if (g > maxVal) { maxVal = g; }
    if (b > maxVal) { maxVal = b; }

    const float mDiff = maxVal - minVal;
    const float mSum  = maxVal + minVal;
    const float l     = 0.5f * mSum;

    float rNorm = 0.0f;
    float gNorm = 0.0f;
    float bNorm = 0.0f;

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

void HLSToRGB(const float hue, const float light, const float saturation, float & fR, float & fG, float & fB)
{
    // Compute RGB from HLS. The light and saturation are between [0,1]
    // and hue is between [0,360]. The returned R,G,B triplet is between [0,1].

    float rh = 0.0f;
    float rl = 0.0f;
    float rs = 0.0f;

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

    float rm2;
    if (rl <= 0.5f)
    {
        rm2 = rl * (1.0f + rs);
    }
    else
    {
        rm2 = rl + rs - rl * rs;
    }

    const float rm1 = 2.0f * rl - rm2;

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
            static float HLS2RGB(float a, float b, float h)
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
