
// ================================================================================================
// -*- C++ -*-
// File: ntb_renderer_gl_core.hpp
// Author: Guilherme R. Lampert
// Created on: 26/04/16
//
// Brief:
//  Default Core OpenGL RenderInterface for NTB (GL3+). Useful when you just want a quick-n-dirty
//  GL renderer that gets the library running right away. This renderer requires OpenGL version 3
//  or more recent. This header file is optional and won't be compiled into the library if you don't
//  include it. You still have to provide your platform-specific GL headers or extension wranglers
//  before including this file, then #define NTB_DEFAULT_RENDERER_GL_CORE before including the file
//  in a .cpp to enable the implementation.
// ================================================================================================

#ifndef NTB_RENDERER_GL_CORE_HPP
#define NTB_RENDERER_GL_CORE_HPP

#include "ntb.hpp"
#include "ntb_utils.hpp"

namespace ntb
{

// ========================================================
// class RenderInterfaceDefaultGLCore:
// ========================================================

class RenderInterfaceDefaultGLCore
    : public RenderInterface
{
public:

    RenderInterfaceDefaultGLCore(int windowW, int windowH);
    virtual ~RenderInterfaceDefaultGLCore();

    // Not copyable.
    RenderInterfaceDefaultGLCore(const RenderInterfaceDefaultGLCore &) = delete;
    RenderInterfaceDefaultGLCore & operator = (const RenderInterfaceDefaultGLCore &) = delete;

    // -- Local queries and helpers --

    bool isCheckingGLErrors() const;
    void setCheckGLErrors(bool doCheck);

    bool isSavingGLStates() const;
    void setSaveGLStates(bool doSave);

    bool isDrawingWithDepthTest() const;
    void setDrawWithDepthTest(bool doDepthTest);

    bool isDrawingLineSmooth() const;
    void setDrawWithLineSmooth(bool useLineSmooth);

    void setWindowDimensions(int w, int h);
    const char * getGlslVersionString() const;

    // Explicitly free all allocated textures, invalidating any existing TextureHandles.
    // Implicitly called by the destructor.
    void freeAllTextures();

    // Explicitly frees shaders and vertex/index buffers.
    // Implicitly called by the destructor.
    void freeAllShadersAndBuffes();

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
                         const UInt16 * indexes, int indexCount,
                         TextureHandle texture, int frameMaxZ) override;

    void drawClipped2DTriangles(const VertexPTC * verts, int vertCount,
                                const UInt16 * indexes, int indexCount,
                                const DrawClippedInfo * drawInfo,
                                int drawInfoCount, int frameMaxZ) override;

private:

    static void * offsetPtr(std::size_t offset);
    static void checkGLError(const char * file, int line);
    static const char * errorToString(GLenum errorCode);
    static void compileShader(GLuint * shader);
    static void linkProgram(GLuint * program);

    void initShaders();
    void initBuffers();
    void makeWhiteTexture();

    void recordGLStates();
    void restoreGLStates() const;

    struct {
        bool  cullFaceEnabled;
        bool  scissorTestEnabled;
        bool  depthTestEnabled;
        bool  blendEnabled;
        bool  lineSmoothEnabled;
        GLint blendFuncSFactor;
        GLint blendFuncDFactor;
        GLint depthFunc;
        GLint texture2D;
        GLint viewport[4];
        GLint scissorBox[4];
        GLint shaderProg;
        GLint vao;
        GLint vbo;
        GLint ibo;
    } glStates;

    char glslVersionStr[64];

    bool  checkGLErrors; // Defaults to true if DEBUG or _DEBUG.
    bool  saveGLStates;  // Always defaults to true.
    bool  drawWithDepth; // Always defaults to true.
    bool  lineSmooth;    // Always defaults to false.
    GLint windowWidth;
    GLint windowHeight;

    GLuint vao;
    GLuint vboLines2D;
    GLuint vboTris2D;
    GLuint iboTris2D;

    GLuint shaderProgLines2D;
    GLint  shaderProgLines2D_ScreenParams;
    GLuint vsLines2D;
    GLuint fsLines2D;

    GLuint shaderProgTris2D;
    GLint  shaderProgTris2D_ScreenParams;
    GLint  shaderProgTris2D_ColorTexture;
    GLuint vsTris2D;
    GLuint fsTris2D;

    struct GLTextureRecord : public ListNode<GLTextureRecord>
    {
        GLint  width;
        GLint  height;
        GLuint texId;
    };

    IntrusiveList<GLTextureRecord> textures;
    const GLTextureRecord * whiteTexture;
};

// ========================================================

inline bool RenderInterfaceDefaultGLCore::isCheckingGLErrors() const
{
    return checkGLErrors;
}

inline void RenderInterfaceDefaultGLCore::setCheckGLErrors(const bool doCheck)
{
    checkGLErrors = doCheck;
}

inline bool RenderInterfaceDefaultGLCore::isSavingGLStates() const
{
    return saveGLStates;
}

inline void RenderInterfaceDefaultGLCore::setSaveGLStates(const bool doSave)
{
    saveGLStates = doSave;
}

inline bool RenderInterfaceDefaultGLCore::isDrawingWithDepthTest() const
{
    return drawWithDepth;
}

inline void RenderInterfaceDefaultGLCore::setDrawWithDepthTest(const bool doDepthTest)
{
    drawWithDepth = doDepthTest;
}

inline bool RenderInterfaceDefaultGLCore::isDrawingLineSmooth() const
{
    return lineSmooth;
}

inline void RenderInterfaceDefaultGLCore::setDrawWithLineSmooth(const bool useLineSmooth)
{
    lineSmooth = useLineSmooth;
}

inline void RenderInterfaceDefaultGLCore::setWindowDimensions(const int w, const int h)
{
    windowWidth  = w;
    windowHeight = h;
}

inline const char * RenderInterfaceDefaultGLCore::getGlslVersionString() const
{
    return glslVersionStr;
}

inline void * RenderInterfaceDefaultGLCore::offsetPtr(std::size_t offset)
{
    return reinterpret_cast<void *>(offset);
}

} // namespace ntb {}

// ================== End of header file ==================
#endif // NTB_RENDERER_GL_CORE_HPP
// ================== End of header file ==================

// ================================================================================================
//
//                           RenderInterfaceDefaultGLCore Implementation
//
// ================================================================================================

#ifdef NTB_DEFAULT_RENDERER_GL_CORE

namespace ntb
{

RenderInterfaceDefaultGLCore::RenderInterfaceDefaultGLCore(const int windowW, const int windowH)
    #if DEBUG || _DEBUG
    : checkGLErrors(true)
    #else // !DEBUG
    : checkGLErrors(false)
    #endif // DEBUG
    , saveGLStates(true)
    , drawWithDepth(true)
    , lineSmooth(false)
    , windowWidth(windowW)
    , windowHeight(windowH)
    , vao(0)
    , vboLines2D(0)
    , vboTris2D(0)
    , iboTris2D(0)
    , shaderProgLines2D(0)
    , shaderProgLines2D_ScreenParams(-1)
    , vsLines2D(0)
    , fsLines2D(0)
    , shaderProgTris2D(0)
    , shaderProgTris2D_ScreenParams(-1)
    , shaderProgTris2D_ColorTexture(-1)
    , vsTris2D(0)
    , fsTris2D(0)
    , whiteTexture(nullptr)
{
    std::memset(&glStates,       0, sizeof(glStates));
    std::memset(&glslVersionStr, 0, sizeof(glslVersionStr));

    // Get initial in case user calls getViewport() before a beginDraw/endDraw pair.
    glGetIntegerv(GL_VIEWPORT, glStates.viewport);

    initBuffers();
    initShaders();
}

RenderInterfaceDefaultGLCore::~RenderInterfaceDefaultGLCore()
{
    freeAllTextures();
    freeAllShadersAndBuffes();
}

void RenderInterfaceDefaultGLCore::initShaders()
{
    //
    // GLSL #version directive:
    // Queried from the GL driver; This ensures we use the best version available.
    //
    int slMajor    = 0;
    int slMinor    = 0;
    int versionNum = 0;
    const char * versionStr = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));

    if (std::sscanf(versionStr, "%d.%d", &slMajor, &slMinor) == 2)
    {
        versionNum = (slMajor * 100) + slMinor;
    }
    else
    {
        // Fall back to the lowest acceptable version.
        // Assume #version 150 -> OpenGL 3.2
        versionNum = 150;
    }
    std::snprintf(glslVersionStr, sizeof(glslVersionStr), "#version %i\n", versionNum);

    //
    // Code shared by all Vertex Shaders:
    //
    static const char vsCommon[] =
        "\n"
        "float toNormScreenX(float x, float scrWidth)\n"
        "{\n"
        "    return ((2.0 * (x - 0.5)) / scrWidth) - 1.0;\n"
        "}\n"
        "\n"
        "float toNormScreenY(float y, float scrHeight)\n"
        "{\n"
        "    return 1.0 - ((2.0 * (y - 0.5)) / scrHeight);\n"
        "}\n"
        "\n"
        "float remapZ(float z, float inMin, float inMax, float outMin, float outMax)\n"
        "{\n"
        "    return (z - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;\n"
        "}\n";

    //
    // Line draw shaders:
    //
    static const char vsLines2DSource[] =
        "\n"
        "in vec3 in_Position;\n"
        "in vec4 in_Color;\n"
        "uniform vec3 u_ScreenParams;\n"
        "\n"
        "out vec4 v_Color;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    gl_Position.x = toNormScreenX(in_Position.x, u_ScreenParams.x);\n"
        "    gl_Position.y = toNormScreenY(in_Position.y, u_ScreenParams.y);\n"
        "    gl_Position.z = remapZ(in_Position.z, 0.0, u_ScreenParams.z, -1.0, 1.0);\n"
        "    gl_Position.w = 1.0;\n"
        "    v_Color       = in_Color;\n"
        "}\n";
    static const char fsLines2DSource[] =
        "\n"
        "in  vec4 v_Color;\n"
        "out vec4 out_FragColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    out_FragColor = v_Color;\n"
        "}\n";

    vsLines2D = glCreateShader(GL_VERTEX_SHADER);
    const char * vsLines2DStrings[] = { glslVersionStr, vsCommon, vsLines2DSource };
    glShaderSource(vsLines2D, ntb::lengthOfArray(vsLines2DStrings), vsLines2DStrings, nullptr);
    compileShader(&vsLines2D);

    fsLines2D = glCreateShader(GL_FRAGMENT_SHADER);
    const char * fsLines2DStrings[] = { glslVersionStr, fsLines2DSource };
    glShaderSource(fsLines2D, ntb::lengthOfArray(fsLines2DStrings), fsLines2DStrings, nullptr);
    compileShader(&fsLines2D);

    shaderProgLines2D = glCreateProgram();
    glAttachShader(shaderProgLines2D, vsLines2D);
    glAttachShader(shaderProgLines2D, fsLines2D);
    glBindAttribLocation(shaderProgLines2D, 0, "in_Position");
    glBindAttribLocation(shaderProgLines2D, 1, "in_Color");
    linkProgram(&shaderProgLines2D);

    shaderProgLines2D_ScreenParams = glGetUniformLocation(shaderProgLines2D, "u_ScreenParams");
    if (shaderProgLines2D_ScreenParams < 0)
    {
        errorF("Unable to get uniform var 'shaderProgLines2D_ScreenParams' location!");
    }

    //
    // 2D/3D triangles shaders:
    //
    static const char vsTris2DSource[] =
        "\n"
        "in vec3 in_Position;\n"
        "in vec2 in_TexCoords;\n"
        "in vec4 in_Color;\n"
        "uniform vec3 u_ScreenParams;\n"
        "\n"
        "out vec2 v_TexCoords;\n"
        "out vec4 v_Color;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    gl_Position.x = toNormScreenX(in_Position.x, u_ScreenParams.x);\n"
        "    gl_Position.y = toNormScreenY(in_Position.y, u_ScreenParams.y);\n"
        "    gl_Position.z = remapZ(in_Position.z, 0.0, u_ScreenParams.z, -1.0, 1.0);\n"
        "    gl_Position.w = 1.0;\n"
        "    v_TexCoords   = in_TexCoords;\n"
        "    v_Color       = in_Color;\n"
        "}\n";
    static const char fsTris2DSource[] =
        "\n"
        "in vec2 v_TexCoords;\n"
        "in vec4 v_Color;\n"
        "uniform sampler2D u_ColorTexture;\n"
        "\n"
        "out vec4 out_FragColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    out_FragColor = v_Color * texture(u_ColorTexture, v_TexCoords);\n"
        "}\n";

    vsTris2D = glCreateShader(GL_VERTEX_SHADER);
    const char * vsTris2DStrings[] = { glslVersionStr, vsCommon, vsTris2DSource };
    glShaderSource(vsTris2D, ntb::lengthOfArray(vsTris2DStrings), vsTris2DStrings, nullptr);
    compileShader(&vsTris2D);

    fsTris2D = glCreateShader(GL_FRAGMENT_SHADER);
    const char * fsTris2DStrings[] = { glslVersionStr, fsTris2DSource };
    glShaderSource(fsTris2D, ntb::lengthOfArray(fsTris2DStrings), fsTris2DStrings, nullptr);
    compileShader(&fsTris2D);

    shaderProgTris2D = glCreateProgram();
    glAttachShader(shaderProgTris2D, vsTris2D);
    glAttachShader(shaderProgTris2D, fsTris2D);
    glBindAttribLocation(shaderProgTris2D, 0, "in_Position");
    glBindAttribLocation(shaderProgTris2D, 1, "in_TexCoords");
    glBindAttribLocation(shaderProgTris2D, 2, "in_Color");
    linkProgram(&shaderProgTris2D);

    shaderProgTris2D_ScreenParams = glGetUniformLocation(shaderProgTris2D, "u_ScreenParams");
    shaderProgTris2D_ColorTexture = glGetUniformLocation(shaderProgTris2D, "u_ColorTexture");

    if (shaderProgTris2D_ScreenParams < 0)
    {
        errorF("Unable to get uniform var 'shaderProgTris2D_ScreenParams' location!");
    }
    if (shaderProgTris2D_ColorTexture < 0)
    {
        errorF("Unable to get uniform var 'shaderProgTris2D_ColorTexture' location!");
    }
}

void RenderInterfaceDefaultGLCore::compileShader(GLuint * shader)
{
    glCompileShader(*shader);
    checkGLError(__FILE__, __LINE__);

    GLint status = GL_FALSE;
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE)
    {
        GLint infoLogLength = 0;
        glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        if (infoLogLength > 0)
        {
            char strInfoLog[1024] = {'\0'};
            glGetShaderInfoLog(*shader, sizeof(strInfoLog), nullptr, strInfoLog);
            errorF("NTB RenderInterfaceDefaultGLCore: Shader compilation failure:\n%s", strInfoLog);
        }
        else
        {
            errorF("NTB RenderInterfaceDefaultGLCore: Shader compilation failure - unknown error.");
        }

        glDeleteShader(*shader);
        (*shader) = 0;
    }
}

void RenderInterfaceDefaultGLCore::linkProgram(GLuint * program)
{
    glLinkProgram(*program);
    checkGLError(__FILE__, __LINE__);

    GLint status = GL_FALSE;
    glGetProgramiv(*program, GL_LINK_STATUS, &status);

    if (status == GL_FALSE)
    {
        GLint infoLogLength = 0;
        glGetProgramiv(*program, GL_INFO_LOG_LENGTH, &infoLogLength);

        if (infoLogLength > 0)
        {
            char strInfoLog[1024] = {'\0'};
            glGetProgramInfoLog(*program, sizeof(strInfoLog), nullptr, strInfoLog);
            errorF("NTB RenderInterfaceDefaultGLCore: Shader program linking failure:\n%s", strInfoLog);
        }
        else
        {
            errorF("NTB RenderInterfaceDefaultGLCore: Shader program linking failure - unknown error.");
        }

        glDeleteProgram(*program);
        (*program) = 0;
    }
}

void RenderInterfaceDefaultGLCore::initBuffers()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboLines2D);
    glGenBuffers(1, &vboTris2D);
    glGenBuffers(1, &iboTris2D);
}

void RenderInterfaceDefaultGLCore::makeWhiteTexture()
{
    const int whiteTexSize = 8; // 8x8 pixels white RGBA texture
    ntb::Color32 whitePixels[whiteTexSize * whiteTexSize];
    std::memset(whitePixels, 0xFF, sizeof(whitePixels));
    whiteTexture = reinterpret_cast<const GLTextureRecord *>(createTexture(whiteTexSize, whiteTexSize, 4, whitePixels));
}

void RenderInterfaceDefaultGLCore::beginDraw()
{
    if (saveGLStates)
    {
        recordGLStates();
    }

    // Viewport might change from frame to frame, so we always record it.
    glGetIntegerv(GL_VIEWPORT, glStates.viewport);

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

    // Optional; there's little visual improvement with smooth lines, but user is free to enable.
    if (lineSmooth)
    {
        glEnable(GL_LINE_SMOOTH);
    }
    else
    {
        glDisable(GL_LINE_SMOOTH);
    }

    // No texture as the default.
    glBindTexture(GL_TEXTURE_2D, 0);

    // Using a shared VAO to simplify stuff.
    glBindVertexArray(vao);

    if (checkGLErrors)
    {
        checkGLError(__FILE__, __LINE__);
    }
}

void RenderInterfaceDefaultGLCore::endDraw()
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

void RenderInterfaceDefaultGLCore::getViewport(int * viewportX, int * viewportY,
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

TextureHandle RenderInterfaceDefaultGLCore::createTexture(int widthPixels, int heightPixels,
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

    const GLenum format = ((colorChannels == 1) ? GL_RED : (colorChannels == 3) ? GL_RGB : GL_RGBA);
    glTexImage2D(GL_TEXTURE_2D, 0, format, widthPixels, heightPixels, 0, format, GL_UNSIGNED_BYTE, pixels);

    // Alpha texture (used by font bitmaps):
    if (colorChannels == 1)
    {
        // RED only texture. Tell GL to fill RED, GREEN and BLUE with 1
        // and place the first component (RED) in the alpha channel.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_ONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_ONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);
    }

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

    textures.pushBack(newTex);
    return reinterpret_cast<TextureHandle>(newTex);
}

void RenderInterfaceDefaultGLCore::destroyTexture(TextureHandle texture)
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

void RenderInterfaceDefaultGLCore::freeAllTextures()
{
    GLTextureRecord * iter = textures.getFirst();
    for (int count = textures.getSize(); count--; iter = iter->next)
    {
        glDeleteTextures(1, &iter->texId);
    }

    textures.unlinkAndFreeAll();
    whiteTexture = nullptr;
}

void RenderInterfaceDefaultGLCore::freeAllShadersAndBuffes()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vboLines2D);
    glDeleteBuffers(1, &vboTris2D);
    glDeleteBuffers(1, &iboTris2D);

    glDeleteProgram(shaderProgLines2D);
    glDeleteShader(vsLines2D);
    glDeleteShader(fsLines2D);

    glDeleteProgram(shaderProgTris2D);
    glDeleteShader(vsTris2D);
    glDeleteShader(fsTris2D);

    vao               = 0;
    vboLines2D        = 0;
    vboTris2D         = 0;
    iboTris2D         = 0;
    shaderProgLines2D = 0;
    vsLines2D         = 0;
    fsLines2D         = 0;
    shaderProgTris2D  = 0;
    vsTris2D          = 0;
    fsTris2D          = 0;

    shaderProgLines2D_ScreenParams = -1;
    shaderProgTris2D_ScreenParams  = -1;
    shaderProgTris2D_ColorTexture  = -1;
}

void RenderInterfaceDefaultGLCore::draw2DLines(const VertexPC * verts, int vertCount, int frameMaxZ)
{
    NTB_ASSERT(verts != nullptr);
    NTB_ASSERT(vertCount > 0);

    glBindBuffer(GL_ARRAY_BUFFER, vboLines2D);
    glBufferData(GL_ARRAY_BUFFER, vertCount * sizeof(VertexPC), verts, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0); // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPC), offsetPtr(0));

    glEnableVertexAttribArray(1); // Color
    glVertexAttribPointer(1, GL_BGRA, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexPC), offsetPtr(sizeof(float) * 3));

    // Set shader:
    glUseProgram(shaderProgLines2D);

    // Set uniform vec3 u_ScreenParams:
    glUniform3f(shaderProgLines2D_ScreenParams,
                static_cast<GLfloat>(glStates.viewport[2] - glStates.viewport[0]),
                static_cast<GLfloat>(glStates.viewport[3] - glStates.viewport[1]),
                static_cast<GLfloat>(frameMaxZ));

    // Draw call:
    glDrawArrays(GL_LINES, 0, vertCount);

    if (checkGLErrors)
    {
        checkGLError(__FILE__, __LINE__);
    }
}

void RenderInterfaceDefaultGLCore::draw2DTriangles(const VertexPTC * verts, int vertCount,
                                                   const UInt16 * indexes, int indexCount,
                                                   TextureHandle texture, int frameMaxZ)
{
    NTB_ASSERT(verts   != nullptr);
    NTB_ASSERT(indexes != nullptr);
    NTB_ASSERT(vertCount  > 0);
    NTB_ASSERT(indexCount > 0);

    glBindBuffer(GL_ARRAY_BUFFER, vboTris2D);
    glBufferData(GL_ARRAY_BUFFER, vertCount * sizeof(VertexPTC), verts, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboTris2D);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(UInt16), indexes, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0); // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTC), offsetPtr(0));

    glEnableVertexAttribArray(1); // Texture coordinate
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexPTC), offsetPtr(sizeof(float) * 3));

    glEnableVertexAttribArray(2); // Color
    glVertexAttribPointer(2, GL_BGRA, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexPTC), offsetPtr(sizeof(float) * 5));

    // Texture is optional.
    // If not set, use a default white texture so we can share the same shader program.
    glActiveTexture(GL_TEXTURE0);
    if (texture != nullptr)
    {
        glBindTexture(GL_TEXTURE_2D, reinterpret_cast<const GLTextureRecord *>(texture)->texId);
    }
    else
    {
        if (whiteTexture == nullptr)
        {
            makeWhiteTexture();
            NTB_ASSERT(whiteTexture != nullptr);
        }
        glBindTexture(GL_TEXTURE_2D, whiteTexture->texId);
    }

    // Set shader:
    glUseProgram(shaderProgTris2D);

    // Set uniform vec3 u_ScreenParams:
    glUniform3f(shaderProgTris2D_ScreenParams,
                static_cast<GLfloat>(glStates.viewport[2] - glStates.viewport[0]),
                static_cast<GLfloat>(glStates.viewport[3] - glStates.viewport[1]),
                static_cast<GLfloat>(frameMaxZ));

    // Set texture to TMU 0:
    glUniform1i(shaderProgTris2D_ColorTexture, 0);

    // Draw call:
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, nullptr);

    if (checkGLErrors)
    {
        checkGLError(__FILE__, __LINE__);
    }
}

void RenderInterfaceDefaultGLCore::drawClipped2DTriangles(const VertexPTC * verts, int vertCount,
                                                          const UInt16 * indexes, int indexCount,
                                                          const DrawClippedInfo * drawInfo,
                                                          int drawInfoCount, int frameMaxZ)
{
    NTB_ASSERT(verts    != nullptr);
    NTB_ASSERT(indexes  != nullptr);
    NTB_ASSERT(drawInfo != nullptr);

    NTB_ASSERT(vertCount     > 0);
    NTB_ASSERT(indexCount    > 0);
    NTB_ASSERT(drawInfoCount > 0);

    glBindBuffer(GL_ARRAY_BUFFER, vboTris2D);
    glBufferData(GL_ARRAY_BUFFER, vertCount * sizeof(VertexPTC), verts, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboTris2D);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(UInt16), indexes, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0); // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTC), offsetPtr(0));

    glEnableVertexAttribArray(1); // Texture coordinate
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexPTC), offsetPtr(sizeof(float) * 3));

    glEnableVertexAttribArray(2); // Color
    glVertexAttribPointer(2, GL_BGRA, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexPTC), offsetPtr(sizeof(float) * 5));

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_SCISSOR_TEST);

    // Set shader:
    glUseProgram(shaderProgTris2D);

    // Set uniform vec3 u_ScreenParams:
    glUniform3f(shaderProgTris2D_ScreenParams,
                static_cast<GLfloat>(glStates.viewport[2] - glStates.viewport[0]),
                static_cast<GLfloat>(glStates.viewport[3] - glStates.viewport[1]),
                static_cast<GLfloat>(frameMaxZ));

    // Set texture to TMU 0 (if any):
    glUniform1i(shaderProgTris2D_ColorTexture, 0);

    GLuint currentTexId = 0;
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

        const GLTextureRecord * texGl = reinterpret_cast<const GLTextureRecord *>(drawInfo[i].texture);
        if (texGl != nullptr && texGl->texId != currentTexId)
        {
            currentTexId = texGl->texId;
            glBindTexture(GL_TEXTURE_2D, currentTexId);
        }
        else
        {
            if (whiteTexture == nullptr)
            {
                makeWhiteTexture();
                NTB_ASSERT(whiteTexture != nullptr);
            }

            if (whiteTexture->texId != currentTexId)
            {
                currentTexId = whiteTexture->texId;
                glBindTexture(GL_TEXTURE_2D, currentTexId);
            }
        }

        // Issue the draw call:
        glDrawElements(GL_TRIANGLES, drawInfo[i].indexCount,
                       GL_UNSIGNED_SHORT, offsetPtr(drawInfo[i].firstIndex));
    }

    glDisable(GL_SCISSOR_TEST);
    glViewport(glStates.viewport[0], glStates.viewport[1],
               glStates.viewport[2], glStates.viewport[3]);

    if (checkGLErrors)
    {
        checkGLError(__FILE__, __LINE__);
    }
}

const char * RenderInterfaceDefaultGLCore::errorToString(const GLenum errorCode)
{
    switch (errorCode)
    {
    case GL_NO_ERROR                      : return "GL_NO_ERROR";
    case GL_INVALID_ENUM                  : return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE                 : return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION             : return "GL_INVALID_OPERATION";
    case GL_OUT_OF_MEMORY                 : return "GL_OUT_OF_MEMORY";
    case GL_INVALID_FRAMEBUFFER_OPERATION : return "GL_INVALID_FRAMEBUFFER_OPERATION";
    default                               : return "Unknown GL error";
    } // switch (errorCode)
}

void RenderInterfaceDefaultGLCore::checkGLError(const char * const file, const int line)
{
    GLenum err = 0;
    while ((err = glGetError()) != 0)
    {
        errorF("%s(%d) : GL_ERROR=0x%X - %s", file, line, err, errorToString(err));
    }
}

void RenderInterfaceDefaultGLCore::recordGLStates()
{
    glStates.depthTestEnabled   = (glIsEnabled(GL_DEPTH_TEST)   == GL_TRUE);
    glStates.cullFaceEnabled    = (glIsEnabled(GL_CULL_FACE)    == GL_TRUE);
    glStates.scissorTestEnabled = (glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE);
    glStates.blendEnabled       = (glIsEnabled(GL_BLEND)        == GL_TRUE);
    glStates.lineSmoothEnabled  = (glIsEnabled(GL_LINE_SMOOTH)  == GL_TRUE);

    glGetIntegerv(GL_DEPTH_FUNC, &glStates.depthFunc);
    glGetIntegerv(GL_BLEND_SRC,  &glStates.blendFuncSFactor);
    glGetIntegerv(GL_BLEND_DST,  &glStates.blendFuncDFactor);

    glGetIntegerv(GL_TEXTURE_BINDING_2D, &glStates.texture2D);
    glGetIntegerv(GL_SCISSOR_BOX, glStates.scissorBox);

    glGetIntegerv(GL_CURRENT_PROGRAM, &glStates.shaderProg);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &glStates.vao);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &glStates.vbo);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &glStates.ibo);

    // Viewport will be recorded every frame, regardless of saveGLStates.
}

void RenderInterfaceDefaultGLCore::restoreGLStates() const
{
    if (glStates.depthTestEnabled)   { glEnable(GL_DEPTH_TEST);    }
    else                             { glDisable(GL_DEPTH_TEST);   }
    if (glStates.cullFaceEnabled)    { glEnable(GL_CULL_FACE);     }
    else                             { glDisable(GL_CULL_FACE);    }
    if (glStates.scissorTestEnabled) { glEnable(GL_SCISSOR_TEST);  }
    else                             { glDisable(GL_SCISSOR_TEST); }
    if (glStates.blendEnabled)       { glEnable(GL_BLEND);         }
    else                             { glDisable(GL_BLEND);        }
    if (glStates.lineSmoothEnabled)  { glEnable(GL_LINE_SMOOTH);   }
    else                             { glDisable(GL_LINE_SMOOTH);  }

    glDepthFunc(glStates.depthFunc);
    glBlendFunc(glStates.blendFuncSFactor, glStates.blendFuncDFactor);

    glBindTexture(GL_TEXTURE_2D, glStates.texture2D);

    glViewport(glStates.viewport[0], glStates.viewport[1],
               glStates.viewport[2], glStates.viewport[3]);

    glScissor(glStates.scissorBox[0], glStates.scissorBox[1],
              glStates.scissorBox[2], glStates.scissorBox[3]);

    if (glStates.vao != 0)
    {
        glBindVertexArray(glStates.vao);
    }
    if (glStates.vbo != 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, glStates.vbo);
    }
    if (glStates.ibo != 0)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glStates.ibo);
    }
    if (glStates.shaderProg != 0)
    {
        glUseProgram(glStates.shaderProg);
    }
}

} // namespace ntb {}

// ================ End of implementation =================
#endif // NTB_DEFAULT_RENDERER_GL_CORE
// ================ End of implementation =================
