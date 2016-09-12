
// ================================================================================================
// -*- C++ -*-
// File: ntb.cpp
// Author: Guilherme R. Lampert
// Created on: 25/04/16
// Brief: Neo Tweak Bar - A lightweight and intuitive C++ GUI library for graphics applications.
// ================================================================================================

#include "ntb.hpp"
#include "ntb_utils.hpp"
#include "ntb_widgets.hpp"
#include <cstdarg> // for va_list & co

namespace ntb
{

// ========================================================
// Type size checking:
// ========================================================

#if NEO_TWEAK_BAR_CXX11_SUPPORTED
    static_assert(sizeof(Int8)    == 1, "Expected 8-bits  integer!");
    static_assert(sizeof(UInt8)   == 1, "Expected 8-bits  integer!");
    static_assert(sizeof(Int16)   == 2, "Expected 16-bits integer!");
    static_assert(sizeof(UInt16)  == 2, "Expected 16-bits integer!");
    static_assert(sizeof(Int32)   == 4, "Expected 32-bits integer!");
    static_assert(sizeof(UInt32)  == 4, "Expected 32-bits integer!");
    static_assert(sizeof(Int64)   == 8, "Expected 64-bits integer!");
    static_assert(sizeof(UInt64)  == 8, "Expected 64-bits integer!");
    static_assert(sizeof(Float32) == 4, "Expected 32-bits float!");
    static_assert(sizeof(Float64) == 8, "Expected 64-bits float!");
    static_assert(sizeof(void *)  == sizeof(std::size_t),    "Expected size_t to be the size of a pointer!");
    static_assert(sizeof(void *)  == sizeof(std::uintptr_t), "Expected uintptr_t to be the size of a pointer!");
    static_assert(sizeof(void *)  == sizeof(std::ptrdiff_t), "Expected ptrdiff_t to be the size of a pointer!");
#else // !C++11
    namespace {
    #define CT_CHECK_SIZE(type, size) typedef int static_assert_##type##_size[(sizeof(type) == (size)) ? 1 : -1]
    CT_CHECK_SIZE(Int8,    1);
    CT_CHECK_SIZE(UInt8,   1);
    CT_CHECK_SIZE(Int16,   2);
    CT_CHECK_SIZE(UInt16,  2);
    CT_CHECK_SIZE(Int32,   4);
    CT_CHECK_SIZE(UInt32,  4);
    CT_CHECK_SIZE(Int64,   8);
    CT_CHECK_SIZE(UInt64,  8);
    CT_CHECK_SIZE(Float32, 4);
    CT_CHECK_SIZE(Float64, 8);
    typedef void * void_pointer;
    CT_CHECK_SIZE(void_pointer, sizeof(std::size_t));
    CT_CHECK_SIZE(void_pointer, sizeof(std::uintptr_t));
    CT_CHECK_SIZE(void_pointer, sizeof(std::ptrdiff_t));
    #undef CT_CHECK_SIZE
    } // unnamed
#endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

// ========================================================
// Virtual destructors anchored to this file:
// ========================================================

ShellInterface::~ShellInterface()
{ }

RenderInterface::~RenderInterface()
{ }

Variable::~Variable()
{ }

Panel::~Panel()
{ }

GUI::~GUI()
{ }

namespace detail {
VarCallbacksInterface::~VarCallbacksInterface() { }
} // namespace detail

// ========================================================
// ShellInterface defaults:
// ========================================================

void * ShellInterface::memAlloc(UInt32 sizeInBytes)
{
    NTB_ASSERT(sizeInBytes != 0);
    return std::malloc(sizeInBytes);
}

void ShellInterface::memFree(void * ptrToFree)
{
    if (ptrToFree != NTB_NULL)
    {
        std::free(ptrToFree);
    }
}

// ========================================================
// RenderInterface defaults:
// ========================================================

void RenderInterface::beginDraw()
{
    // Nothing.
}

void RenderInterface::endDraw()
{
    // Nothing.
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

void RenderInterface::getViewport(int * viewportX, int * viewportY,
                                  int * viewportW, int * viewportH) const
{
    // A default arbitrary screen size.
    (*viewportX) = 0;
    (*viewportY) = 0;
    (*viewportW) = 1024;
    (*viewportH) = 768;
}

TextureHandle RenderInterface::createTexture(int, int, int, const void *)
{
    // No-op.
    return NTB_NULL;
}

void RenderInterface::destroyTexture(TextureHandle)
{
    // Nothing.
}

void RenderInterface::draw2DLines(const VertexPC *, int, int)
{
    // Nothing.
}

void RenderInterface::draw2DTriangles(const VertexPTC *, int, const UInt16 *, int, TextureHandle, int)
{
    // Nothing.
}

void RenderInterface::drawClipped2DTriangles(const VertexPTC *, int, const UInt16 *, int,
                                             const DrawClippedInfo *, int, int)
{
    // Nothing.
}

TextureHandle RenderInterface::createCheckerboardTexture(int widthPixels, int heightPixels, int squares)
{
    NTB_ASSERT(widthPixels  > 0);
    NTB_ASSERT(heightPixels > 0);
    NTB_ASSERT(squares      > 0);
    NTB_ASSERT((widthPixels  % squares) == 0);
    NTB_ASSERT((heightPixels % squares) == 0);

    const UInt8 colors[2][4] =
    {
        { 0,   0,   0,   255 },
        { 255, 255, 255, 255 }
    };
    const int checkerSize = widthPixels / squares; // Size of one checker square, in pixels.
    UInt8 * buffer = implAllocT<UInt8>(widthPixels * heightPixels * 4);

    for (int y = 0; y < heightPixels; ++y)
    {
        for (int x = 0; x < widthPixels; ++x)
        {
            const int colorIndex  = ((y / checkerSize) + (x / checkerSize)) % 2;
            const int bufferIndex = x + (y * widthPixels);

            for (int i = 0; i < 4; ++i)
            {
                buffer[(bufferIndex * 4) + i] = colors[colorIndex][i];
            }
        }
    }

    TextureHandle newTex = createTexture(widthPixels, heightPixels, 4, buffer);
    implFree(buffer);

    return newTex;
}

// ========================================================
// class VarCallbacksAny:
// ========================================================

VarCallbacksAny::VarCallbacksAny()
{
    clear(); // isNull() == true
}

VarCallbacksAny::VarCallbacksAny(const VarCallbacksAny & other)
{
    clear();
    NTB_ASSERT(other.callbacks != NTB_NULL);
    callbacks = other.callbacks->clone(getDataPtr());
}

VarCallbacksAny & VarCallbacksAny::operator = (const VarCallbacksAny & other)
{
    clear();
    NTB_ASSERT(other.callbacks != NTB_NULL);
    callbacks = other.callbacks->clone(getDataPtr());
    return *this;
}

void VarCallbacksAny::callGetter(void * valueOut) const
{
    NTB_ASSERT(callbacks != NTB_NULL);
    callbacks->callGetter(valueOut);
}

void VarCallbacksAny::callSetter(const void * valueIn)
{
    NTB_ASSERT(callbacks != NTB_NULL);
    callbacks->callSetter(valueIn);
}

void VarCallbacksAny::clear()
{
    // NOTE: Cutting a corner here. The correct would be to call
    // 'callbacks' destructor before setting it to null, but assuming
    // the implementation classes are simple types that allocate no
    // memory, we can ignore that and dodge the virtual destructor call.
    callbacks = NTB_NULL;
    std::memset(getDataPtr(), 0, DataSizeBytes);
}

bool VarCallbacksAny::isNull() const
{
    return callbacks == NTB_NULL;
}

// ========================================================
// Library initialization/shutdown and shared context:
// ========================================================

static ShellInterface  * g_pShellInterface  = NTB_NULL;
static RenderInterface * g_pRenderInterface = NTB_NULL;

bool initialize(ShellInterface * shell, RenderInterface * renderer)
{
    if (shell == NTB_NULL || renderer == NTB_NULL)
    {
        return errorF("ntb::initialize() requires a ShellInterface and a RenderInterface!");
    }

    g_pShellInterface  = shell;
    g_pRenderInterface = renderer;

    //TODO
}

void shutdown()
{
    //TODO

    g_pShellInterface  = NTB_NULL;
    g_pRenderInterface = NTB_NULL;
}

ShellInterface * getShellInterface()
{
    return g_pShellInterface;
}

RenderInterface * getRenderInterface()
{
    return g_pRenderInterface;
}

// ========================================================
// GUI management:
// ========================================================

GUI * findGUI(const char * guiName)
{
    //TODO
}

GUI * createGUI(const char * guiName)
{
    //TODO
}

bool destroyGUI(GUI * gui)
{
    //TODO
}

void destroyAllGUIs()
{
    //TODO
}

int getGUICount()
{
    //TODO
}

void enumerateAllGUIs(GUIEnumerateCallback enumCallback, void * userContext)
{
    //TODO
}

// ========================================================
// Library error handler:
// ========================================================

static void defaultErrorHandlerCb(const char * const message, void * /* userContext */)
{
    std::fprintf(stderr, "[NTB_ERROR]: %s\n", message);
}

static ErrorHandlerCallback g_errorHandler = &defaultErrorHandlerCb;
static void *               g_errorUserCtx = NTB_NULL;
static bool                 g_silentErrors = false;

bool errorF(const char * const fmt, ...)
{
    if (g_silentErrors || fmt == NTB_NULL)
    {
        return false;
    }

    va_list vaList;
    char tempStr[2048];

    va_start(vaList, fmt);
    const int result = std::vsnprintf(tempStr, lengthOfArray(tempStr), fmt, vaList);
    va_end(vaList);

    if (result > 0)
    {
        g_errorHandler(tempStr, g_errorUserCtx);
    }

    // Always returns false, so we can write "return errorF(...);"
    return false;
}

void setErrorCallback(ErrorHandlerCallback errorHandler, void * userContext)
{
    if (errorHandler == NTB_NULL) // Use null to restore the default.
    {
        g_errorHandler = &defaultErrorHandlerCb;
        g_errorUserCtx = NTB_NULL;
    }
    else
    {
        g_errorHandler = errorHandler;
        g_errorUserCtx = userContext;
    }
}

ErrorHandlerCallback getErrorCallback()
{
    return g_errorHandler;
}

void silenceErrors(const bool trueIfShouldSilence)
{
    g_silentErrors = trueIfShouldSilence;
}

// ========================================================
// Enum to string helpers:
// ========================================================

const char * MouseButton::toString(const MouseButton::Enum button)
{
    static char str[16];
    switch (button)
    {
    case MouseButton::Left   : copyString(str, sizeof(str), "Left");    break;
    case MouseButton::Right  : copyString(str, sizeof(str), "Right");   break;
    case MouseButton::Middle : copyString(str, sizeof(str), "Middle");  break;
    default                  : copyString(str, sizeof(str), "Unknown"); break;
    } // switch (button)
    return str;
}

const char * SpecialKeys::toString(const KeyCode keyCode)
{
    static char str[16];
    if (keyCode > 0 && keyCode < 256) // ASCII keys
    {
        str[0] = static_cast<char>(keyCode);
        str[1] = '\0';
    }
    else
    {
        switch (keyCode)
        {
        case SpecialKeys::Null       : copyString(str, sizeof(str), "No key");     break;
        case SpecialKeys::Return     : copyString(str, sizeof(str), "Return");     break;
        case SpecialKeys::Escape     : copyString(str, sizeof(str), "Escape");     break;
        case SpecialKeys::Backspace  : copyString(str, sizeof(str), "Backspace");  break;
        case SpecialKeys::Delete     : copyString(str, sizeof(str), "Delete");     break;
        case SpecialKeys::Tab        : copyString(str, sizeof(str), "Tab");        break;
        case SpecialKeys::Home       : copyString(str, sizeof(str), "Home");       break;
        case SpecialKeys::End        : copyString(str, sizeof(str), "End");        break;
        case SpecialKeys::PageUp     : copyString(str, sizeof(str), "PageUp");     break;
        case SpecialKeys::PageDown   : copyString(str, sizeof(str), "PageDown");   break;
        case SpecialKeys::UpArrow    : copyString(str, sizeof(str), "UpArrow");    break;
        case SpecialKeys::DownArrow  : copyString(str, sizeof(str), "DownArrow");  break;
        case SpecialKeys::RightArrow : copyString(str, sizeof(str), "RightArrow"); break;
        case SpecialKeys::LeftArrow  : copyString(str, sizeof(str), "LeftArrow");  break;
        case SpecialKeys::Insert     : copyString(str, sizeof(str), "Insert");     break;
        case SpecialKeys::F1         : copyString(str, sizeof(str), "F1");         break;
        case SpecialKeys::F2         : copyString(str, sizeof(str), "F2");         break;
        case SpecialKeys::F3         : copyString(str, sizeof(str), "F3");         break;
        case SpecialKeys::F4         : copyString(str, sizeof(str), "F4");         break;
        case SpecialKeys::F5         : copyString(str, sizeof(str), "F5");         break;
        case SpecialKeys::F6         : copyString(str, sizeof(str), "F6");         break;
        case SpecialKeys::F7         : copyString(str, sizeof(str), "F7");         break;
        case SpecialKeys::F8         : copyString(str, sizeof(str), "F8");         break;
        case SpecialKeys::F9         : copyString(str, sizeof(str), "F9");         break;
        case SpecialKeys::F10        : copyString(str, sizeof(str), "F10");        break;
        case SpecialKeys::F11        : copyString(str, sizeof(str), "F11");        break;
        case SpecialKeys::F12        : copyString(str, sizeof(str), "F12");        break;
        default                      : copyString(str, sizeof(str), "Unknown");    break;
        } // switch (keyCode)
    }
    return str;
}

const char * KeyModifiers::toString(const KeyModFlags modifiers)
{
    static char str[24];
    char * ptr  = str;
    int    left = lengthOfArray(str);
    int    n    = 0;

    if (modifiers & KeyModifiers::Shift)
    {
        n = copyString(ptr, left, "Shift ");
        ptr += n; left -= n;
    }
    if (modifiers & KeyModifiers::Ctrl)
    {
        n = copyString(ptr, left, "Ctrl ");
        ptr += n; left -= n;
    }
    if (modifiers & KeyModifiers::Cmd)
    {
        n = copyString(ptr, left, "Cmd ");
        ptr += n; left -= n;
    }

    return str;
}

} // namespace ntb {}
