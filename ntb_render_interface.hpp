
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
// Draw vertex types used by NTB and the Texture handle:
// ========================================================

// Vertex with XYZ position, UV texture coords and RGBA(8:8:8:8) color.
struct VertexPTC
{
    float x, y, z;
    float u, v;
    Color32 color;
};

// Vertex with XYZ position and RGBA(8:8:8:8) color.
struct VertexPC
{
    float x, y, z;
    Color32 color;
};

// Opaque handle to a texture type, implemented by the user.
struct OpaqueTextureType;
typedef OpaqueTextureType * TextureHandle;

// ========================================================
// class RenderInterface:
// ========================================================

class RenderInterface
{
public:

    // optional
    virtual void beginDraw();
    virtual void endDraw();

    virtual int getMaxZ() const;

    virtual TextureHandle createTexture(int widthPixels, int heightPixels,
                                        int colorChannels, const void * pixels) = 0;

    virtual void destroyTexture(TextureHandle texture) = 0;

    virtual void draw2DLines(const VertexPC * verts, int vertCount) = 0;

    virtual void draw2DTriangles(const VertexPTC * verts, int vertCount,
                                 const UInt16 * indexes, int indexCount,
                                 TextureHandle texture) = 0;

    virtual void draw3DTriangles(const VertexPTC * verts, int vertCount,
                                 const UInt16 * indexes, int indexCount,
                                 TextureHandle texture) = 0;

    virtual ~RenderInterface();
};

} // namespace ntb {}

#endif // NEO_TWEAK_BAR_RENDER_INTERFACE_HPP
