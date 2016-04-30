
// ================================================================================================
// -*- C++ -*-
// File: ntb_render_interface.hpp
// Author: Guilherme R. Lampert
// Created on: 13/09/15
// Brief: Interface the user implements to provide rendering methods for NeoTweakBar.
// ================================================================================================

#ifndef NEO_TWEAK_BAR_RENDER_INTERFACE_HPP
#define NEO_TWEAK_BAR_RENDER_INTERFACE_HPP

namespace ntb
{

// TODO TEMP: move to a separate place!
class ShellInterface
{
public:
    virtual Int64 getTimeMilliseconds() const = 0;
    virtual ~ShellInterface();
};

// ========================================================

// Opaque handle to a texture type, implemented by the user.
struct OpaqueTextureType;
typedef OpaqueTextureType * TextureHandle;

struct DrawClippedInfo
{
    TextureHandle texture; // may be null
    Rectangle viewport;
    Rectangle clipBox;
    int firstIndex;
    int indexCount;
};

// Vertex with XYZ position, UV texture coords and RGBA(8:8:8:8) color.
struct VertexPTC
{
    Float32 x, y, z;
    Float32 u, v;
    Color32 color;
};

// Vertex with XYZ position and RGBA(8:8:8:8) color.
struct VertexPC
{
    Float32 x, y, z;
    Color32 color;
};

// ========================================================
// class RenderInterface:
// ========================================================

class RenderInterface
{
public:

    // everything is optional

    virtual void beginDraw();
    virtual void endDraw();

    virtual int getMaxZ() const;
    virtual Rectangle getViewport() const;

    virtual TextureHandle createTexture(int widthPixels, int heightPixels,
                                        int colorChannels, const void * pixels);
    virtual void destroyTexture(TextureHandle texture);

    virtual void draw2DLines(const VertexPC * verts, int vertCount, int frameMaxZ);

    virtual void draw2DTriangles(const VertexPTC * verts, int vertCount,
                                 const UInt16 * indexes, int indexCount,
                                 TextureHandle texture, int frameMaxZ);

    virtual void drawClipped2DTriangles(const VertexPTC * verts, int vertCount,
                                        const UInt16 * indexes, int indexCount,
                                        const DrawClippedInfo * drawInfo, int drawInfoCount,
                                        int frameMaxZ);

    virtual ~RenderInterface() = 0;
};

} // namespace ntb {}

#endif // NEO_TWEAK_BAR_RENDER_INTERFACE_HPP
