
// ================================================================================================
// -*- C++ -*-
// File: test_geometry_batch_gl.cpp
// Author: Guilherme R. Lampert
// Created on: 29/08/16
//
// Brief:
//  Various tests for the underlaying NTB RenderInterfaces and GeometryBatch, using OpenGL.
//  --gl-core:   Runs in OpenGL Core Profile mode (GL 3+);
//  --gl-legacy: Runs in Legacy mode (OpenGL 2.0 or lower);
//  If no command line arguments are given, defaults to legacy mode.
// ================================================================================================

//FIXME: gl3w and legacy GL can't live side-by-side in the same file!
//       need to restructure. Maybe add a libSamples.a or something...

#include "ntb.hpp"
#include "ntb_widgets.hpp"

#include <GL/gl3w.h> // An OpenGL extension wrangler (https://github.com/skaslev/gl3w).
#include <GLFW/glfw3.h>
#include <vectormath.h>

#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>

//TODO temp
//#define NTB_DEFAULT_RENDERER_GL_LEGACY
//#include "ntb_renderer_gl_legacy.hpp"

#if !defined(NEO_TWEAK_BAR_STD_STRING_INTEROP)
    #error "NEO_TWEAK_BAR_STD_STRING_INTEROP is required for this sample!"
#endif // NEO_TWEAK_BAR_STD_STRING_INTEROP

// ================================================================================================

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

    void beginDraw() NTB_OVERRIDE;
    void endDraw()   NTB_OVERRIDE;

    void getViewport(int * viewportX, int * viewportY,
                     int * viewportW, int * viewportH) const NTB_OVERRIDE;

    // -- Texture allocation --

    TextureHandle createTexture(int widthPixels, int heightPixels,
                                int colorChannels, const void * pixels) NTB_OVERRIDE;

    void destroyTexture(TextureHandle texture) NTB_OVERRIDE;

    // -- Drawing commands --

    void draw2DLines(const VertexPC * verts, int vertCount, int frameMaxZ) NTB_OVERRIDE;

    void draw2DTriangles(const VertexPTC * verts, int vertCount,
                         const UInt16 * indexes, int indexCount,
                         TextureHandle texture, int frameMaxZ) NTB_OVERRIDE;

    void drawClipped2DTriangles(const VertexPTC * verts, int vertCount,
                                const UInt16 * indexes, int indexCount,
                                const DrawClippedInfo * drawInfo,
                                int drawInfoCount, int frameMaxZ) NTB_OVERRIDE;

private:

    static GLvoid * offsetPtr(std::size_t offset);
    static void checkGLError(const char * file, int line);
    static const char * errorToString(GLenum errorCode);
    static void compileShader(GLuint & shader);
    static void linkProgram(GLuint & program);

    void initShaders();
    void initBuffers();
    void makeWhiteTexture();

    void recordGLStates();
    void restoreGLStates() const;

    //TODO save user buffers/vao and shader prog!
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
    } glStates;

    GLchar glslVersionStr[64];

    bool checkGLErrors; // Defaults to true if DEBUG or _DEBUG.
    bool saveGLStates;  // Always defaults to true.
    bool drawWithDepth; // Always defaults to true.
    bool lineSmooth;    // Always defaults to false.
    int  windowWidth;
    int  windowHeight;

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
    GLTextureRecord * whiteTexture;
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

inline GLvoid * RenderInterfaceDefaultGLCore::offsetPtr(std::size_t offset)
{
    return reinterpret_cast<GLvoid *>(offset);
}

// ================================================================================================

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
    , whiteTexture(NTB_NULL)
{
    std::memset(&glStates,       0, sizeof(glStates));
    std::memset(&glslVersionStr, 0, sizeof(glslVersionStr));

    // Get initial in case user calls getViewport() before a beginDraw/endDraw pair.
    glGetIntegerv(GL_VIEWPORT, glStates.viewport);

    //FIXME temp
    printf("\n");
    printf("viewport.x = %d\n", glStates.viewport[0]);
    printf("viewport.y = %d\n", glStates.viewport[1]);
    printf("viewport.w = %d\n", glStates.viewport[2]);
    printf("viewport.h = %d\n", glStates.viewport[3]);

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

    std::printf("GLSL VER: %s\n", glslVersionStr);//TODO temp

    //
    // Code shared by all Vertex Shaders:
    //
    static const GLchar vsCommon[] =
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
    static const GLchar vsLines2DSource[] =
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
    static const GLchar fsLines2DSource[] =
        "\n"
        "in  vec4 v_Color;\n"
        "out vec4 out_FragColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    out_FragColor = v_Color;\n"
        "}\n";

    vsLines2D = glCreateShader(GL_VERTEX_SHADER);
    const GLchar * vsLines2DStrings[] = { glslVersionStr, vsCommon, vsLines2DSource };
    glShaderSource(vsLines2D, ntb::lengthOfArray(vsLines2DStrings), vsLines2DStrings, NTB_NULL);
    compileShader(vsLines2D);

    fsLines2D = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar * fsLines2DStrings[] = { glslVersionStr, fsLines2DSource };
    glShaderSource(fsLines2D, ntb::lengthOfArray(fsLines2DStrings), fsLines2DStrings, NTB_NULL);
    compileShader(fsLines2D);

    shaderProgLines2D = glCreateProgram();
    glAttachShader(shaderProgLines2D, vsLines2D);
    glAttachShader(shaderProgLines2D, fsLines2D);
    glBindAttribLocation(shaderProgLines2D, 0, "in_Position");
    glBindAttribLocation(shaderProgLines2D, 1, "in_Color");
    linkProgram(shaderProgLines2D);

    shaderProgLines2D_ScreenParams = glGetUniformLocation(shaderProgLines2D, "u_ScreenParams");
    if (shaderProgLines2D_ScreenParams < 0)
    {
        errorF("Unable to get uniform var 'shaderProgLines2D_ScreenParams' location!");
    }

    //
    // 2D/3D triangles shaders:
    //
    static const GLchar vsTris2DSource[] =
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
    static const GLchar fsTris2DSource[] =
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
    const GLchar * vsTris2DStrings[] = { glslVersionStr, vsCommon, vsTris2DSource };
    glShaderSource(vsTris2D, ntb::lengthOfArray(vsTris2DStrings), vsTris2DStrings, NTB_NULL);
    compileShader(vsTris2D);

    fsTris2D = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar * fsTris2DStrings[] = { glslVersionStr, fsTris2DSource };
    glShaderSource(fsTris2D, ntb::lengthOfArray(fsTris2DStrings), fsTris2DStrings, NTB_NULL);
    compileShader(fsTris2D);

    shaderProgTris2D = glCreateProgram();
    glAttachShader(shaderProgTris2D, vsTris2D);
    glAttachShader(shaderProgTris2D, fsTris2D);
    glBindAttribLocation(shaderProgTris2D, 0, "in_Position");
    glBindAttribLocation(shaderProgTris2D, 1, "in_TexCoords");
    glBindAttribLocation(shaderProgTris2D, 2, "in_Color");
    linkProgram(shaderProgTris2D);

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

void RenderInterfaceDefaultGLCore::compileShader(GLuint & shader)
{
    glCompileShader(shader);
    checkGLError(__FILE__, __LINE__);

    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE)
    {
        GLint infoLogLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        if (infoLogLength > 0)
        {
            GLchar strInfoLog[1024] = {'\0'};
            glGetShaderInfoLog(shader, sizeof(strInfoLog), NTB_NULL, strInfoLog);
            errorF("NTB RenderInterfaceDefaultGLCore: Shader compilation failure:\n%s", strInfoLog);
        }
        else
        {
            errorF("NTB RenderInterfaceDefaultGLCore: Shader compilation failure - unknown error.");
        }

        glDeleteShader(shader);
        shader = 0;
    }
}

void RenderInterfaceDefaultGLCore::linkProgram(GLuint & program)
{
    glLinkProgram(program);
    checkGLError(__FILE__, __LINE__);

    GLint status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (status == GL_FALSE)
    {
        GLint infoLogLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

        if (infoLogLength > 0)
        {
            GLchar strInfoLog[1024] = {'\0'};
            glGetProgramInfoLog(program, sizeof(strInfoLog), NTB_NULL, strInfoLog);
            errorF("NTB RenderInterfaceDefaultGLCore: Shader program linking failure:\n%s", strInfoLog);
        }
        else
        {
            errorF("NTB RenderInterfaceDefaultGLCore: Shader program linking failure - unknown error.");
        }

        glDeleteProgram(program);
        program = 0;
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
    const int whiteTexSize = 8;
    ntb::Color32 whitePixels[whiteTexSize * whiteTexSize];
    std::memset(whitePixels, 0xFF, sizeof(whitePixels));
    whiteTexture = reinterpret_cast<GLTextureRecord *>(createTexture(whiteTexSize, whiteTexSize, 4, whitePixels));
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
    NTB_ASSERT(viewportX != NTB_NULL);
    NTB_ASSERT(viewportY != NTB_NULL);
    NTB_ASSERT(viewportW != NTB_NULL);
    NTB_ASSERT(viewportH != NTB_NULL);

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
    NTB_ASSERT(pixels != NTB_NULL);

    GLTextureRecord * newTex = implAllocT<GLTextureRecord>();
    newTex->width  = widthPixels;
    newTex->height = heightPixels;
    newTex->texId  = 0;
    newTex->prev   = NTB_NULL;
    newTex->next   = NTB_NULL;

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
    if (texture == NTB_NULL)
    {
        return;
    }

    GLTextureRecord * found = NTB_NULL;
    GLTextureRecord * iter  = textures.getFirst();

    for (int count = textures.getSize(); count--; iter = iter->next)
    {
        if (iter == reinterpret_cast<GLTextureRecord *>(texture))
        {
            found = iter;
            break;
        }
    }

    if (!found)
    {
        errorF("GL texture handle %p not allocated from this RenderInterface!",
               reinterpret_cast<void *>(texture));
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
    whiteTexture = NTB_NULL;
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
}

void RenderInterfaceDefaultGLCore::draw2DLines(const VertexPC * verts, int vertCount, int frameMaxZ)
{
    NTB_ASSERT(verts != NTB_NULL);
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
                static_cast<GLfloat>(glStates.viewport[2]),
                static_cast<GLfloat>(glStates.viewport[3]),
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
    NTB_ASSERT(verts   != NTB_NULL);
    NTB_ASSERT(indexes != NTB_NULL);
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
    if (texture != NTB_NULL)
    {
        glBindTexture(GL_TEXTURE_2D, reinterpret_cast<GLTextureRecord *>(texture)->texId);
    }
    else
    {
        if (whiteTexture == NTB_NULL)
        {
            makeWhiteTexture();
            NTB_ASSERT(whiteTexture != NTB_NULL);
        }
        glBindTexture(GL_TEXTURE_2D, whiteTexture->texId);
    }

    // Set shader:
    glUseProgram(shaderProgTris2D);

    // Set uniform vec3 u_ScreenParams:
    glUniform3f(shaderProgTris2D_ScreenParams,
                static_cast<GLfloat>(glStates.viewport[2]),
                static_cast<GLfloat>(glStates.viewport[3]),
                static_cast<GLfloat>(frameMaxZ));

    // Set texture to TMU 0:
    glUniform1i(shaderProgTris2D_ColorTexture, 0);

    // Draw call:
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, NTB_NULL);

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
    NTB_ASSERT(verts    != NTB_NULL);
    NTB_ASSERT(indexes  != NTB_NULL);
    NTB_ASSERT(drawInfo != NTB_NULL);

    NTB_ASSERT(vertCount     > 0);
    NTB_ASSERT(indexCount    > 0);
    NTB_ASSERT(drawInfoCount > 0);

    // Assert only.
    (void)vertCount;

    glEnable(GL_SCISSOR_TEST);

    int texturedDraws = 0;
    //TODO

    glDisable(GL_SCISSOR_TEST);
    glViewport(glStates.viewport[0], glStates.viewport[1],
               glStates.viewport[2], glStates.viewport[3]);

    // Restore the default no-texture state assumed by the other draw methods.
    if (texturedDraws > 0)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (checkGLErrors)
    {
        checkGLError(__FILE__, __LINE__);
    }
}

const char * RenderInterfaceDefaultGLCore::errorToString(const GLenum errorCode)
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
}

} // namespace ntb {}

// ================================================================================================

// ========================================================
// MyNTBShellInterfaceGLFW:
// ========================================================

class MyNTBShellInterfaceGLFW : public ntb::ShellInterface
{
public:

    ~MyNTBShellInterfaceGLFW();
    ntb::Int64 getTimeMilliseconds() const NTB_OVERRIDE;
};

MyNTBShellInterfaceGLFW::~MyNTBShellInterfaceGLFW()
{ }

ntb::Int64 MyNTBShellInterfaceGLFW::getTimeMilliseconds() const
{
    const ntb::Float64 seconds = glfwGetTime();
    return static_cast<ntb::Int64>(seconds * 1000.0);
}

// ========================================================

static const int AppWindowWidth  = 1024;
static const int AppWindowHeight = 768;

struct AppContext
{
    GLFWwindow           * window;
    ntb::RenderInterface * renderInterface;
    ntb::ShellInterface  * shellInterface;
    bool                   coreProfile;
};

static GLFWwindow * appInitInternal(const int glVersionMajor, const int glVersionMinor,
                                    const bool coreProfile, const char * const title)
{
    std::printf("\nNTB sample \"%s\" starting up...\n", title);

    if (!glfwInit())
    {
        std::fprintf(stderr, "[APP_ERROR]: Failed to initialize GLFW!\n");
        return NTB_NULL;
    }

    // Things we need for the window / GL render context:
    glfwWindowHint(GLFW_RESIZABLE, false);
    glfwWindowHint(GLFW_DEPTH_BITS, 32);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glVersionMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glVersionMinor);

    if (coreProfile)
    {
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    GLFWwindow * window = glfwCreateWindow(AppWindowWidth, AppWindowHeight, title, NTB_NULL, NTB_NULL);
    if (window == NTB_NULL)
    {
        std::fprintf(stderr, "[APP_ERROR]: Failed to create GLFW window!\n");
        return NTB_NULL;
    }

    glfwMakeContextCurrent(window);

    if (coreProfile)
    {
        if (!gl3wInit())
        {
            std::fprintf(stderr, "[APP_WARNING]: Failed to initialize GL3W extensions library!\n");
        }
        if (!gl3wIsSupported(3, 2))
        {
            std::fprintf(stderr, "[APP_WARNING]: This sample application requires at least OpenGL version 3.2 to run!\n");
        }
    }

    std::printf("GL_VENDOR:    %s\n", glGetString(GL_VERSION));
    std::printf("GL_VERSION:   %s\n", glGetString(GL_VENDOR));
    std::printf("GLSL_VERSION: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    return window;
}

static AppContext appInit(const int argc, const char * argv[], const char * const title)
{
    AppContext ctx;
    std::memset(&ctx, 0, sizeof(ctx));

    int glVersionMajor = 2;
    int glVersionMinor = 0;
    std::string fullTitle = title;

    for (int i = 0; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--gl-core") == 0)
        {
            ctx.coreProfile = true;
            glVersionMajor  = 3;
            glVersionMinor  = 2;
            fullTitle      += " - Core OpenGL";
        }
        else if (std::strcmp(argv[i], "--gl-legacy") == 0)
        {
            ctx.coreProfile = false;
            glVersionMajor  = 2;
            glVersionMinor  = 0;
            fullTitle      += " - Legacy OpenGL";
        }
        else if (std::strcmp(argv[i], "--help") == 0)
        {
            std::printf("\nUsage:\n  $ %s [--gl-code | --gl-legacy | --help]\n", argv[0]);
        }
    }

    ctx.window = appInitInternal(glVersionMajor, glVersionMinor, ctx.coreProfile, fullTitle.c_str());
    if (ctx.window != NTB_NULL)
    {
        if (ctx.coreProfile)
        {
            std::printf("Attempting to initialize sample renderer with GL Core profile...\n");
            ctx.renderInterface = new ntb::RenderInterfaceDefaultGLCore(AppWindowWidth, AppWindowHeight);
        }
        else
        {
            std::printf("Attempting to initialize sample renderer with GL Legacy profile...\n");
            ctx.renderInterface = nullptr;//new ntb::RenderInterfaceDefaultGLLegacy(AppWindowWidth, AppWindowHeight); //TODO
        }

        ctx.shellInterface = new MyNTBShellInterfaceGLFW();
        std::printf("Done!\n\n");
    }

    return ctx;
}

static void appShutdown(AppContext & ctx)
{
    delete ctx.renderInterface;
    delete ctx.shellInterface;

    ctx.renderInterface = NTB_NULL;
    ctx.shellInterface  = NTB_NULL;
    ctx.window          = NTB_NULL;

    if (ctx.coreProfile)
    {
        gl3wShutdown();
    }

    glfwTerminate();
}

static void appFrameUpdate(AppContext & ctx, bool * outIsDone)
{
    // NTB starts writing at Z=0 and increases for each primitive.
    // Since we decide to draw without sorting, then the depth buffer
    // must be cleared to zero before drawing the UI.
    glClearDepth(0);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (outIsDone != NTB_NULL)
    {
        (*outIsDone) = glfwWindowShouldClose(ctx.window);
    }
}

static void appFramePresent(AppContext & ctx)
{
    glfwSwapBuffers(ctx.window);
    glfwPollEvents();
}

static void appMakeScreenProjectedBox(ntb::PODArray * scrProjectedVerts, ntb::PODArray * scrProjectedIndexes,
                                      const ntb::Mat4x4 & modelToWorldMatrix, const ntb::Mat4x4 & viewProjMatrix)
{
    ntb::BoxVert tempBoxVerts[24];
    ntb::UInt16  tempBoxIndexes[36];

    int viewport[4];
    ntb::Rectangle scrViewport;

    ntb::RenderInterface * renderer = ntb::getRenderInterface();
    renderer->getViewport(&viewport[0], &viewport[1], &viewport[2], &viewport[3]);
    scrViewport.set(viewport);

    const ntb::Float32 w          = 0.4f;
    const ntb::Float32 h          = 0.4f;
    const ntb::Float32 d          = 0.4f;
    const ntb::Color32 shadeColor = ntb::packColor(0, 0, 0, 255);
    const ntb::BoxVert * pVert    = tempBoxVerts;
    const ntb::UInt16  * pIndex   = tempBoxIndexes;
    const ntb::Int32 vertCount    = ntb::lengthOfArray(tempBoxVerts);
    const ntb::Int32 indexCount   = ntb::lengthOfArray(tempBoxIndexes);

    // Each face can be colored independently.
    static const ntb::Color32 tempFaceColors[6] = {
        ntb::packColor(0,   200, 200),
        ntb::packColor(200, 0,   200),
        ntb::packColor(200, 200, 0  ),
        ntb::packColor(0,   0,   200),
        ntb::packColor(0,   200, 0  ),
        ntb::packColor(200, 200, 200)
    };
    ntb::makeTexturedBoxGeometry(tempBoxVerts, tempBoxIndexes, tempFaceColors, w, h, d);

    scrProjectedVerts->allocateExact(vertCount);
    scrProjectedIndexes->allocateExact(indexCount);

    for (ntb::Int32 v = 0; v < vertCount; ++v, ++pVert)
    {
        const ntb::Vec3 wp = ntb::Mat4x4::transformPointAffine(pVert->position, modelToWorldMatrix);
        const ntb::Vec3 wn = ntb::Mat4x4::transformPointAffine(pVert->normal,   modelToWorldMatrix);
        const ntb::Color32 vertColor = ntb::blendColors(shadeColor, pVert->color, std::fabs(ntb::clamp(wn.z, -1.0f, 1.0f)));

        ntb::VertexPTC scrVert = { wp.x, wp.y, wp.z, pVert->u, pVert->v, vertColor };
        ntb::screenProjectionXY(scrVert, scrVert, viewProjMatrix, scrViewport);
        scrProjectedVerts->pushBack<ntb::VertexPTC>(scrVert);
    }

    for (ntb::Int32 i = 0; i < indexCount; ++i, ++pIndex)
    {
        scrProjectedIndexes->pushBack<ntb::UInt16>(*pIndex);
    }
}

// ========================================================

int main(const int argc, const char * argv[])
{
    AppContext ctx = appInit(argc, argv, "NTB GeometryBatch Test");
    if (ctx.window == NTB_NULL)
    {
        std::fprintf(stderr, "[APP_ERROR]: Failed to initialize sample app!\n");
        return EXIT_FAILURE;
    }

    ntb::initialize(ctx.shellInterface, ctx.renderInterface);
    {
        bool done = false;
        ntb::GeometryBatch geoBatch;
        ntb::TextureHandle sampleTex = ctx.renderInterface->createCheckerboardTexture(64, 64, 4);

        ntb::PODArray scrProjectedVerts(sizeof(ntb::VertexPTC));
        ntb::PODArray scrProjectedIndexes(sizeof(ntb::UInt16));

        ntb::Float32 rotationDegreesX = 0.0f;
        ntb::Float32 rotationDegreesZ = 0.0f;

        ntb::Mat4x4 modelToWorldMatrix;
        modelToWorldMatrix.setIdentity();

        while (!done)
        {
            appFrameUpdate(ctx, &done);
            geoBatch.beginDraw();

            //
            // Draw a textured quad without batching:
            //
            const ntb::Float32 batchZ    = geoBatch.getNextZ();
            const ntb::UInt16 indexes[]  = { 0, 1, 2, 2, 3, 0 };
            const ntb::VertexPTC verts[] =
            {
                { 10,  10,  batchZ, 0.0f, 0.0f, ntb::packColor(255, 0,   0)   },
                { 10,  200, batchZ, 0.0f, 1.0f, ntb::packColor(0,   255, 0)   },
                { 200, 200, batchZ, 1.0f, 1.0f, ntb::packColor(0,   0,   255) },
                { 200, 10,  batchZ, 0.0f, 1.0f, ntb::packColor(255, 255, 255) }
            };
            ctx.renderInterface->draw2DTriangles(verts, ntb::lengthOfArray(verts),
                                             indexes, ntb::lengthOfArray(indexes),
                                             sampleTex, ctx.renderInterface->getMaxZ());

            //
            // Now add some items to the GeometryBatch:
            //

            // Simple rectangles:
            geoBatch.drawRectOutline(ntb::makeRect(10, 250, 210, 450), ntb::packColor(255, 0, 0));
            geoBatch.drawRectFilled(ntb::makeRect(10, 500, 210, 700),  ntb::packColor(0, 255, 0));

            // Simple text string with a background box and outline:
            const char * hello = "Hello World!";
            const int helloLength = ntb::lengthOfString(hello);
            ntb::Rectangle textAlignBox = ntb::makeRect(10, 850, 500, 950);

            geoBatch.drawRectOutline(textAlignBox, ntb::packColor(255, 255, 0));
            geoBatch.drawRectFilled(textAlignBox.shrunk(10, 10), ntb::packColor(128, 200, 0));

            textAlignBox.moveBy(0, 25);
            geoBatch.drawTextConstrained(hello, helloLength, textAlignBox, textAlignBox, 2.0f,
                                         ntb::packColor(255, 255, 255), ntb::TextAlign::Center);

            // Block with all available characters in the built-in font:
            static const char allChars[] =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
                "abcdefghijklmnopqrstuvwxyz\n"
                "1234567890\n"
                "\"!`?\'.,;:()[]{}<>|/@\\^$-%+=#_&~*\n"
                "¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»\n"
                "¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙ\n"
                "ÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷\n"
                "øùúûüýþÿ\n";

            const int allCharsLength = ntb::lengthOfString(allChars);
            textAlignBox = ntb::makeRect(550, 50, 1500, 1000);

            // Large block of text:
            geoBatch.drawTextConstrained(allChars, allCharsLength, textAlignBox, textAlignBox,
                                         2.0f, ntb::packColor(255, 255, 255), ntb::TextAlign::Center);

            // Small block of text:
            geoBatch.drawTextConstrained(allChars, allCharsLength, textAlignBox.moveBy(0, 600), textAlignBox.moveBy(0, 600),
                                         1.0f, ntb::packColor(0, 200, 200), ntb::TextAlign::Center);

            // Text outline box:
            textAlignBox = ntb::makeRect(550, 50, 1500, 1000);
            geoBatch.drawRectOutline(textAlignBox.moveBy(0, -25), ntb::packColor(255, 255, 0));

            // Some screen-projected 3D geometry:
            ntb::Rectangle clipViewport;
            clipViewport.xMins = textAlignBox.xMins + 20;
            clipViewport.yMins = textAlignBox.yMaxs + 30;
            clipViewport.xMaxs = clipViewport.xMins + 500;
            clipViewport.yMaxs = clipViewport.yMins + 500;

            const ntb::Mat4x4 projMatrix =
                ntb::Mat4x4::perspective(ntb::degToRad(60.0f),
                                         clipViewport.getAspect(),
                                         0.5f, 100.0f);
            const ntb::Mat4x4 viewMatrix =
                ntb::Mat4x4::lookAt(ntb::makeVec3(0.0f, 0.0f, +1.0f),
                                    ntb::makeVec3(0.0f, 0.0f, -1.0f),
                                    ntb::makeVec3(0.0f, 1.0f,  0.0f));
            const ntb::Mat4x4 viewProjMatrix = ntb::Mat4x4::multiply(viewMatrix, projMatrix);

            scrProjectedVerts.clear();
            scrProjectedIndexes.clear();
            appMakeScreenProjectedBox(&scrProjectedVerts, &scrProjectedIndexes, modelToWorldMatrix, viewProjMatrix);

            // Rotate it:
            rotationDegreesX = ntb::normalizeAngle360(rotationDegreesX + 0.07f);
            rotationDegreesZ = ntb::normalizeAngle360(rotationDegreesZ + 0.07f);
            const ntb::Mat4x4 matRx = ntb::Mat4x4::rotationX(ntb::degToRad(rotationDegreesX));
            const ntb::Mat4x4 matRz = ntb::Mat4x4::rotationZ(ntb::degToRad(rotationDegreesZ));
            modelToWorldMatrix = ntb::Mat4x4::multiply(matRz, matRx);

            geoBatch.drawRectFilled(clipViewport, ntb::packColor(200, 200, 200));
            geoBatch.drawClipped2DTriangles(scrProjectedVerts.getData<ntb::VertexPTC>(), scrProjectedVerts.getSize(),
                                            scrProjectedIndexes.getData<ntb::UInt16>(), scrProjectedIndexes.getSize(),
                                            clipViewport, clipViewport);
            geoBatch.drawRectOutline(clipViewport.expanded(10, 10), ntb::packColor(255, 0, 0));

            // Finally, test some overlapping draws to make sure depth testing is working as expected.
            ntb::Rectangle box = ntb::makeRect(1200, 1000, 1400, 1200);
            geoBatch.drawRectFilled(box, ntb::packColor(255, 0, 0));
            geoBatch.drawRectFilled(box.moveBy(40, 40), ntb::packColor(0, 255, 0));
            geoBatch.drawRectFilled(box.moveBy(40, 40), ntb::packColor(0, 0, 255));
            geoBatch.drawRectFilled(box.moveBy(40, 40), ntb::packColor(255, 255, 255));
            geoBatch.drawRectOutline(box.shrunk(50, 50), ntb::packColor(0, 0, 0));
            geoBatch.drawArrowFilled(box.shrunk(80, 80), ntb::packColor(0, 200, 0), ntb::packColor(0, 0, 0), 1);

            geoBatch.endDraw();
            appFramePresent(ctx);
        }
    }

    appShutdown(ctx);
    ntb::shutdown();
}

// ================================================================================================
// GL3W is an OpenGL extension wrangler (https://github.com/skaslev/gl3w).
// This would ideally be built separately as a source file in the project, but to
// simplify things in this demo app, I have just included the .cpp file directly in here.
#include "gl3w.cpp"
// ================================================================================================

