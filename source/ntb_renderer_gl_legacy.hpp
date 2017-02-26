
// ================================================================================================
// -*- C++ -*-
// File: ntb_renderer_gl_legacy.hpp
// Author: Guilherme R. Lampert
// Created on: 26/04/16
//
// Brief:
//  Default Legacy OpenGL RenderInterface for NTB. Useful when you just want a quick-n-dirty
//  GL renderer that gets the library running right away. This renderer works well with GL 2.0.
//  This header file is optional and won't be compiled into the library if you don't include it.
//  You still have to provide your platform-specific GL headers before including this file,
//  then #define NTB_DEFAULT_RENDERER_GL_LEGACY before including the file in a .cpp to enable
//  the implementation.
// ================================================================================================

#ifndef NTB_RENDERER_GL_LEGACY_HPP
#define NTB_RENDERER_GL_LEGACY_HPP

#include "ntb.hpp"
#include "ntb_utils.hpp"

namespace ntb
{

// ========================================================
// class RenderInterfaceDefaultGLLegacy:
// ========================================================

class RenderInterfaceDefaultGLLegacy
    : public RenderInterface
{
public:

    RenderInterfaceDefaultGLLegacy(int windowW, int windowH);
    virtual ~RenderInterfaceDefaultGLLegacy();

    // Not copyable.
    RenderInterfaceDefaultGLLegacy(const RenderInterfaceDefaultGLLegacy &) = delete;
    RenderInterfaceDefaultGLLegacy & operator = (const RenderInterfaceDefaultGLLegacy &) = delete;

    // -- Local queries and helpers --

    bool isCheckingGLErrors() const;
    void setCheckGLErrors(bool doCheck);

    bool isSavingGLStates() const;
    void setSaveGLStates(bool doSave);

    bool isDrawingWithDepthTest() const;
    void setDrawWithDepthTest(bool doDepthTest);

    void setWindowDimensions(int w, int h);

    // Explicitly free all allocated textures, invalidating any existing TextureHandles.
    // Implicitly called by the destructor.
    void freeAllTextures();

    //
    // ntb::RenderInterface overrides:
    //

    // -- Miscellaneous --

    void beginDraw() override;
    void endDraw()   override;

    void getViewport(int * viewportX, int * viewportY,
                     int * viewportW, int * viewportH) const override;

    // -- Texture allocation --

    TextureHandle createTexture(int widthPixels, int heightPixels,
                                int colorChannels, const void * pixels) override;

    void destroyTexture(TextureHandle texture) override;

    // -- Drawing commands --

    void draw2DLines(const VertexPC * verts, int vertCount, int frameMaxZ) override;

    void draw2DTriangles(const VertexPTC * verts, int vertCount,
                         const std::uint16_t * indexes, int indexCount,
                         TextureHandle texture, int frameMaxZ) override;

    void drawClipped2DTriangles(const VertexPTC * verts, int vertCount,
                                const std::uint16_t * indexes, int indexCount,
                                const DrawClippedInfo * drawInfo,
                                int drawInfoCount, int frameMaxZ) override;

private:

    static void checkGLError(const char * file, int line);
    static const char * errorToString(GLenum errorCode);
    static void * grayscaleToRgba(int widthPixels, int heightPixels, const void * pixels);

    void recordGLStates();
    void restoreGLStates() const;

    struct {
        bool    texture2DEnabled;
        bool    cullFaceEnabled;
        bool    scissorTestEnabled;
        bool    depthTestEnabled;
        bool    blendEnabled;
        GLint   blendFuncSFactor;
        GLint   blendFuncDFactor;
        GLint   depthFunc;
        GLint   texture2D;
        GLint   matrixMode;
        GLint   viewport[4];
        GLint   scissorBox[4];
        GLfloat matrix[4 * 4];
    } glStates;

    bool checkGLErrors; // Defaults to true if DEBUG or _DEBUG.
    bool saveGLStates;  // Always defaults to true.
    bool drawWithDepth; // Always defaults to true.
    int  windowWidth;
    int  windowHeight;

    struct GLTextureRecord : public ListNode<GLTextureRecord>
    {
        GLint  width;
        GLint  height;
        GLuint texId;
    };
    IntrusiveList<GLTextureRecord> textures;
};

// ========================================================

inline bool RenderInterfaceDefaultGLLegacy::isCheckingGLErrors() const
{
    return checkGLErrors;
}

inline void RenderInterfaceDefaultGLLegacy::setCheckGLErrors(const bool doCheck)
{
    checkGLErrors = doCheck;
}

inline bool RenderInterfaceDefaultGLLegacy::isSavingGLStates() const
{
    return saveGLStates;
}

inline void RenderInterfaceDefaultGLLegacy::setSaveGLStates(const bool doSave)
{
    saveGLStates = doSave;
}

inline bool RenderInterfaceDefaultGLLegacy::isDrawingWithDepthTest() const
{
    return drawWithDepth;
}

inline void RenderInterfaceDefaultGLLegacy::setDrawWithDepthTest(const bool doDepthTest)
{
    drawWithDepth = doDepthTest;
}

inline void RenderInterfaceDefaultGLLegacy::setWindowDimensions(const int w, const int h)
{
    windowWidth  = w;
    windowHeight = h;
}

} // namespace ntb {}

// ================== End of header file ==================
#endif // NTB_RENDERER_GL_LEGACY_HPP
// ================== End of header file ==================

// ================================================================================================
//
//                           RenderInterfaceDefaultGLLegacy Implementation
//
// ================================================================================================

#ifdef NTB_DEFAULT_RENDERER_GL_LEGACY

namespace ntb
{

RenderInterfaceDefaultGLLegacy::RenderInterfaceDefaultGLLegacy(const int windowW, const int windowH)
    #if DEBUG || _DEBUG
    : checkGLErrors(true)
    #else // !DEBUG
    : checkGLErrors(false)
    #endif // DEBUG
    , saveGLStates(true)
    , drawWithDepth(true)
    , windowWidth(windowW)
    , windowHeight(windowH)
{
    std::memset(&glStates, 0, sizeof(glStates));

    // Get initial in case user calls getViewport() before a beginDraw/endDraw pair.
    glGetIntegerv(GL_VIEWPORT, glStates.viewport);
}

RenderInterfaceDefaultGLLegacy::~RenderInterfaceDefaultGLLegacy()
{
    freeAllTextures();
}

void RenderInterfaceDefaultGLLegacy::beginDraw()
{
    if (saveGLStates)
    {
        recordGLStates();
    }

    // Viewport might change from frame to frame, so we always record it.
    glGetIntegerv(GL_VIEWPORT, glStates.viewport);

    // Legacy 2D draw settings:
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, glStates.viewport[2], glStates.viewport[3], 0, -99999, 99999);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);

    if (drawWithDepth)
    {
        // Caller should have already cleared the depth buffer to 0 at some point.
        glDepthFunc(GL_GEQUAL);
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }

    // No texturing as the default.
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (checkGLErrors)
    {
        checkGLError(__FILE__, __LINE__);
    }
}

void RenderInterfaceDefaultGLLegacy::endDraw()
{
    if (saveGLStates)
    {
        restoreGLStates();
    }

    if (checkGLErrors)
    {
        checkGLError(__FILE__, __LINE__);
    }
}

void RenderInterfaceDefaultGLLegacy::getViewport(int * viewportX, int * viewportY,
                                                 int * viewportW, int * viewportH) const
{
    NTB_ASSERT(viewportX != nullptr);
    NTB_ASSERT(viewportY != nullptr);
    NTB_ASSERT(viewportW != nullptr);
    NTB_ASSERT(viewportH != nullptr);

    (*viewportX) = glStates.viewport[0];
    (*viewportY) = glStates.viewport[1];
    (*viewportW) = glStates.viewport[2];
    (*viewportH) = glStates.viewport[3];
}

TextureHandle RenderInterfaceDefaultGLLegacy::createTexture(int widthPixels, int heightPixels,
                                                            int colorChannels, const void * pixels)
{
    NTB_ASSERT(widthPixels   >  0);
    NTB_ASSERT(heightPixels  >  0);
    NTB_ASSERT(colorChannels >  0);
    NTB_ASSERT(colorChannels <= 4); // Up to GL_RGBA
    NTB_ASSERT(pixels != nullptr);

    GLTextureRecord * newTex = implAllocT<GLTextureRecord>();
    newTex->width  = widthPixels;
    newTex->height = heightPixels;
    newTex->texId  = 0;
    newTex->prev   = nullptr;
    newTex->next   = nullptr;

    GLint oldTexture = 0;
    GLint oldUnpackAlign = 0;

    if (saveGLStates)
    {
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexture);
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &oldUnpackAlign);
    }

    glGenTextures(1, &newTex->texId);
    glBindTexture(GL_TEXTURE_2D, newTex->texId);

    // Set the row alignment to the highest value that
    // the size of a row divides evenly. Options are: 8,4,2,1.
    const GLuint rowSizeBytes = widthPixels * colorChannels;
    if ((rowSizeBytes % 8) == 0)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
    }
    else if ((rowSizeBytes % 4) == 0)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    }
    else if ((rowSizeBytes % 2) == 0)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
    }
    else
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }

    bool didConvert;
    if (colorChannels == 1)
    {
        pixels = grayscaleToRgba(widthPixels, heightPixels, pixels);
        didConvert = true;
    }
    else
    {
        didConvert = false;
    }

    const GLenum format = (colorChannels == 3 ? GL_RGB : GL_RGBA);
    glTexImage2D(GL_TEXTURE_2D, 0, format, widthPixels, heightPixels, 0, format, GL_UNSIGNED_BYTE, pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Restore the texture and alignment we had before or just set to 0 if not saving states.
    if (saveGLStates)
    {
        glBindTexture(GL_TEXTURE_2D, oldTexture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, oldUnpackAlign);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }

    if (checkGLErrors)
    {
        checkGLError(__FILE__, __LINE__);
    }

    if (didConvert)
    {
        // Not the user pointer, so free it.
        implFree(const_cast<void *>(pixels));
    }

    textures.pushBack(newTex);
    return reinterpret_cast<TextureHandle>(newTex);
}

void RenderInterfaceDefaultGLLegacy::destroyTexture(TextureHandle texture)
{
    if (texture == nullptr)
    {
        return;
    }

    GLTextureRecord * found = nullptr;
    GLTextureRecord * iter  = textures.getFirst();

    for (int count = textures.getSize(); count--; iter = iter->next)
    {
        if (iter == reinterpret_cast<const GLTextureRecord *>(texture))
        {
            found = iter;
            break;
        }
    }

    if (!found)
    {
        errorF("GL texture handle %p not allocated from this RenderInterface!",
               reinterpret_cast<const void *>(texture));
        return;
    }

    glDeleteTextures(1, &found->texId);
    textures.unlink(found);
    implFree(found);
}

void * RenderInterfaceDefaultGLLegacy::grayscaleToRgba(int widthPixels, int heightPixels, const void * pixels)
{
    struct Rgba
    {
        std::uint8_t r, g, b, a;
    };

    Rgba * expanded = implAllocT<Rgba>(widthPixels * heightPixels);
    const auto p = static_cast<const std::uint8_t *>(pixels);

    // Expand graymap the RGBA:
    const int count = widthPixels * heightPixels;
    for (int i = 0; i < count; ++i)
    {
        expanded[i].r = 255;
        expanded[i].g = 255;
        expanded[i].b = 255;
        expanded[i].a = p[i];
    }

    return expanded;
}

void RenderInterfaceDefaultGLLegacy::freeAllTextures()
{
    GLTextureRecord * iter = textures.getFirst();
    for (int count = textures.getSize(); count--; iter = iter->next)
    {
        glDeleteTextures(1, &iter->texId);
    }

    textures.unlinkAndFreeAll();
}

void RenderInterfaceDefaultGLLegacy::draw2DLines(const VertexPC * verts, int vertCount, int frameMaxZ)
{
    NTB_ASSERT(verts != nullptr);
    NTB_ASSERT(vertCount > 0);

    glBegin(GL_LINES);
    for (int v = 0; v < vertCount; ++v)
    {
        std::uint8_t r, g, b, a;
        unpackColor(verts[v].color, r, g, b, a);
        glColor4ub(r, g, b, a);

        const Float32 z = remap(verts[v].z, 0.0f, static_cast<Float32>(frameMaxZ), 99999.0f, -99999.0f);
        glVertex3f(verts[v].x, verts[v].y, z);
    }
    glEnd();

    if (checkGLErrors)
    {
        checkGLError(__FILE__, __LINE__);
    }
}

void RenderInterfaceDefaultGLLegacy::draw2DTriangles(const VertexPTC * verts, int vertCount,
                                                     const std::uint16_t * indexes, int indexCount,
                                                     TextureHandle texture, int frameMaxZ)
{
    NTB_ASSERT(verts   != nullptr);
    NTB_ASSERT(indexes != nullptr);
    NTB_ASSERT(vertCount  > 0);
    NTB_ASSERT(indexCount > 0);

    // Assert only.
    (void)vertCount;

    if (texture != nullptr)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, reinterpret_cast<const GLTextureRecord *>(texture)->texId);
    }

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < indexCount; ++i)
    {
        const std::uint32_t v = indexes[i];

        std::uint8_t r, g, b, a;
        unpackColor(verts[v].color, r, g, b, a);

        glColor4ub(r, g, b, a);
        glTexCoord2f(verts[v].u, verts[v].v);

        const Float32 z = remap(verts[v].z, 0.0f, static_cast<Float32>(frameMaxZ), 99999.0f, -99999.0f);
        glVertex3f(verts[v].x, verts[v].y, z);
    }
    glEnd();

    // Restore the default no-texture state assumed by the other draw methods.
    if (texture != nullptr)
    {
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (checkGLErrors)
    {
        checkGLError(__FILE__, __LINE__);
    }
}

void RenderInterfaceDefaultGLLegacy::drawClipped2DTriangles(const VertexPTC * verts, int vertCount,
                                                            const std::uint16_t * indexes, int indexCount,
                                                            const DrawClippedInfo * drawInfo,
                                                            int drawInfoCount, int frameMaxZ)
{
    NTB_ASSERT(verts    != nullptr);
    NTB_ASSERT(indexes  != nullptr);
    NTB_ASSERT(drawInfo != nullptr);

    NTB_ASSERT(vertCount     > 0);
    NTB_ASSERT(indexCount    > 0);
    NTB_ASSERT(drawInfoCount > 0);

    // Assert only.
    (void)vertCount;

    glEnable(GL_SCISSOR_TEST);

    int texturedDraws = 0;
    for (int i = 0; i < drawInfoCount; ++i)
    {
        int viewportX = drawInfo[i].viewportX;
        int viewportY = drawInfo[i].viewportY;
        int viewportW = drawInfo[i].viewportW;
        int viewportH = drawInfo[i].viewportH;

        int clipX = drawInfo[i].clipBoxX;
        int clipY = drawInfo[i].clipBoxY;
        int clipW = drawInfo[i].clipBoxW;
        int clipH = drawInfo[i].clipBoxH;

        // Invert Y for OpenGL. In GL the origin of the
        // window/framebuffer is the bottom left corner,
        // and so is the origin of the viewport/scissor-box
        // (that's why the `- viewportH` part is also needed).
        const int framebufferH = glStates.viewport[3] - glStates.viewport[1];
        viewportY = framebufferH - viewportY - viewportH;
        clipY = framebufferH - clipY - clipH;

        glViewport(viewportX, viewportY, viewportW, viewportH);
        glScissor(clipX, clipY, clipW, clipH);

        if (drawInfo[i].texture != nullptr)
        {
            if (texturedDraws == 0)
            {
                glEnable(GL_TEXTURE_2D);
            }

            glBindTexture(GL_TEXTURE_2D, reinterpret_cast<const GLTextureRecord *>(drawInfo[i].texture)->texId);
            ++texturedDraws;
        }

        glBegin(GL_TRIANGLES);
        const int idxCount = drawInfo[i].indexCount;
        const int firstIdx = drawInfo[i].firstIndex;
        for (int j = 0; j < idxCount; ++j)
        {
            const std::uint32_t v = indexes[firstIdx + j];

            std::uint8_t r, g, b, a;
            unpackColor(verts[v].color, r, g, b, a);

            glColor4ub(r, g, b, a);
            glTexCoord2f(verts[v].u, verts[v].v);

            const Float32 z = remap(verts[v].z, 0.0f, static_cast<Float32>(frameMaxZ), 99999.0f, -99999.0f);
            glVertex3f(verts[v].x, verts[v].y, z);
        }
        glEnd();
    }

    glDisable(GL_SCISSOR_TEST);
    glViewport(glStates.viewport[0], glStates.viewport[1],
               glStates.viewport[2], glStates.viewport[3]);

    // Restore the default no-texture state assumed by the other draw methods.
    if (texturedDraws > 0)
    {
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (checkGLErrors)
    {
        checkGLError(__FILE__, __LINE__);
    }
}

const char * RenderInterfaceDefaultGLLegacy::errorToString(const GLenum errorCode)
{
    switch (errorCode)
    {
    case GL_NO_ERROR          : return "GL_NO_ERROR";
    case GL_INVALID_ENUM      : return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE     : return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION : return "GL_INVALID_OPERATION";
    case GL_OUT_OF_MEMORY     : return "GL_OUT_OF_MEMORY";
    case GL_STACK_UNDERFLOW   : return "GL_STACK_UNDERFLOW"; // Legacy only; not used on GL3+
    case GL_STACK_OVERFLOW    : return "GL_STACK_OVERFLOW";  // Legacy only; not used on GL3+
    default                   : return "Unknown GL error";
    } // switch (errorCode)
}

void RenderInterfaceDefaultGLLegacy::checkGLError(const char * const file, const int line)
{
    GLenum err = 0;
    while ((err = glGetError()) != 0)
    {
        errorF("%s(%d) : GL_ERROR=0x%X - %s", file, line, err, errorToString(err));
    }
}

void RenderInterfaceDefaultGLLegacy::recordGLStates()
{
    glStates.texture2DEnabled   = (glIsEnabled(GL_TEXTURE_2D)   == GL_TRUE);
    glStates.depthTestEnabled   = (glIsEnabled(GL_DEPTH_TEST)   == GL_TRUE);
    glStates.cullFaceEnabled    = (glIsEnabled(GL_CULL_FACE)    == GL_TRUE);
    glStates.scissorTestEnabled = (glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE);
    glStates.blendEnabled       = (glIsEnabled(GL_BLEND)        == GL_TRUE);

    glGetIntegerv(GL_DEPTH_FUNC, &glStates.depthFunc);
    glGetIntegerv(GL_BLEND_SRC,  &glStates.blendFuncSFactor);
    glGetIntegerv(GL_BLEND_DST,  &glStates.blendFuncDFactor);

    glGetIntegerv(GL_TEXTURE_BINDING_2D, &glStates.texture2D);
    glGetIntegerv(GL_SCISSOR_BOX, glStates.scissorBox);

    glGetIntegerv(GL_MATRIX_MODE, &glStates.matrixMode);
    glGetFloatv(glStates.matrixMode, glStates.matrix);

    // Viewport will be recorded every frame, regardless of saveGLStates.
}

void RenderInterfaceDefaultGLLegacy::restoreGLStates() const
{
    if (glStates.texture2DEnabled)   { glEnable(GL_TEXTURE_2D);    }
    else                             { glDisable(GL_TEXTURE_2D);   }
    if (glStates.depthTestEnabled)   { glEnable(GL_DEPTH_TEST);    }
    else                             { glDisable(GL_DEPTH_TEST);   }
    if (glStates.cullFaceEnabled)    { glEnable(GL_CULL_FACE);     }
    else                             { glDisable(GL_CULL_FACE);    }
    if (glStates.scissorTestEnabled) { glEnable(GL_SCISSOR_TEST);  }
    else                             { glDisable(GL_SCISSOR_TEST); }
    if (glStates.blendEnabled)       { glEnable(GL_BLEND);         }
    else                             { glDisable(GL_BLEND);        }

    glDepthFunc(glStates.depthFunc);
    glBlendFunc(glStates.blendFuncSFactor, glStates.blendFuncDFactor);

    glBindTexture(GL_TEXTURE_2D, glStates.texture2D);

    glMatrixMode(glStates.matrixMode);
    glLoadMatrixf(glStates.matrix);

    glViewport(glStates.viewport[0], glStates.viewport[1],
               glStates.viewport[2], glStates.viewport[3]);

    glScissor(glStates.scissorBox[0], glStates.scissorBox[1],
              glStates.scissorBox[2], glStates.scissorBox[3]);
}

} // namespace ntb {}

// ================ End of implementation =================
#endif // NTB_DEFAULT_RENDERER_GL_LEGACY
// ================ End of implementation =================
