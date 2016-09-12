
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstring>

#include <iostream>
#include <vector>
#include <string>

#include <GL/gl3w.h> // An OpenGL extension wrangler (https://github.com/skaslev/gl3w).
#include <GLFW/glfw3.h>
#include <vectormath.h>

#include "neo_tweak_bar.hpp"

//TEMP
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-const-variable"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wweak-vtables"

// Check for a couple C++11 goodies we'd like to use if available...
#if NEO_TWEAK_BAR_CXX11_SUPPORTED
#define OVERRIDE_METHOD override
#define FINAL_CLASS final
#define NULLPTR nullptr
#else // !C++11
#define OVERRIDE_METHOD
#define FINAL_CLASS
#define NULLPTR NULL
#endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

// App window dimensions; Not resizable.
static const int windowWidth = 1024;
static const int windowHeight = 768;

// Time in milliseconds since the application started.
static ntb::Int64 getTimeMilliseconds()
{
    const ntb::Float64 seconds = glfwGetTime();
    return static_cast<ntb::Int64>(seconds * 1000.0);
}

// ================================================================================================

class NtbShellInterfaceGLFW FINAL_CLASS
    : public ntb::ShellInterface
{
public:

    ntb::Int64 getTimeMilliseconds() const { return ::getTimeMilliseconds(); }
};

// ================================================================================================

static const char * getGLErrorString(const GLenum errorCode)
{
    switch (errorCode)
    {
    case GL_NO_ERROR :
        return "GL_NO_ERROR";
    case GL_INVALID_ENUM :
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE :
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION :
        return "GL_INVALID_OPERATION";
    case GL_INVALID_FRAMEBUFFER_OPERATION :
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_OUT_OF_MEMORY :
        return "GL_OUT_OF_MEMORY";
    case GL_STACK_UNDERFLOW :
        return "GL_STACK_UNDERFLOW"; // Legacy; not used on GL3+
    case GL_STACK_OVERFLOW :
        return "GL_STACK_OVERFLOW"; // Legacy; not used on GL3+
    default :
        return "Unknown GL error";
    } // switch (errorCode)
}

static void checkGLError(const char * file, int line, const char * func)
{
    GLenum err = 0;
    char msg[512];
    while ((err = glGetError()) != 0)
    {
        sprintf(msg, "%s(%d) : [%s] GL_CORE_ERROR=0x%x ( %s )\n", file, line, func, err, getGLErrorString(err));
        fprintf(stderr, "%s", msg);
    }
}
#define CHECK_GL_ERRORS() checkGLError(__FILE__, __LINE__, __func__)

static GLuint compileShader(GLuint shader)
{
    glCompileShader(shader);
    CHECK_GL_ERRORS();

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    CHECK_GL_ERRORS();

    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        CHECK_GL_ERRORS();
        GLchar strInfoLog[512];
        glGetShaderInfoLog(shader, sizeof(strInfoLog), NULLPTR, strInfoLog);
        CHECK_GL_ERRORS();
        fprintf(stderr, "Compile failure: %s\n", strInfoLog);
        shader = 0;
    }

    return shader;
}

static GLuint linkProgram(GLuint program)
{
    glLinkProgram(program);
    CHECK_GL_ERRORS();

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    CHECK_GL_ERRORS();

    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
        CHECK_GL_ERRORS();
        GLchar strInfoLog[512];
        glGetProgramInfoLog(program, sizeof(strInfoLog), NULLPTR, strInfoLog);
        CHECK_GL_ERRORS();
        fprintf(stderr, "Shader linker failure: %s\n", strInfoLog);
        program = 0;
    }

    return program;
}

static float toNormScreenX(float x, int scrWidth)
{
    return ((2.0f * (x - 0.5f)) / scrWidth) - 1.0f;
}

static float toNormScreenY(float y, int scrHeight)
{
    return 1.0f - ((2.0f * (y - 0.5f)) / scrHeight);
}

static GLvoid * offsetPtr(std::size_t offset)
{
    return reinterpret_cast<GLvoid *>(offset);
}

static const ntb::Color32 * mkWhiteTex()
{
    static ntb::Color32 img[64 * 64];
    for (int i = 0; i < 64 * 64; ++i)
    {
        img[i] = ntb::packColor(255, 255, 255, 255);
    }
    return img;
}

// ================================================================================================

class NtbRenderInterfaceCoreGL FINAL_CLASS
    : public ntb::RenderInterface
{
public:

    NtbRenderInterfaceCoreGL();
    ~NtbRenderInterfaceCoreGL();

    void beginDraw() OVERRIDE_METHOD;
    void endDraw() OVERRIDE_METHOD;

    ntb::Rectangle getViewport() const NTB_OVERRIDE
    {
        return ntb::makeRect(0, 0, windowWidth, windowHeight);
    }

    ntb::TextureHandle createTexture(int widthPixels, int heightPixels,
                                     int colorChannels, const void * pixels) OVERRIDE_METHOD;

    void destroyTexture(ntb::TextureHandle texture) OVERRIDE_METHOD;

    void drawClipped2DTriangles(const ntb::VertexPTC * verts, int vertCount,
                                const ntb::UInt16 * indexes, int indexCount,
                                const ntb::DrawClippedInfo * drawInfo, int drawInfoCount,
                                int frameMaxZ) OVERRIDE_METHOD;

    void draw2DTriangles(const ntb::VertexPTC * verts, int vertCount,
                         const ntb::UInt16 * indexes, int indexCount,
                         ntb::TextureHandle texture, int frameMaxZ) OVERRIDE_METHOD;

    // each vertex pair is a line.
    void draw2DLines(const ntb::VertexPC * verts, int vertCount, int frameMaxZ) OVERRIDE_METHOD;

private:

    void initShaders();
    void initBuffers();

    bool noDraw2D = false;
    bool noDraw3D = false;

    struct
    {
        GLint viewport[4];
    } savedState;

    ntb::TextureHandle whiteTex;

    GLuint lines2dProgram;
    GLuint lines2dVS;
    GLuint lines2dFS;

    GLuint tris2dProgram;
    GLuint tris3dProgram;
    GLuint tris2dVS;
    GLuint tris2dFS;

    GLuint tris3dVS;
    GLint mvpMatrixLoc;

    GLuint commonVAO;
    GLuint lines2dVBO;
    GLuint tris2dVBO;
    GLuint tris2dIBO;

    struct GLTexture
    {
        GLuint textureId;
        GLint width;
        GLint height;
    };

    std::vector<ntb::VertexPTC> temp2dVerts;
    std::vector<ntb::VertexPC> temp2dLines;
};

// ========================================================

NtbRenderInterfaceCoreGL::NtbRenderInterfaceCoreGL()
{
    initShaders();
    initBuffers();

    //  glEnable(GL_LINE_SMOOTH); // works, but not strictly necessary
    //  glLineWidth(4.0f); // doesn't work, but we assume width=1 anyways

    whiteTex = createTexture(64, 64, 4, (unsigned char *)mkWhiteTex());

    glGetIntegerv(GL_VIEWPORT, savedState.viewport);
    printf("\n");
    printf("viewport.x = %d\n", savedState.viewport[0]);
    printf("viewport.y = %d\n", savedState.viewport[1]);
    printf("viewport.w = %d\n", savedState.viewport[2]);
    printf("viewport.h = %d\n", savedState.viewport[3]);
}

NtbRenderInterfaceCoreGL::~NtbRenderInterfaceCoreGL()
{
    // TODO cleanup!
}

void NtbRenderInterfaceCoreGL::initShaders()
{
    // TODO don't hardcode GLSL #version! Get is from the GL!

    //
    // Line draw shaders:
    //
    {
        const GLchar * lines2dVSsrc[] =
        {
          "#version 410 core\n"
          "in vec3 inPosition;\n"
          "in vec4 inColor;\n"
          "\n"
          "out vec4 vColor;\n"
          "void main()\n"
          "{\n"
          "    gl_Position = vec4(inPosition, 1.0);\n"
          "    vColor = inColor;\n"
          "}\n"
        };
        lines2dVS = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(lines2dVS, 1, lines2dVSsrc, NULLPTR);
        compileShader(lines2dVS);

        const GLchar * lines2dFSsrc[] =
        {
          "#version 410 core\n"
          "in  vec4 vColor;\n"
          "out vec4 outColor;\n"
          "void main()\n"
          "{\n"
          "    outColor = vColor;\n"
          "}\n"
        };
        lines2dFS = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(lines2dFS, 1, lines2dFSsrc, NULLPTR);
        compileShader(lines2dFS);

        lines2dProgram = glCreateProgram();
        glAttachShader(lines2dProgram, lines2dVS);
        glAttachShader(lines2dProgram, lines2dFS);
        glBindAttribLocation(lines2dProgram, 0, "inPosition");
        glBindAttribLocation(lines2dProgram, 1, "inColor");
        linkProgram(lines2dProgram);
    }

    //
    // 2D/3D tris shaders:
    //
    {
        const GLchar * tris2dVSsrc[] =
        {
          "#version 410 core\n"
          "in vec3 inPosition;\n"
          "in vec2 inTexCoords;\n"
          "in vec4 inColor;\n"
          "\n"
          "out vec2 vTexCoords;\n"
          "out vec4 vColor;\n"
          "void main()\n"
          "{\n"
          "    gl_Position = vec4(inPosition, 1.0);\n"
          "    vTexCoords = inTexCoords;\n"
          "    vColor = inColor;\n"
          "}\n"
        };
        tris2dVS = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(tris2dVS, 1, tris2dVSsrc, NULLPTR);
        compileShader(tris2dVS);

        const GLchar * tris2dFSsrc[] =
        {
          "#version 410 core\n"
          "uniform sampler2D colorTexture;\n"
          "in  vec2 vTexCoords;\n"
          "in  vec4 vColor;\n"
          "out vec4 outColor;\n"
          "void main()\n"
          "{\n"
          "    outColor = vColor;\n"
          "    outColor.a *= texture(colorTexture, vTexCoords).r;\n"
          "}\n"
        };
        tris2dFS = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(tris2dFS, 1, tris2dFSsrc, NULLPTR);
        compileShader(tris2dFS);

        tris2dProgram = glCreateProgram();
        glAttachShader(tris2dProgram, tris2dVS);
        glAttachShader(tris2dProgram, tris2dFS);
        glBindAttribLocation(tris2dProgram, 0, "inPosition");
        glBindAttribLocation(tris2dProgram, 1, "inTexCoords");
        glBindAttribLocation(tris2dProgram, 2, "inColor");
        linkProgram(tris2dProgram);

        // ******************

        const GLchar * tris3dVSsrc[] =
        {
          "#version 410 core\n"
          "in vec3 inPosition;\n"
          "in vec2 inTexCoords;\n"
          "in vec4 inColor;\n"
          "\n"
          "uniform mat4 u_MvpMatrix;\n"
          "\n"
          "out vec2 vTexCoords;\n"
          "out vec4 vColor;\n"
          "void main()\n"
          "{\n"
          "    gl_Position = u_MvpMatrix * vec4(inPosition, 1.0);\n"
          "    vTexCoords = inTexCoords;\n"
          "    vColor = inColor;\n"
          "}\n"
        };
        tris3dVS = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(tris3dVS, 1, tris3dVSsrc, NULLPTR);
        compileShader(tris3dVS);

        // We can share the fragment shader
        //
        tris3dProgram = glCreateProgram();
        glAttachShader(tris3dProgram, tris3dVS);
        glAttachShader(tris3dProgram, tris2dFS);
        glBindAttribLocation(tris3dProgram, 0, "inPosition");
        glBindAttribLocation(tris3dProgram, 1, "inTexCoords");
        glBindAttribLocation(tris3dProgram, 2, "inColor");
        linkProgram(tris3dProgram);

        mvpMatrixLoc = glGetUniformLocation(tris3dProgram, "u_MvpMatrix");
        assert(mvpMatrixLoc >= 0);
    }
}

void NtbRenderInterfaceCoreGL::initBuffers()
{
    glGenVertexArrays(1, &commonVAO);
    glGenBuffers(1, &lines2dVBO);
    glGenBuffers(1, &tris2dVBO);
    glGenBuffers(1, &tris2dIBO);
}

void NtbRenderInterfaceCoreGL::beginDraw()
{
    // to restore if we change it on draw 3D
	glGetIntegerv(GL_VIEWPORT, savedState.viewport);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);

    glDepthFunc(GL_GEQUAL);
    glEnable(GL_DEPTH_TEST);

    // use a shared VAO to simplify stuff...
    glBindVertexArray(commonVAO);

    CHECK_GL_ERRORS();
}

void NtbRenderInterfaceCoreGL::endDraw()
{
    // Cleanup (optional)
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    CHECK_GL_ERRORS();
}

ntb::TextureHandle NtbRenderInterfaceCoreGL::createTexture(int widthPixels, int heightPixels,
                                                           int colorChannels, const void * pixels)
{
    assert(widthPixels   > 0);
    assert(heightPixels  > 0);
    assert(colorChannels > 0);
    assert(colorChannels <= 4); // Up to RGBA
    assert(pixels != NULLPTR);

    GLTexture * newTex = new GLTexture();
    newTex->width = widthPixels;
    newTex->height = heightPixels;

    glGenTextures(1, &newTex->textureId);
    glBindTexture(GL_TEXTURE_2D, newTex->textureId);

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

    const GLenum format = (colorChannels == 1) ? GL_RED : (colorChannels == 3) ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, format, widthPixels, heightPixels, 0, format, GL_UNSIGNED_BYTE, pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    CHECK_GL_ERRORS();

    return reinterpret_cast<ntb::TextureHandle>(newTex);
}

void NtbRenderInterfaceCoreGL::destroyTexture(ntb::TextureHandle texture)
{
    if (texture == NULLPTR)
    {
        return;
    }

    GLTexture * texObj = reinterpret_cast<GLTexture *>(texture);
    delete texObj;
}

void NtbRenderInterfaceCoreGL::drawClipped2DTriangles(const ntb::VertexPTC * verts, int vertCount,
                                                      const ntb::UInt16 * indexes, int indexCount,
                                                      const ntb::DrawClippedInfo * drawInfo, int drawInfoCount, int frameMaxZ)
{
    assert(verts != NULLPTR);
    assert(indexes != NULLPTR);
    assert(drawInfo != NULLPTR);
    assert(vertCount > 0);
    assert(indexCount > 0);
    assert(drawInfoCount > 0);

    if (noDraw3D) { return; }

    glEnable(GL_SCISSOR_TEST);

    for (int i = 0; i < drawInfoCount; ++i)
    {
        const ntb::Rectangle viewport = drawInfo[i].viewport;
        const ntb::Rectangle clipBox  = drawInfo[i].clipBox;

        const int framebufferW = savedState.viewport[2] - savedState.viewport[0];
        const int framebufferH = savedState.viewport[3] - savedState.viewport[1];
        const int diffW = (framebufferW - windowWidth);
        const int diffH = (framebufferH - windowHeight);

        // 1024 * scale = 2048
        // y * scale = z
        //
        // scale = 2048/1024
        // scale = y/z

        int fbScaleX = 1;
        int fbScaleY = 1;
        if (diffW > 0) { fbScaleX = framebufferW / windowWidth;  }
        if (diffH > 0) { fbScaleY = framebufferH / windowHeight; }

        int viewportX = viewport.getX() * fbScaleX;
        int viewportY = viewport.getY() * fbScaleY;
        int viewportW = viewport.getWidth() * fbScaleX;
        int viewportH = viewport.getHeight() * fbScaleY;

        int clipX = clipBox.getX() * fbScaleX;
        int clipY = clipBox.getY() * fbScaleY;
        int clipW = clipBox.getWidth() * fbScaleX;
        int clipH = clipBox.getHeight() * fbScaleY;

        // Invert Y for OpenGL. In GL the origin of the
        // window/framebuffer is the bottom left corner,
        // and so is the origin of the viewport/scissor-box
        // (that's why the - viewportH part is also needed).
        viewportY = framebufferH - viewportY - viewportH;
        clipY = framebufferH - clipY - clipH;

        glViewport(viewportX, viewportY, viewportW, viewportH);
        glScissor(clipX, clipY, clipW, clipH);

        //FIXME: better to optimize for a single VBO update!
        draw2DTriangles(verts, vertCount,
                        &indexes[drawInfo[i].firstIndex], drawInfo[i].indexCount,
                        drawInfo[i].texture, frameMaxZ);
    }

    glDisable(GL_SCISSOR_TEST);
    glViewport(savedState.viewport[0], savedState.viewport[1],
               savedState.viewport[2], savedState.viewport[3]);
}

void NtbRenderInterfaceCoreGL::draw2DTriangles(const ntb::VertexPTC * verts, int vertCount,
                                               const ntb::UInt16 * indexes, int indexCount,
                                               ntb::TextureHandle texture, int frameMaxZ)
{
    assert(verts != NULLPTR);
    assert(indexes != NULLPTR);
    assert(vertCount > 0);
    assert(indexCount > 0);

    if (noDraw2D) { return; }

    //
    // Map to OpenGL NDC:
    // (TODO NOTE: this is probably more efficient if done in the shader!)
    //
    temp2dVerts.reserve(vertCount);
    for (int v = 0; v < vertCount; ++v)
    {
        const float x = toNormScreenX(verts[v].x, windowWidth);
        const float y = toNormScreenY(verts[v].y, windowHeight);
        const float z = ntb::remap(verts[v].z, 0.0f, static_cast<float>(frameMaxZ), -1.0f, +1.0f);

        const ntb::VertexPTC vertNdc = { x, y, z, verts[v].u, verts[v].v, verts[v].color };
        temp2dVerts.push_back(vertNdc);
    }

    //
    // Update the IBO/VBO and issue a draw call:
    //

    glBindBuffer(GL_ARRAY_BUFFER, tris2dVBO);
    glBufferData(GL_ARRAY_BUFFER, temp2dVerts.size() * sizeof(ntb::VertexPTC), &temp2dVerts[0], GL_DYNAMIC_DRAW);
    CHECK_GL_ERRORS();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tris2dIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(ntb::UInt16), indexes, GL_DYNAMIC_DRAW);
    CHECK_GL_ERRORS();

    // Positions:
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ntb::VertexPTC), offsetPtr(0));
    CHECK_GL_ERRORS();

    // Texture coordinate:
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ntb::VertexPTC), offsetPtr(sizeof(float) * 3));
    CHECK_GL_ERRORS();

    // Color:
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, GL_BGRA, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ntb::VertexPTC), offsetPtr(sizeof(float) * 5));
    CHECK_GL_ERRORS();

    glUseProgram(tris2dProgram);

    // Texture is optional.
    glActiveTexture(GL_TEXTURE0);
    if (texture != NULLPTR)
    {
        glBindTexture(GL_TEXTURE_2D, reinterpret_cast<GLTexture *>(texture)->textureId);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, reinterpret_cast<GLTexture *>(whiteTex)->textureId);
    }

    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, NULLPTR);
    CHECK_GL_ERRORS();

    temp2dVerts.clear();
}

void NtbRenderInterfaceCoreGL::draw2DLines(const ntb::VertexPC * verts, int vertCount, int frameMaxZ)
{
    assert(verts != NULLPTR);
    assert(vertCount > 0);

    if (noDraw2D) { return; }

    //
    // Map to OpenGL NDC:
    // (NOTE: this is probably more efficient if done in the shader!)
    //
    temp2dLines.reserve(vertCount);
    for (int v = 0; v < vertCount; ++v)
    {
        const float x = toNormScreenX(verts[v].x, windowWidth);
        const float y = toNormScreenY(verts[v].y, windowHeight);
        const float z = ntb::remap(verts[v].z, 0.0f, static_cast<float>(frameMaxZ), -1.0f, +1.0f);

        const ntb::VertexPC vertNdc = { x, y, z, verts[v].color };
        temp2dLines.push_back(vertNdc);
    }

    //
    // Update the VBO and issue a draw call:
    //
    assert(lines2dProgram != 0);
    assert(lines2dVBO != 0);

//    glDisable(GL_BLEND);

    glBindBuffer(GL_ARRAY_BUFFER, lines2dVBO);
    glBufferData(GL_ARRAY_BUFFER, temp2dLines.size() * sizeof(ntb::VertexPC), &temp2dLines[0], GL_DYNAMIC_DRAW);
    CHECK_GL_ERRORS();

    // Positions:
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ntb::VertexPC), offsetPtr(0));
    CHECK_GL_ERRORS();

    // Color:
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, GL_BGRA, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ntb::VertexPC), offsetPtr(sizeof(float) * 3));
    CHECK_GL_ERRORS();

    glUseProgram(lines2dProgram);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(temp2dLines.size()));
    CHECK_GL_ERRORS();

    temp2dLines.clear();
}

// ================================================================================================

static NtbRenderInterfaceCoreGL * renderInterface = NULLPTR;
static NtbShellInterfaceGLFW * shellInterface = NULLPTR;
static ntb::GUI   * gui1 = NULLPTR;
static ntb::Panel * pan1 = NULLPTR;
static ntb::Panel * pan2 = NULLPTR;
static ntb::Panel * pan3 = NULLPTR;

static void add_test_vars(ntb::Panel * panel)
{
    ntb::Variable * var;

    static bool b = true;
    var = panel->addBoolRW("bool var", &b);

    static int i = 1;
    var = panel->addNumberRW("int var", &i);

    static unsigned int u = 2;
    var = panel->addNumberRW(var, "uint var", &u);

    static void * p = (void*)0xDEADBEEF;
    var = panel->addPointerRO(var, "ptr var", &p);

    static float f = 3.141592f;
    var = panel->addNumberRW("float var", &f);

    static double d = 2.345;
    var = panel->addNumberRW(var, "double var", &d);

    static std::string s = "Hello!";
    var = panel->addStringRW("std::string var", &s);

    panel->addHierarchyParent("hierarchy parent");
}

//TODO NumPad keys will probably require additional translation!
//
using namespace ntb;
static bool capsLockMode = false;

static void asciiKeyCallback(GLFWwindow * /*window*/, const unsigned int key, const int mods)
{
    // Handled in the other callback because GLFW doesn't give us CONTROL modifiers in here.
    if ((key >= 'a' && key <= 'z') ||
        (key >= 'A' && key <= 'Z'))
    {
        return;
    }

    KeyCode keyCode;
    KeyModFlags keyModFlags = 0;

    if (mods & GLFW_MOD_SHIFT) { keyModFlags |= KeyModifiers::Shift; }
    if (mods & GLFW_MOD_SUPER) { keyModFlags |= KeyModifiers::Cmd;   }

    if (key > 0 && key <= 255)
    {
        keyCode = key;
    }
    else
    {
        keyCode = 0; // Unknown/null key; ignore.
    }

    gui1->onKeyPressed(keyCode, keyModFlags);
}

static void specialKeyCallback(GLFWwindow * /*window*/, const int key, int /*scanCode*/, const int action, const int mods)
{
    if (action == GLFW_RELEASE)
    {
        if (key == GLFW_KEY_CAPS_LOCK) { capsLockMode = false; }
        return;
    }

    KeyCode keyCode = 0;
    KeyModFlags keyModFlags = 0;

    if (mods & GLFW_MOD_SHIFT)   { keyModFlags |= KeyModifiers::Shift; }
    if (mods & GLFW_MOD_CONTROL) { keyModFlags |= KeyModifiers::Ctrl;  }
    if (mods & GLFW_MOD_SUPER)   { keyModFlags |= KeyModifiers::Cmd;   }

    // The alphabet keys are handled here because GLFW refuses to give us
    // a CONTROL modifier in the char callback above. The other punctuation
    // keys are handled there to properly deal with SHIFT modes.
    if (key >= 'A' && key <= 'Z')
    {
        // GLFW uses uppercase key codes by default.
        if (!(mods & GLFW_MOD_SHIFT) && !capsLockMode)
        {
            keyCode = static_cast<KeyCode>(std::tolower(key));
        }
        else
        {
            keyCode = static_cast<KeyCode>(key);
        }
    }
    else // Special non-ASCII keys:
    {
        switch (key)
        {
        case GLFW_KEY_ENTER     : keyCode = SpecialKeys::Return;     break;
        case GLFW_KEY_ESCAPE    : keyCode = SpecialKeys::Escape;     break;
        case GLFW_KEY_BACKSPACE : keyCode = SpecialKeys::Backspace;  break;
        case GLFW_KEY_DELETE    : keyCode = SpecialKeys::Delete;     break;
        case GLFW_KEY_TAB       : keyCode = SpecialKeys::Tab;        break;
        case GLFW_KEY_HOME      : keyCode = SpecialKeys::Home;       break;
        case GLFW_KEY_END       : keyCode = SpecialKeys::End;        break;
        case GLFW_KEY_PAGE_UP   : keyCode = SpecialKeys::PageUp;     break;
        case GLFW_KEY_PAGE_DOWN : keyCode = SpecialKeys::PageDown;   break;
        case GLFW_KEY_UP        : keyCode = SpecialKeys::UpArrow;    break;
        case GLFW_KEY_DOWN      : keyCode = SpecialKeys::DownArrow;  break;
        case GLFW_KEY_RIGHT     : keyCode = SpecialKeys::RightArrow; break;
        case GLFW_KEY_LEFT      : keyCode = SpecialKeys::LeftArrow;  break;
        case GLFW_KEY_INSERT    : keyCode = SpecialKeys::Insert;     break;
        case GLFW_KEY_F1        : keyCode = SpecialKeys::F1;         break;
        case GLFW_KEY_F2        : keyCode = SpecialKeys::F2;         break;
        case GLFW_KEY_F3        : keyCode = SpecialKeys::F3;         break;
        case GLFW_KEY_F4        : keyCode = SpecialKeys::F4;         break;
        case GLFW_KEY_F5        : keyCode = SpecialKeys::F5;         break;
        case GLFW_KEY_F6        : keyCode = SpecialKeys::F6;         break;
        case GLFW_KEY_F7        : keyCode = SpecialKeys::F7;         break;
        case GLFW_KEY_F8        : keyCode = SpecialKeys::F8;         break;
        case GLFW_KEY_F9        : keyCode = SpecialKeys::F9;         break;
        case GLFW_KEY_F10       : keyCode = SpecialKeys::F10;        break;
        case GLFW_KEY_F11       : keyCode = SpecialKeys::F11;        break;
        case GLFW_KEY_F12       : keyCode = SpecialKeys::F12;        break;
        case GLFW_KEY_CAPS_LOCK : capsLockMode = true;               break;
        default : break; // Unknown key; ignore.
        } // switch (key)
    }

    gui1->onKeyPressed(keyCode, keyModFlags);
}

static void mouseScrollCallback(GLFWwindow * /*window*/, double /*xOffset*/, const double yOffset)
{
    gui1->onMouseScroll(static_cast<int>(yOffset));
    /*
    if (yOffset < 0.0)
    {
        // Scroll forward
    }
    else
    {
        // Scroll back
    }
    */
}

static void mousePositionCallback(GLFWwindow * /*window*/, const double xPos, const double yPos)
{
    int mx = static_cast<int>(xPos);
    int my = static_cast<int>(yPos);

    // Clamp to window bounds:
    if      (mx > windowWidth)  { mx = windowWidth;  }
    else if (mx < 0)            { mx = 0;            }
    if      (my > windowHeight) { my = windowHeight; }
    else if (my < 0)            { my = 0;            }

    gui1->onMouseMotion(mx, my);
}

static void mouseButtonCallback(GLFWwindow * /*window*/, const int button, const int action, int /*mods*/)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        const  ntb::Int64 doubleClickTimeMs = 350; // Milliseconds between clicks for a double click
        static ntb::Int64 lastClickTimeMs   = 0;   // Milliseconds of last mouse click

        int clicks;
        if (action == GLFW_PRESS)
        {
            if ((getTimeMilliseconds() - lastClickTimeMs) <= doubleClickTimeMs)
            {
                clicks = 2;
                lastClickTimeMs = 0;
            }
            else
            {
                clicks = 1;
                lastClickTimeMs = getTimeMilliseconds();
            }
        }
        else // Button released.
        {
            clicks = -1;
        }

        if (clicks != -1) printf("clicks: %s\n", (clicks == 2 ? "DOUBLE" : "CLICK"));
        gui1->onMouseButton(ntb::MouseButton::Left, clicks);
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        const int clicks = (action == GLFW_PRESS) ? 1 : -1;
        gui1->onMouseButton(ntb::MouseButton::Right, clicks);
    }
}

static void sampleAppDraw()
{
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    // NTB starts writing at Z=0 and increases.
    glClearDepth(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gui1->onFrameRender();
}

static void sampleAppStart()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return;
    }

    // Things we need for the window / GL render context:
    glfwWindowHint(GLFW_RESIZABLE, false);
    glfwWindowHint(GLFW_DEPTH_BITS, 32);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    GLFWwindow * window = glfwCreateWindow(windowWidth, windowHeight,
                                           "NTB Sample - Core OpenGL",
                                           NULLPTR, NULLPTR);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        return;
    }

    glfwMakeContextCurrent(window);

    if (!gl3wInit())
    {
        std::cerr << "Failed to initialize GL3W extension library!" << std::endl;
        return;
    }

    if (!gl3wIsSupported(3, 2))
    {
        std::cerr << "This sample application requires at least OpenGL version 3.2 to run!" << std::endl;
        return;
    }

    // GLFW input callbacks:
    glfwSetCursorPosCallback(window,   &mousePositionCallback);
    glfwSetMouseButtonCallback(window, &mouseButtonCallback);
    glfwSetScrollCallback(window,      &mouseScrollCallback);
    glfwSetCharModsCallback(window,    &asciiKeyCallback);
    glfwSetKeyCallback(window,         &specialKeyCallback);

    renderInterface = new NtbRenderInterfaceCoreGL();
    shellInterface = new NtbShellInterfaceGLFW();

    ntb::initialize(renderInterface, shellInterface, NULLPTR);

    gui1 = ntb::createGUI("Gui 1");
    pan1 = gui1->createPanel("Pan 1");
//    pan2 = gui1->createPanel("Pan 2");
//    pan3 = gui1->createPanel("Pan 3");

    pan1->setSize(400,400);
//    pan2->setSize(100,100)->setPosition(300, 50);
//    pan3->setPosition(1024-500, 10);

    {
//        static ntb::UByte cbytes[] = { 0, 200, 200, 180 };
//        static ntb::Color32 clr = ntb::packColor(cbytes[0], cbytes[1], cbytes[2], cbytes[3]);
//        pan1->addColorRW("My color", &clr);

        static int foo = -42;
        pan1->addNumberRW("foo", &foo);

        static bool bar = false;
        pan1->addBoolRW("bar", &bar);

        static float pi = 3.141592f;
        Variable * var_pi = pan1->addNumberRW("PI", &pi);
        var_pi->setCustomTextColor(ntb::packColor(0,255,255));

        static void * ptr = (void *)0xDEADBEEF;
        pan1->addPointerRO("ptr", &ptr);

        static const float v4[] = { 1.1f, 2.2f, 3.3f, 4.4f };
        pan1->addFloatVecRO<4>("v4", v4);

        static std::string long_str = "testing 123";//"Some long string, testing 1234 - hello!";
        pan1->addStringRW("long_str", &long_str);

        pan1->printHierarchy();
    }

//    add_test_vars(pan1);
//    add_test_vars(pan2);
//    add_test_vars(pan3);

    // Loop until the user closes the window:
    while (!glfwWindowShouldClose(window))
    {
        sampleAppDraw();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ntb::shutdown();
}

void run_glfw_test_app()
{
    sampleAppStart();
    gl3wShutdown();
    glfwTerminate();
}
