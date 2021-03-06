
// ================================================================================================
// -*- C++ -*-
// File: ntb.hpp
// Author: Guilherme R. Lampert
// Created on: 25/04/16
// Brief: Neo Tweak Bar - A lightweight and intuitive C++ GUI library for graphics applications.
// ================================================================================================

#ifndef NTB_HPP
#define NTB_HPP

#include <cstddef>
#include <cstdint>
#include <type_traits>

// Overridable assert() macro for NTB.
#ifndef NTB_ASSERT
    #include <cassert>
    #define NTB_ASSERT assert
#endif // NTB_ASSERT

// ntb::Panel only accepts std::string if this switch is defined.
#if NEO_TWEAK_BAR_STD_STRING_INTEROP
    #include <string>
#endif // NEO_TWEAK_BAR_STD_STRING_INTEROP

namespace ntb
{

// ========================================================
// Auxiliary types and functions:
// ========================================================

// Forward declarations:
class Variable;
class Panel;
class GUI;

//
// Sized floating-point types:
//
using Float32 = float;
using Float64 = double;

//
// 32 bits ARGB color value.
//
using Color32 = std::uint32_t;

// Pack each byte into an integer Color32.
// 0x00-00-00-00
//   aa-rr-gg-bb
// Order will be ARGB, but APIs like OpenGL read it right-to-left as BGRA (GL_BGRA).
constexpr Color32 packColor(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b, const std::uint8_t a = 255)
{
    return static_cast<Color32>((a << 24) | (r << 16) | (g << 8) | b);
}

// Undo the work of packColor().
inline void unpackColor(const Color32 color, std::uint8_t & r, std::uint8_t & g, std::uint8_t & b, std::uint8_t & a)
{
    b = static_cast<std::uint8_t>((color & 0x000000FF) >> 0);
    g = static_cast<std::uint8_t>((color & 0x0000FF00) >> 8);
    r = static_cast<std::uint8_t>((color & 0x00FF0000) >> 16);
    a = static_cast<std::uint8_t>((color & 0xFF000000) >> 24);
}

// Set alpha channel of the color. Other channel values are not changed.
constexpr Color32 setAlphaChannel(const Color32 color, const std::uint8_t alpha)
{
    return ((alpha << 24) | (color & 0x00FFFFFF));
}

// Byte in [0,255] range to floating-point in [0,1] range.
// Used for color space conversions.
constexpr Float64 byteToFloat(const std::uint8_t b)
{
    return static_cast<Float64>(b) * (1.0 / 255.0);
}

// Float in [0,1] range to byte in [0,255] range.
// Used for color space conversions. Note that 'f' is not clamped!
constexpr std::uint8_t floatToByte(const Float64 f)
{
    return static_cast<std::uint8_t>(f * 255.0);
}

//
// Displayed numerical bases for number variables.
//
enum class NumberFormat : std::uint8_t
{
    Binary      = 2,
    Octal       = 8,
    Decimal     = 10,
    Hexadecimal = 16
};

//
// List of allowed constant values for enum variables.
//
struct EnumConstant final
{
    const char * const name;
    const std::int64_t value;

    EnumConstant(const char * n, const std::int64_t v)
        : name(n), value(v)
    { }

    template<typename EnumType>
    EnumConstant(const char * n, const EnumType v)
        : name(n), value(static_cast<std::int64_t>(v))
    { }
};

template<typename EnumType>
inline EnumConstant EnumTypeDecl()
{
    // Dummy constant with the size of a value of type EnumType.
    // This is not displayed in the UI.
    return EnumConstant("(enum size bytes)", sizeof(EnumType));
}

// Length in elements of a statically-declared C-style array.
template<typename T, int Length>
constexpr int lengthOfArray(const T (&)[Length])
{
    return Length;
}

// Remaps the value 'x' from one arbitrary min,max range to another.
template<typename T>
constexpr T remap(const T x, const T inMin, const T inMax, const T outMin, const T outMax)
{
    return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

// Clamp 'x' between min/max bounds.
template<typename T>
constexpr T clamp(const T x, const T minimum, const T maximum)
{
    return (x < minimum) ? minimum : (x > maximum) ? maximum : x;
}

// ========================================================
// Input helpers:
// ========================================================

enum class MouseButton
{
    Left,
    Right,
    Middle
};

struct SpecialKeys
{
    enum
    {
        // Zero is reserved as a flag for "no key pressed".
        Null = 0,

        // First 0-255 keys are reserved for the ASCII characters.
        Return = 256,
        Escape,
        Backspace,
        Delete,
        Tab,
        Home,
        End,
        PageUp,
        PageDown,
        UpArrow,
        DownArrow,
        RightArrow,
        LeftArrow,
        Insert,

        // These are not used and free for user-defined bindings.
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,

        // Sentinel value; Used internally.
        LastKey
    };
};

struct KeyModifiers
{
    static constexpr std::uint32_t Shift = 1 << 0;
    static constexpr std::uint32_t Ctrl  = 1 << 1;
    static constexpr std::uint32_t Cmd   = 1 << 2;
};

// KeyModifiers flags can be ORed together.
using KeyModFlags = std::uint32_t;

// Keyboard keys:
// - Lowercase ASCII keys + 0-9 and digits;
// - Special keys as the SpecialKeys enum.
using KeyCode = std::uint32_t;

// Debug printing helpers:
#if NEO_TWEAK_BAR_DEBUG
const char * mouseButtonToString(MouseButton button);
const char * keyCodeToString(KeyCode keyCode);
const char * keyModFlagsToString(KeyModFlags modifiers);
#endif // NEO_TWEAK_BAR_DEBUG

// ========================================================
// class ShellInterface:
// ========================================================

class ShellInterface
{
public:

    virtual ~ShellInterface();

    // Optional memory allocation callbacks.
    // If you choose not to override these to use a custom
    // allocator, the default implementation just forwards
    // to std::malloc() and std::free().
    virtual void * memAlloc(std::uint32_t sizeInBytes);
    virtual void memFree(void * ptrToFree);

    // Get the current time in milliseconds for things like
    // cursor animation and other UI effects. This method
    // is required and must be implemented.
    virtual std::int64_t getTimeMilliseconds() const = 0;
};

// ========================================================
// class RenderInterface and helpers:
// ========================================================

// Opaque handle to a texture type, implemented by the user.
struct OpaqueTextureType;
using TextureHandle = OpaqueTextureType *;

// Range of indexes to draw with custom clipbox and viewport.
struct DrawClippedInfo
{
    // Texture to apply. May be null for color-only drawing.
    TextureHandle texture;

    // Viewport box:
    int viewportX, viewportY;
    int viewportW, viewportH;

    // Box to clip (not necessary == viewport):
    int clipBoxX, clipBoxY;
    int clipBoxW, clipBoxH;

    int firstIndex; // First index to draw from the list passed to the function.
    int indexCount; // Number of indexes do draw from the list passed.
};

// Vertex with XYZ position, UV texture coords and ARGB(8:8:8:8) color.
struct VertexPTC
{
    Float32 x, y, z;
    Float32 u, v;
    Color32 color;
};

// Vertex with XYZ position and ARGB(8:8:8:8) color.
struct VertexPC
{
    Float32 x, y, z;
    Color32 color;
};

class RenderInterface
{
public:

    // Force the class to abstract with at least one pure virtual method.
    virtual ~RenderInterface() = 0;

    // Optional. Use these to save/restore render states before and after the NTB UI is drawn.
    virtual void beginDraw();
    virtual void endDraw();

    // Optional. Returns the maximum number of 2D layers.
    // Default value returned is = 999999.
    virtual int getMaxZ() const;

    // Optional. Returns the dimensions of the rendering viewport/window.
    // Defaults returned are = [0,0, 1024,768]
    virtual void getViewport(int * viewportX, int * viewportY,
                             int * viewportW, int * viewportH) const;

    // Optional. Creates a texture from the given array of pixels.
    // The data format is always bytes in the 0,255 color range. Number
    // of channels is either 1, 3 or 4 (grayscale, RGB, RGBA, respectively).
    // The default implementation returns a null handle that is ignored.
    virtual TextureHandle createTexture(int widthPixels, int heightPixels,
                                        int colorChannels, const void * pixels);

    // Optional. Deletes a texture previously created by createTexture().
    // If createTexture() didn't return null, then this method is called on GUI shutdown.
    virtual void destroyTexture(TextureHandle texture);

    // Optional. Draw a batch of unindexed 2D lines.
    // Vertexes are in screen-space, according to the size given by getViewport().
    virtual void draw2DLines(const VertexPC * verts, int vertCount, int frameMaxZ);

    // Optional. Draw a batch of indexed 2D triangles - Texture will be null for color only triangles.
    // Vertexes are in screen-space, according to the size given by getViewport().
    virtual void draw2DTriangles(const VertexPTC * verts, int vertCount,
                                 const std::uint16_t * indexes, int indexCount,
                                 TextureHandle texture, int frameMaxZ);

    // Similar to draw2DTriangles(), but takes an array of DrawClippedInfos to apply additional
    // clipping and custom viewports to the primitives. Each DrawClippedInfo corresponds to a
    // separate draw call.
    virtual void drawClipped2DTriangles(const VertexPTC * verts, int vertCount,
                                        const std::uint16_t * indexes, int indexCount,
                                        const DrawClippedInfo * drawInfo,
                                        int drawInfoCount, int frameMaxZ);

    // Creates a simple black-and-white checkerboard texture for debugging.
    TextureHandle createCheckerboardTexture(int widthPixels, int heightPixels, int squares);
};

// ========================================================
// Variable callbacks detail:
// ========================================================

namespace detail
{

template<typename T> struct StripPtrRefConst            { using Type = T; };
template<typename T> struct StripPtrRefConst<T *>       { using Type = T; };
template<typename T> struct StripPtrRefConst<T &>       { using Type = T; };
template<typename T> struct StripPtrRefConst<const T *> { using Type = T; };
template<typename T> struct StripPtrRefConst<const T &> { using Type = T; };

// These are needed for Panel::addPointer(). We must preserve the pointer qualifier
// for the special case of a var storing the raw pointer value of a void*.
template<> struct StripPtrRefConst<      void *> { using Type =       void *; };
template<> struct StripPtrRefConst<const void *> { using Type = const void *; };

// ========================================================
// class VarCallbacksInterface:
// ========================================================

class VarCallbacksInterface
{
public:

    virtual ~VarCallbacksInterface();
    virtual void callGetter(void * valueOut) const = 0;
    virtual void callSetter(const void * valueIn)  = 0;
    virtual VarCallbacksInterface * clone(void * where) const = 0;
};

// ========================================================
// class VarCallbacksMemFuncByValOrRef:
// ========================================================

template<typename OT, typename VT, typename RT = void>
class VarCallbacksMemFuncByValOrRef final
    : public VarCallbacksInterface
{
public:
    using ObjType = typename StripPtrRefConst<OT>::Type;
    using VarType = typename StripPtrRefConst<VT>::Type;
    using GetCBType = VT (OT::*)() const;
    using SetCBType = RT (OT::*)(VT);

    VarCallbacksMemFuncByValOrRef(const ObjType * o, GetCBType getCb, SetCBType setCb)
        : obj(const_cast<ObjType *>(o))
        , getter(getCb)
        , setter(setCb)
    { }
    void callGetter(void * valueOut) const override
    {
        NTB_ASSERT(obj    != nullptr);
        NTB_ASSERT(getter != nullptr);
        *static_cast<VarType *>(valueOut) = (obj->*getter)();
    }
    void callSetter(const void * valueIn) override
    {
        NTB_ASSERT(obj    != nullptr);
        NTB_ASSERT(setter != nullptr);
        (obj->*setter)(*static_cast<const VarType *>(valueIn));
    }
    VarCallbacksInterface * clone(void * where) const override
    {
        return ::new(where) VarCallbacksMemFuncByValOrRef<OT, VT, RT>(*this);
    }

private:
    ObjType * obj;
    GetCBType getter;
    SetCBType setter;
};

// ========================================================
// class VarCallbacksMemFuncByPointer:
// ========================================================

template<typename OT, typename VT>
class VarCallbacksMemFuncByPointer final
    : public VarCallbacksInterface
{
public:
    using ObjType = typename StripPtrRefConst<OT>::Type;
    using VarType = typename StripPtrRefConst<VT>::Type;
    using GetCBType = void (OT::*)(VarType *) const;
    using SetCBType = void (OT::*)(const VarType *);

    VarCallbacksMemFuncByPointer(const ObjType * o, GetCBType getCb, SetCBType setCb)
        : obj(const_cast<ObjType *>(o))
        , getter(getCb)
        , setter(setCb)
    { }
    void callGetter(void * valueOut) const override
    {
        NTB_ASSERT(obj    != nullptr);
        NTB_ASSERT(getter != nullptr);
        (obj->*getter)(static_cast<VarType *>(valueOut));
    }
    void callSetter(const void * valueIn) override
    {
        NTB_ASSERT(obj    != nullptr);
        NTB_ASSERT(setter != nullptr);
        (obj->*setter)(static_cast<const VarType *>(valueIn));
    }
    VarCallbacksInterface * clone(void * where) const override
    {
        return ::new(where) VarCallbacksMemFuncByPointer<OT, VT>(*this);
    }

private:
    ObjType * obj;
    GetCBType getter;
    SetCBType setter;
};

// ========================================================
// class VarCallbacksCFuncPtr:
// ========================================================

template<typename OT, typename VT>
class VarCallbacksCFuncPtr final
    : public VarCallbacksInterface
{
public:
    using ObjType = typename StripPtrRefConst<OT>::Type;
    using VarType = typename StripPtrRefConst<VT>::Type;
    using GetCBType = void (*)(const ObjType *, VarType *);
    using SetCBType = void (*)(ObjType *, const VarType *);

    VarCallbacksCFuncPtr(const ObjType * o, GetCBType getCb, SetCBType setCb)
        : obj(const_cast<ObjType *>(o))
        , getter(getCb)
        , setter(setCb)
    { }
    void callGetter(void * valueOut) const override
    {
        NTB_ASSERT(getter != nullptr);
        getter(obj, static_cast<VarType *>(valueOut));
    }
    void callSetter(const void * valueIn) override
    {
        NTB_ASSERT(setter != nullptr);
        setter(obj, static_cast<const VarType *>(valueIn));
    }
    VarCallbacksInterface * clone(void * where) const override
    {
        return ::new(where) VarCallbacksCFuncPtr<OT, VT>(*this);
    }

private:
    ObjType * obj;
    GetCBType getter;
    SetCBType setter;
};

} // namespace detail {}

// ========================================================
// class VarCallbacksAny:
// ========================================================

class VarCallbacksAny final
{
public:

    template<typename OT, typename VT, typename RT>
    VarCallbacksAny(const detail::VarCallbacksMemFuncByValOrRef<OT, VT, RT> & cbs)
    {
        static_assert(DataSizeBytes >= sizeof(cbs), "Size mismatch!");

        clear();
        callbacks = cbs.clone(getDataPtr());
    }

    template<typename OT, typename VT>
    VarCallbacksAny(const detail::VarCallbacksMemFuncByPointer<OT, VT> & cbs)
    {
        static_assert(DataSizeBytes >= sizeof(cbs), "Size mismatch!");

        clear();
        callbacks = cbs.clone(getDataPtr());
    }

    template<typename OT, typename VT>
    VarCallbacksAny(const detail::VarCallbacksCFuncPtr<OT, VT> & cbs)
    {
        static_assert(DataSizeBytes >= sizeof(cbs), "Size mismatch!");

        clear();
        callbacks = cbs.clone(getDataPtr());
    }

    VarCallbacksAny();
    VarCallbacksAny(const VarCallbacksAny & other);
    VarCallbacksAny & operator = (const VarCallbacksAny & other);

    void callGetter(void * valueOut) const;
    void callSetter(const void * valueIn);

    void clear();
    bool isNull() const;

private:

    // Inline storage for a VarCallbacks implementation:
    struct S;
    union Blob
    {
         Blob() { }
        ~Blob() { }

        std::max_align_t maxAlign;
        detail::VarCallbacksMemFuncByValOrRef<S, std::int64_t> dummy0;
        detail::VarCallbacksMemFuncByPointer<S, std::int64_t> dummy1;
        detail::VarCallbacksCFuncPtr<S, std::int64_t> dummy2;
    };

    // The pointer will hold our placement-new constructed callback impl,
    // the data blob is where we place it, so heap allocations are avoided.
    detail::VarCallbacksInterface * callbacks;
    Blob dataBlob;

    // We use the raw pointer to placement-new callbacks into it.
    void * getDataPtr() { return &dataBlob; }
    static constexpr std::size_t DataSizeBytes = sizeof(Blob);
};

// ========================================================
// Callbacks from member functions,
// dealing with references or values:
//
// VT OT::getCallback() const;
// RetType OT::setCallback(VT valueIn);
//
// VT can be a reference, const reference or value.
// ========================================================

template<typename OT, typename VT>
inline detail::VarCallbacksMemFuncByValOrRef<OT, VT> callbacks(const OT * obj, VT (OT::*getCb)() const)
{
    // The "member function by val or ref" callbacks
    // won't accept a pointer type, but the error
    // message generated is less than clear (it fails
    // inside the VarCallbacksMemFuncByValOrRef class).
    // If we are building for C++11 or above, this check
    // will provide a clean error message right away,
    // which should help the user find out the offending
    // method more easily. If C++11 is not available,
    // we'd still get a compiler error, but it might be a
    // little harder to figure out where is the offending method.
    static_assert(!std::is_pointer<VT>::value, "Variable cannot be a pointer for this type of callback!");
    return detail::VarCallbacksMemFuncByValOrRef<OT, VT>(obj, getCb, nullptr);
}

template<typename OT, typename VT, typename RT = void>
inline detail::VarCallbacksMemFuncByValOrRef<OT, VT, RT> callbacks(OT * obj, VT (OT::*getCb)() const, RT (OT::*setCb)(VT))
{
    // No pointers allowed here!
    // See the comment above in the other function.
    static_assert(!std::is_pointer<VT>::value, "Variable cannot be a pointer for this type of callbacks!");
    return detail::VarCallbacksMemFuncByValOrRef<OT, VT, RT>(obj, getCb, setCb);
}

// ========================================================
// Callbacks from member functions, dealing with pointers:
//
// void OT::getCallback(VT * valueOut) const;
// void OT::setCallback(const VT * valueIn);
//
// ========================================================

template<typename OT, typename VT>
inline detail::VarCallbacksMemFuncByPointer<OT, VT> callbacks(const OT * obj, void (OT::*getCb)(VT *) const)
{
    return detail::VarCallbacksMemFuncByPointer<OT, VT>(obj, getCb, nullptr);
}

template<typename OT, typename VT>
inline detail::VarCallbacksMemFuncByPointer<OT, VT> callbacks(OT * obj, void (OT::*getCb)(VT *) const, void (OT::*setCb)(const VT *))
{
    return detail::VarCallbacksMemFuncByPointer<OT, VT>(obj, getCb, setCb);
}

// ========================================================
// Callbacks from C-style function pointers:
//
// void getCallback(const OT * obj, VT * valueOut);
// void setCallback(OT * obj, const VT * valueIn);
//
// ========================================================

template<typename OT, typename VT>
inline detail::VarCallbacksCFuncPtr<OT, VT> callbacks(const OT * obj, void (*getCb)(const OT *, VT *))
{
    return detail::VarCallbacksCFuncPtr<OT, VT>(obj, getCb, nullptr);
}

template<typename OT, typename VT>
inline detail::VarCallbacksCFuncPtr<OT, VT> callbacks(OT * obj, void (*getCb)(const OT *, VT *), void (*setCb)(OT *, const VT *))
{
    return detail::VarCallbacksCFuncPtr<OT, VT>(obj, getCb, setCb);
}

// ========================================================
// struct ColorScheme:
// ========================================================

struct ColorScheme final
{
    // Box/rectangle styles:
    struct Box
    {
        // Box background:
        Color32 bgTopLeft;
        Color32 bgTopRight;
        Color32 bgBottomLeft;
        Color32 bgBottomRight;

        // Box outline:
        Color32 outlineTop;
        Color32 outlineBottom;
        Color32 outlineLeft;
        Color32 outlineRight;
    } box;

    // Shadow styles:
    struct Shadow
    {
        Color32 dark;
        Color32 light;
        int     offset;
    } shadow;

    // Text styles:
    struct Text
    {
        Color32 normal;
        Color32 alternate;
        Color32 informational;
        Color32 selection;
        Color32 cursor;
    } text;

    // List widget entries:
    struct ListItem
    {
        Color32 fillColorNormal;
        Color32 fillColorSelected;
        Color32 outlineColorNormal;
        Color32 outlineColorHovered;
    } listItem;

    // Check box buttons:
    Color32 checkMarkFill;
    Color32 checkBoxBorder;

    // Scroll-bar misc:
    Color32 scrollBarCenterLine1;
    Color32 scrollBarCenterLine2;

    // Outline of the 3D viewport on a View3DWidget:
    Color32 view3dOutline;

    // View3DWidget objects:
    Color32 view3dArrowObj;
    Color32 view3dBoxObj;

    // Resize handles in a window/panel, if it has any.
    Color32 resizeHandle;
};

// ========================================================
// class Variable:
// ========================================================

class Variable
{
public:

    virtual ~Variable();

    // Get name/title and name hash code:
    virtual const char * getName() const = 0;
    virtual std::uint32_t getHashCode() const = 0;

    // Get the GUI that owns the Panel that owns the variable:
    virtual const GUI * getGUI() const = 0;
    virtual GUI * getGUI() = 0;

    // Get the Panel that owns the variable:
    virtual const Panel * getPanel() const = 0;
    virtual Panel * getPanel() = 0;

    // Styling methods:
    virtual Variable * setName(const char * newName) = 0;
};

// Callback for Panel::enumerateAllVariables().
// First argument is the variable being iterated, second is the
// user context/data passed to the enumerate function. Returning
// true continues the enumeration, retuning false stops it.
using VariableEnumerateCallback = bool (*)(Variable *, void *);

// ========================================================
// class Panel:
// ========================================================

class Panel
{
public:

    virtual ~Panel();

    //
    // Boolean variables:
    //

    virtual Variable * addBoolRO(const char * name, const bool * var) = 0;
    virtual Variable * addBoolRO(Variable * parent, const char * name, const bool * var) = 0;
    virtual Variable * addBoolRW(const char * name, bool * var) = 0;
    virtual Variable * addBoolRW(Variable * parent, const char * name, bool * var) = 0;

    virtual Variable * addBoolRO(const char * name, const VarCallbacksAny & callbacks) = 0;
    virtual Variable * addBoolRO(Variable * parent, const char * name, const VarCallbacksAny & callbacks) = 0;
    virtual Variable * addBoolRW(const char * name, const VarCallbacksAny & callbacks) = 0;
    virtual Variable * addBoolRW(Variable * parent, const char * name, const VarCallbacksAny & callbacks) = 0;

    //
    // Single char variables:
    //

    virtual Variable * addCharRO(const char * name, const char * var) = 0;
    virtual Variable * addCharRO(Variable * parent, const char * name, const char * var) = 0;
    virtual Variable * addCharRW(const char * name, char * var) = 0;
    virtual Variable * addCharRW(Variable * parent, const char * name, char * var) = 0;

    virtual Variable * addCharRO(const char * name, const VarCallbacksAny & callbacks) = 0;
    virtual Variable * addCharRO(Variable * parent, const char * name, const VarCallbacksAny & callbacks) = 0;
    virtual Variable * addCharRW(const char * name, const VarCallbacksAny & callbacks) = 0;
    virtual Variable * addCharRW(Variable * parent, const char * name, const VarCallbacksAny & callbacks) = 0;

    //
    // Integer/float number variables:
    //

    virtual Variable * addNumberRO(const char * name, const std::int8_t * var) = 0;
    virtual Variable * addNumberRO(Variable * parent, const char * name, const std::int8_t * var) = 0;
    virtual Variable * addNumberRW(const char * name, std::int8_t * var) = 0;
    virtual Variable * addNumberRW(Variable * parent, const char * name, std::int8_t * var) = 0;

    virtual Variable * addNumberRO(const char * name, const std::uint8_t * var) = 0;
    virtual Variable * addNumberRO(Variable * parent, const char * name, const std::uint8_t * var) = 0;
    virtual Variable * addNumberRW(const char * name, std::uint8_t * var) = 0;
    virtual Variable * addNumberRW(Variable * parent, const char * name, std::uint8_t * var) = 0;

    virtual Variable * addNumberRO(const char * name, const std::int16_t * var) = 0;
    virtual Variable * addNumberRO(Variable * parent, const char * name, const std::int16_t * var) = 0;
    virtual Variable * addNumberRW(const char * name, std::int16_t * var) = 0;
    virtual Variable * addNumberRW(Variable * parent, const char * name, std::int16_t * var) = 0;

    virtual Variable * addNumberRO(const char * name, const std::uint16_t * var) = 0;
    virtual Variable * addNumberRO(Variable * parent, const char * name, const std::uint16_t * var) = 0;
    virtual Variable * addNumberRW(const char * name, std::uint16_t * var) = 0;
    virtual Variable * addNumberRW(Variable * parent, const char * name, std::uint16_t * var) = 0;

    virtual Variable * addNumberRO(const char * name, const std::int32_t * var) = 0;
    virtual Variable * addNumberRO(Variable * parent, const char * name, const std::int32_t * var) = 0;
    virtual Variable * addNumberRW(const char * name, std::int32_t * var) = 0;
    virtual Variable * addNumberRW(Variable * parent, const char * name, std::int32_t * var) = 0;

    virtual Variable * addNumberRO(const char * name, const std::uint32_t * var) = 0;
    virtual Variable * addNumberRO(Variable * parent, const char * name, const std::uint32_t * var) = 0;
    virtual Variable * addNumberRW(const char * name, std::uint32_t * var) = 0;
    virtual Variable * addNumberRW(Variable * parent, const char * name, std::uint32_t * var) = 0;

    virtual Variable * addNumberRO(const char * name, const std::int64_t * var) = 0;
    virtual Variable * addNumberRO(Variable * parent, const char * name, const std::int64_t * var) = 0;
    virtual Variable * addNumberRW(const char * name, std::int64_t * var) = 0;
    virtual Variable * addNumberRW(Variable * parent, const char * name, std::int64_t * var) = 0;

    virtual Variable * addNumberRO(const char * name, const std::uint64_t * var) = 0;
    virtual Variable * addNumberRO(Variable * parent, const char * name, const std::uint64_t * var) = 0;
    virtual Variable * addNumberRW(const char * name, std::uint64_t * var) = 0;
    virtual Variable * addNumberRW(Variable * parent, const char * name, std::uint64_t * var) = 0;

    virtual Variable * addNumberRO(const char * name, const Float32 * var) = 0;
    virtual Variable * addNumberRO(Variable * parent, const char * name, const Float32 * var) = 0;
    virtual Variable * addNumberRW(const char * name, Float32 * var) = 0;
    virtual Variable * addNumberRW(Variable * parent, const char * name, Float32 * var) = 0;

    virtual Variable * addNumberRO(const char * name, const Float64 * var) = 0;
    virtual Variable * addNumberRO(Variable * parent, const char * name, const Float64 * var) = 0;
    virtual Variable * addNumberRW(const char * name, Float64 * var) = 0;
    virtual Variable * addNumberRW(Variable * parent, const char * name, Float64 * var) = 0;

    virtual Variable * addNumberRO(const char * name, const VarCallbacksAny & callbacks) = 0;
    virtual Variable * addNumberRO(Variable * parent, const char * name, const VarCallbacksAny & callbacks) = 0;
    virtual Variable * addNumberRW(const char * name, const VarCallbacksAny & callbacks) = 0;
    virtual Variable * addNumberRW(Variable * parent, const char * name, const VarCallbacksAny & callbacks) = 0;

    //
    // Hexadecimal pointer value:
    //

    virtual Variable * addPointerRO(const char * name, void * const * ptr) = 0;
    virtual Variable * addPointerRO(Variable * parent, const char * name, void * const * ptr) = 0;
    virtual Variable * addPointerRW(const char * name, void ** ptr) = 0;
    virtual Variable * addPointerRW(Variable * parent, const char * name, void ** ptr) = 0;

    virtual Variable * addPointerRO(const char * name, const VarCallbacksAny & callbacks) = 0;
    virtual Variable * addPointerRO(Variable * parent, const char * name, const VarCallbacksAny & callbacks) = 0;
    virtual Variable * addPointerRW(const char * name, const VarCallbacksAny & callbacks) = 0;
    virtual Variable * addPointerRW(Variable * parent, const char * name, const VarCallbacksAny & callbacks) = 0;

    //
    // Generic float vectors (2,3,4 elements):
    //

    virtual Variable * addFloatVecRO(const char * name, const Float32 * vec, int size) = 0;
    virtual Variable * addFloatVecRO(Variable * parent, const char * name, const Float32 * vec, int size) = 0;
    virtual Variable * addFloatVecRW(const char * name, Float32 * vec, int size) = 0;
    virtual Variable * addFloatVecRW(Variable * parent, const char * name, Float32 * vec, int size) = 0;

    virtual Variable * addFloatVecRO(const char * name, const VarCallbacksAny & callbacks, int size) = 0;
    virtual Variable * addFloatVecRO(Variable * parent, const char * name, const VarCallbacksAny & callbacks, int size) = 0;
    virtual Variable * addFloatVecRW(const char * name, const VarCallbacksAny & callbacks, int size) = 0;
    virtual Variable * addFloatVecRW(Variable * parent, const char * name, const VarCallbacksAny & callbacks, int size) = 0;

    //
    // Direction vector (3 elements):
    //

    virtual Variable * addDirectionVecRO(const char * name, const Float32 * vec) = 0;
    virtual Variable * addDirectionVecRO(Variable * parent, const char * name, const Float32 * vec) = 0;
    virtual Variable * addDirectionVecRW(const char * name, Float32 * vec) = 0;
    virtual Variable * addDirectionVecRW(Variable * parent, const char * name, Float32 * vec) = 0;

    virtual Variable * addDirectionVecRO(const char * name, const VarCallbacksAny & callbacks) = 0;
    virtual Variable * addDirectionVecRO(Variable * parent, const char * name, const VarCallbacksAny & callbacks) = 0;
    virtual Variable * addDirectionVecRW(const char * name, const VarCallbacksAny & callbacks) = 0;
    virtual Variable * addDirectionVecRW(Variable * parent, const char * name, const VarCallbacksAny & callbacks) = 0;

    //
    // Rotation quaternions (4 elements):
    //

    virtual Variable * addRotationQuatRO(const char * name, const Float32 * quat) = 0;
    virtual Variable * addRotationQuatRO(Variable * parent, const char * name, const Float32 * quat) = 0;
    virtual Variable * addRotationQuatRW(const char * name, Float32 * quat) = 0;
    virtual Variable * addRotationQuatRW(Variable * parent, const char * name, Float32 * quat) = 0;

    virtual Variable * addRotationQuatRO(const char * name, const VarCallbacksAny & callbacks) = 0;
    virtual Variable * addRotationQuatRO(Variable * parent, const char * name, const VarCallbacksAny & callbacks) = 0;
    virtual Variable * addRotationQuatRW(const char * name, const VarCallbacksAny & callbacks) = 0;
    virtual Variable * addRotationQuatRW(Variable * parent, const char * name, const VarCallbacksAny & callbacks) = 0;

    //
    // Color values -- 3 or 4 components, range [0,255] or [0,1]:
    //

    // Byte-sized color components [0,255]:
    virtual Variable * addColorRO(const char * name, const std::uint8_t * color, int size) = 0;
    virtual Variable * addColorRO(Variable * parent, const char * name, const std::uint8_t * color, int size) = 0;
    virtual Variable * addColorRW(const char * name, std::uint8_t * color, int size) = 0;
    virtual Variable * addColorRW(Variable * parent, const char * name, std::uint8_t * color, int size) = 0;

    // Floating-point color components [0,1]:
    virtual Variable * addColorRO(const char * name, const Float32 * color, int size) = 0;
    virtual Variable * addColorRO(Variable * parent, const char * name, const Float32 * color, int size) = 0;
    virtual Variable * addColorRW(const char * name, Float32 * color, int size) = 0;
    virtual Variable * addColorRW(Variable * parent, const char * name, Float32 * color, int size) = 0;

    // Integer-packed 32 bits ARGB color:
    virtual Variable * addColorRO(const char * name, const Color32 * color) = 0;
    virtual Variable * addColorRO(Variable * parent, const char * name, const Color32 * color) = 0;
    virtual Variable * addColorRW(const char * name, Color32 * color) = 0;
    virtual Variable * addColorRW(Variable * parent, const char * name, Color32 * color) = 0;

    // Colors from callbacks:
    virtual Variable * addColorRO(const char * name, const VarCallbacksAny & callbacks, int size) = 0;
    virtual Variable * addColorRO(Variable * parent, const char * name, const VarCallbacksAny & callbacks, int size) = 0;
    virtual Variable * addColorRW(const char * name, const VarCallbacksAny & callbacks, int size) = 0;
    virtual Variable * addColorRW(Variable * parent, const char * name, const VarCallbacksAny & callbacks, int size) = 0;

    //
    // String variables:
    //

    // By pointer to char buffer (read-only):
    virtual Variable * addStringRO(const char * name, const char * str) = 0;
    virtual Variable * addStringRO(Variable * parent, const char * name, const char * str) = 0;

    // Pointer to fixed-size char buffer (read-write):
    virtual Variable * addStringRW(const char * name, char * buffer, int bufferSize) = 0;
    virtual Variable * addStringRW(Variable * parent, const char * name, char * buffer, int bufferSize) = 0;

    // By pointer to std::string (optional interface):
    #if NEO_TWEAK_BAR_STD_STRING_INTEROP
    virtual Variable * addStringRO(const char * name, const std::string * str) = 0;
    virtual Variable * addStringRO(Variable * parent, const char * name, const std::string * str) = 0;
    virtual Variable * addStringRW(const char * name, std::string * str) = 0;
    virtual Variable * addStringRW(Variable * parent, const char * name, std::string * str) = 0;
    #endif // NEO_TWEAK_BAR_STD_STRING_INTEROP

    // Strings from callbacks:
    virtual Variable * addStringRO(const char * name, const VarCallbacksAny & callbacks) = 0;
    virtual Variable * addStringRO(Variable * parent, const char * name, const VarCallbacksAny & callbacks) = 0;
    virtual Variable * addStringRW(const char * name, const VarCallbacksAny & callbacks) = 0;
    virtual Variable * addStringRW(Variable * parent, const char * name, const VarCallbacksAny & callbacks) = 0;

    //
    // User-defined enums (constants are not copied):
    //

    virtual Variable * addEnumRO(const char * name, const void * var, const EnumConstant * constants, int numOfConstants) = 0;
    virtual Variable * addEnumRO(Variable * parent, const char * name, const void * var, const EnumConstant * constants, int numOfConstants) = 0;
    virtual Variable * addEnumRW(const char * name, void * var, const EnumConstant * constants, int numOfConstants) = 0;
    virtual Variable * addEnumRW(Variable * parent, const char * name, void * var, const EnumConstant * constants, int numOfConstants) = 0;

    virtual Variable * addEnumRO(const char * name, const VarCallbacksAny & callbacks, const EnumConstant * constants, int numOfConstants) = 0;
    virtual Variable * addEnumRO(Variable * parent, const char * name, const VarCallbacksAny & callbacks, const EnumConstant * constants, int numOfConstants) = 0;
    virtual Variable * addEnumRW(const char * name, const VarCallbacksAny & callbacks, const EnumConstant * constants, int numOfConstants) = 0;
    virtual Variable * addEnumRW(Variable * parent, const char * name, const VarCallbacksAny & callbacks, const EnumConstant * constants, int numOfConstants) = 0;

    //
    // User-defined hierarchy parent. Can be used group variables:
    //

    virtual Variable * addHierarchyParent(const char * name) = 0;
    virtual Variable * addHierarchyParent(Variable * parent, const char * name) = 0;

    //
    // Panel/Variable management:
    //

    virtual Variable * findVariable(const char * varName) const = 0;
    virtual Variable * findVariable(std::uint32_t varNameHashCode) const = 0;
    virtual bool destroyVariable(Variable * variable) = 0;
    virtual void destroyAllVariables() = 0;
    virtual int getVariablesCount() const = 0;
    virtual void enumerateAllVariables(VariableEnumerateCallback enumCallback, void * userContext) = 0;

    // Miscellaneous accessors:
    virtual const char * getName() const = 0;
    virtual std::uint32_t getHashCode() const = 0;

    virtual const GUI * getGUI() const = 0;
    virtual GUI * getGUI() = 0;

    virtual int getPositionX() const = 0;
    virtual int getPositionY() const = 0;

    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;

    // Styling methods:
    virtual Panel * setName(const char * newName) = 0; // Also the window's title.
    virtual Panel * setPosition(int newPosX, int newPosY) = 0;
    virtual Panel * setSize(int newWidth, int newHeight) = 0;
};

// Callback for GUI::enumerateAllPanels().
// First argument is the Panel being iterated, second is the
// user context/data passed to the enumerate function. Returning
// true continues the enumeration, retuning false stops it.
using PanelEnumerateCallback = bool (*)(Panel *, void *);

// ========================================================
// class GUI:
// ========================================================

class GUI
{
public:

    virtual ~GUI();

    // Panel creation and management:
    virtual Panel * findPanel(const char * panelName) const = 0;
    virtual Panel * findPanel(std::uint32_t panelNameHashCode) const = 0;
    virtual Panel * createPanel(const char * panelName) = 0;
    virtual bool destroyPanel(Panel * panel) = 0;
    virtual void destroyAllPanels() = 0;
    virtual int getPanelCount() const = 0;
    virtual void enumerateAllPanels(PanelEnumerateCallback enumCallback, void * userContext) = 0;

    // Input events:
    virtual bool onKeyPressed(KeyCode key, KeyModFlags modifiers) = 0;
    virtual bool onMouseButton(MouseButton button, int clicks) = 0;
    virtual bool onMouseMotion(int mx, int my) = 0;
    virtual bool onMouseScroll(int yScroll) = 0; // +Y=forward, -Y=back

    // Draws the UI using the current RenderInterface.
    virtual void onFrameRender(bool forceRefresh = false) = 0;

    // Other UI control methods:
    virtual void minimizeAllPanels() = 0;
    virtual void maximizeAllPanels() = 0;
    virtual void hideAllPanels() = 0;
    virtual void showAllPanels() = 0;

    // UI and text scaling factors for this GUI and all of its Panels. Default is 1.0f.
    virtual void setGlobalUIScaling(Float32 scaling) = 0;
    virtual void setGlobalTextScaling(Float32 scaling) = 0;
    virtual Float32 getGlobalUIScaling() const = 0;
    virtual Float32 getGlobalTextScaling() const = 0;

    // Miscellaneous accessors:
    virtual const char * getName() const = 0;
    virtual std::uint32_t getHashCode() const = 0;
};

// Callback for ntb::enumerateAllGUIs().
// First argument is the GUI being iterated, second is the
// user context/data passed to the enumerate function. Returning
// true continues the enumeration, retuning false stops it.
using GUIEnumerateCallback = bool (*)(GUI *, void *);

// ========================================================
// Library initialization/shutdown and GUI allocation:
// ========================================================

// Initialize Neo Tweak Bar. You must call this function once
// before you can create any ntb::GUI instances. Call shutdown()
// when you are done with the library to automatically destroy
// all remaining GUI instances.
//
// A RenderInterface and a ShellInterface implementation must be
// provided. If you pass null pointers, initialization will fail.
bool initialize(ShellInterface * shell, RenderInterface * renderer);

// Performs global library shutdown.
// This will also destroy any remaining GUI instances
// that might still be active, so be sure to dispose
// any GUI pointers you might be holding on to after
// shutdown() is called.
void shutdown();

// Retrieve the user pointers set on initialization.
// These will assert if the library was never initialized.
ShellInterface  & getShellInterface();
RenderInterface & getRenderInterface();

// Find existing GUI by name. Returns null if not found.
// If more than one GUI with the same name exits, the
// first one found is returned.
GUI * findGUI(const char * guiName);
GUI * findGUI(std::uint32_t guiNameHashCode);

// Create a new GUI instance. Name doesn't have to be unique,
// but it must not be null nor an empty string.
GUI * createGUI(const char * guiName);

// Destroys the given GUI instance. GUIs don't have to be
// explicitly destroyed. Calling ntb::shutdown() will destroy
// all remaining GUIs.
bool destroyGUI(GUI * gui);

// Destroys all GUI instances. Any existing pointers to GUIs
// will be invalid when this function returns. Be careful!
// ntb::shutdown() implicitly calls this function.
void destroyAllGUIs();

// GUI enumeration/listing:
void enumerateAllGUIs(GUIEnumerateCallback enumCallback, void * userContext);
int getGUICount();

// ========================================================
// Library error handler:
// ========================================================

// First argument is the error message.
// Second is a user-defined pointer to data/context.
using ErrorHandlerCallback = void (*)(const char *, void *);

// Error printer used internally by the library.
// The output can be controlled via the error handler callback.
// This function can also be muted by calling 'silenceErrors(true)'.
// By default errors are enabled and printed to stderr.
#if defined(__GNUC__) || defined(__clang__)
    bool errorF(const char * fmt, ...) __attribute__((format(printf, 1, 2)));
#else // !GNU && !Clang
    bool errorF(const char * fmt, ...);
#endif // GNU || Clang

// Set/get the error handler that errorF() forwards its message to.
// The default handler just prints to stderr. To restore the default
// handler, pass null to the setErrorCallback() function.
void setErrorCallback(ErrorHandlerCallback errorHandler, void * userContext);
ErrorHandlerCallback getErrorCallback();

// Enable/disable error printing via errorF().
// Errors are initially enabled.
void silenceErrors(bool trueIfShouldSilence);

} // namespace ntb {}

#endif // NTB_HPP
