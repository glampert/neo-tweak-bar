
// ================================================================================================
// -*- C++ -*-
// File: ntb.cpp
// Author: Guilherme R. Lampert
// Created on: 25/04/16
// Brief: Neo Tweak Bar - A lightweight and intuitive C++ GUI library for graphics applications.
// ================================================================================================

#include "ntb_utils.hpp"
#include "ntb_widgets.hpp"
#include <cstdarg> // for va_list & co

namespace ntb
{

// ========================================================
// Type size checking:
// ========================================================

static_assert(sizeof(Float32) == 4, "Expected 32-bits float!");
static_assert(sizeof(Float64) == 8, "Expected 64-bits float!");
static_assert(sizeof(void *)  == sizeof(std::size_t),    "Expected size_t to be the size of a pointer!");
static_assert(sizeof(void *)  == sizeof(std::uintptr_t), "Expected uintptr_t to be the size of a pointer!");
static_assert(sizeof(void *)  == sizeof(std::ptrdiff_t), "Expected ptrdiff_t to be the size of a pointer!");

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

namespace detail
{
VarCallbacksInterface::~VarCallbacksInterface() { }
} // namespace detail

// ========================================================
// ShellInterface defaults:
// ========================================================

void * ShellInterface::memAlloc(std::uint32_t sizeInBytes)
{
    NTB_ASSERT(sizeInBytes != 0);
    return std::malloc(sizeInBytes);
}

void ShellInterface::memFree(void * ptrToFree)
{
    if (ptrToFree != nullptr)
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
    return nullptr;
}

void RenderInterface::destroyTexture(TextureHandle)
{
    // Nothing.
}

void RenderInterface::draw2DLines(const VertexPC *, int, int)
{
    // Nothing.
}

void RenderInterface::draw2DTriangles(const VertexPTC *, int, const std::uint16_t *, int,
                                      TextureHandle, int)
{
    // Nothing.
}

void RenderInterface::drawClipped2DTriangles(const VertexPTC *, int, const std::uint16_t *, int,
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

    const std::uint8_t colors[2][4] =
    {
        { 0,   0,   0,   255 },
        { 255, 255, 255, 255 }
    };
    const int checkerSize = widthPixels / squares; // Size of one checker square, in pixels.
    std::uint8_t * buffer = implAllocT<std::uint8_t>(widthPixels * heightPixels * 4);

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
    NTB_ASSERT(other.callbacks != nullptr);
    callbacks = other.callbacks->clone(getDataPtr());
}

VarCallbacksAny & VarCallbacksAny::operator = (const VarCallbacksAny & other)
{
    clear();
    NTB_ASSERT(other.callbacks != nullptr);
    callbacks = other.callbacks->clone(getDataPtr());
    return *this;
}

void VarCallbacksAny::callGetter(void * valueOut) const
{
    NTB_ASSERT(callbacks != nullptr);
    callbacks->callGetter(valueOut);
}

void VarCallbacksAny::callSetter(const void * valueIn)
{
    NTB_ASSERT(callbacks != nullptr);
    callbacks->callSetter(valueIn);
}

void VarCallbacksAny::clear()
{
    // NOTE: Cutting a corner here. The correct would be to call
    // 'callbacks' destructor before setting it to null, but assuming
    // the implementation classes are simple types that allocate no
    // memory, we can ignore that and dodge the virtual destructor call.
    callbacks = nullptr;
    std::memset(getDataPtr(), 0, DataSizeBytes);
}

bool VarCallbacksAny::isNull() const
{
    return callbacks == nullptr;
}

// ========================================================
// Library initialization/shutdown and shared context:
// ========================================================

static ShellInterface  * g_pShellInterface  = nullptr;
static RenderInterface * g_pRenderInterface = nullptr;

bool initialize(ShellInterface * shell, RenderInterface * renderer)
{
    if (shell == nullptr || renderer == nullptr)
    {
        return errorF("ntb::initialize() requires a ShellInterface and a RenderInterface!");
    }

    g_pShellInterface  = shell;
    g_pRenderInterface = renderer;

    //TODO
    return true;
}

void shutdown()
{
    //TODO

    g_pShellInterface  = nullptr;
    g_pRenderInterface = nullptr;
}

ShellInterface & getShellInterface()
{
    NTB_ASSERT(g_pShellInterface != nullptr);
    return *g_pShellInterface;
}

RenderInterface & getRenderInterface()
{
    NTB_ASSERT(g_pRenderInterface != nullptr);
    return *g_pRenderInterface;
}

// ========================================================
// GUI management:
// ========================================================

GUI * findGUI(const char * guiName)
{
    //TODO
    (void)guiName;
    return nullptr;
}

GUI * findGUI(std::uint32_t guiNameHashCode)
{
    //TODO
    (void)guiNameHashCode;
    return nullptr;
}

GUI * createGUI(const char * guiName)
{
    //TODO
    (void)guiName;
    static char fakeGUI[sizeof(GUI)];
    return (GUI*)&fakeGUI;
}

bool destroyGUI(GUI * gui)
{
    //TODO
    (void)gui;
    return false;
}

void destroyAllGUIs()
{
    //TODO
}

int getGUICount()
{
    //TODO
    return 0;
}

void enumerateAllGUIs(GUIEnumerateCallback enumCallback, void * userContext)
{
    //TODO
    (void)enumCallback;
    (void)userContext;
}

// ========================================================
// Library error handler:
// ========================================================

static void defaultErrorHandlerCb(const char * const message, void * /* userContext */)
{
    std::fprintf(stderr, "[NTB_ERROR]: %s\n", message);
}

static ErrorHandlerCallback g_errorHandler = &defaultErrorHandlerCb;
static void *               g_errorUserCtx = nullptr;
static bool                 g_silentErrors = false;

bool errorF(const char * const fmt, ...)
{
    if (g_silentErrors || fmt == nullptr)
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
    if (errorHandler == nullptr) // Use null to restore the default.
    {
        g_errorHandler = &defaultErrorHandlerCb;
        g_errorUserCtx = nullptr;
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
// Enum to string debugging helpers:
// ========================================================

#if NEO_TWEAK_BAR_DEBUG

const char * mouseButtonToString(const MouseButton button)
{
    static char str[16];
    switch (button)
    {
    case MouseButton::Left   : copyString(str, "Left");    break;
    case MouseButton::Right  : copyString(str, "Right");   break;
    case MouseButton::Middle : copyString(str, "Middle");  break;
    default                  : copyString(str, "Unknown"); break;
    } // switch (button)
    return str;
}

const char * keyCodeToString(const KeyCode keyCode)
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
        case SpecialKeys::Null       : copyString(str, "No key");     break;
        case SpecialKeys::Return     : copyString(str, "Return");     break;
        case SpecialKeys::Escape     : copyString(str, "Escape");     break;
        case SpecialKeys::Backspace  : copyString(str, "Backspace");  break;
        case SpecialKeys::Delete     : copyString(str, "Delete");     break;
        case SpecialKeys::Tab        : copyString(str, "Tab");        break;
        case SpecialKeys::Home       : copyString(str, "Home");       break;
        case SpecialKeys::End        : copyString(str, "End");        break;
        case SpecialKeys::PageUp     : copyString(str, "PageUp");     break;
        case SpecialKeys::PageDown   : copyString(str, "PageDown");   break;
        case SpecialKeys::UpArrow    : copyString(str, "UpArrow");    break;
        case SpecialKeys::DownArrow  : copyString(str, "DownArrow");  break;
        case SpecialKeys::RightArrow : copyString(str, "RightArrow"); break;
        case SpecialKeys::LeftArrow  : copyString(str, "LeftArrow");  break;
        case SpecialKeys::Insert     : copyString(str, "Insert");     break;
        case SpecialKeys::F1         : copyString(str, "F1");         break;
        case SpecialKeys::F2         : copyString(str, "F2");         break;
        case SpecialKeys::F3         : copyString(str, "F3");         break;
        case SpecialKeys::F4         : copyString(str, "F4");         break;
        case SpecialKeys::F5         : copyString(str, "F5");         break;
        case SpecialKeys::F6         : copyString(str, "F6");         break;
        case SpecialKeys::F7         : copyString(str, "F7");         break;
        case SpecialKeys::F8         : copyString(str, "F8");         break;
        case SpecialKeys::F9         : copyString(str, "F9");         break;
        case SpecialKeys::F10        : copyString(str, "F10");        break;
        case SpecialKeys::F11        : copyString(str, "F11");        break;
        case SpecialKeys::F12        : copyString(str, "F12");        break;
        default                      : copyString(str, "Unknown");    break;
        } // switch (keyCode)
    }
    return str;
}

const char * keyModFlagsToString(const KeyModFlags modifiers)
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

#endif // NEO_TWEAK_BAR_DEBUG

} // namespace ntb {}
