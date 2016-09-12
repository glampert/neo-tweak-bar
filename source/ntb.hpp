
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

//
// If the user didn't specify if C++11 or above is supported, try to guess
// from the value of '__cplusplus'. It should be 199711L for pre-C++11 compilers
// and 201103L in those supporting C++11, but this is not a guarantee that all
// C++11 features will be available and stable, so again, we are making a guess.
// It is recommended to instead supply the NEO_TWEAK_BAR_CXX11_SUPPORTED switch
// yourself while building the library.
//
#ifndef NEO_TWEAK_BAR_CXX11_SUPPORTED
    #if (__cplusplus > 199711L)
        #define NEO_TWEAK_BAR_CXX11_SUPPORTED 1
    #endif // __cplusplus
#endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

//
// Overridable assert() macro for NTB.
//
#ifndef NTB_ASSERT
    #include <cassert>
    #define NTB_ASSERT assert
#endif // NTB_ASSERT

//
// cstdint/stdint.h and type_traits are only standard starting from C++11.
//
#if NEO_TWEAK_BAR_CXX11_SUPPORTED
    #include <cstdint>
    #include <type_traits>
#endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

//
// ntb::Panel only accepts std::string if this switch is defined.
//
#if NEO_TWEAK_BAR_STD_STRING_INTEROP
    #include <string>
#endif // NEO_TWEAK_BAR_STD_STRING_INTEROP

//
// C++11 => C++98/03 compatibility.
//
#if NEO_TWEAK_BAR_CXX11_SUPPORTED
    #define NTB_NULL        nullptr
    #define NTB_OVERRIDE    override
    #define NTB_FINAL_CLASS final
    #define NTB_DISABLE_COPY_ASSIGN(className) \
        private:                               \
        className(const className &) = delete; \
        className & operator = (const className &) = delete
#else // !C++11
    #define NTB_NULL        NULL
    #define NTB_OVERRIDE    /* nothing */
    #define NTB_FINAL_CLASS /* nothing */
    #define NTB_DISABLE_COPY_ASSIGN(className) \
        private:                               \
        className(const className &);          \
        className & operator = (const className &)
#endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

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
// Integer/floating-point sized types:
//
#if NEO_TWEAK_BAR_CXX11_SUPPORTED
    typedef std::int8_t        Int8;
    typedef std::uint8_t       UInt8;
    typedef std::int16_t       Int16;
    typedef std::uint16_t      UInt16;
    typedef std::int32_t       Int32;
    typedef std::uint32_t      UInt32;
    typedef std::int64_t       Int64;
    typedef std::uint64_t      UInt64;
    typedef float              Float32;
    typedef double             Float64;
#else // !C++11
    typedef signed char        Int8;
    typedef unsigned char      UInt8;
    typedef signed short       Int16;
    typedef unsigned short     UInt16;
    typedef signed int         Int32;
    typedef unsigned int       UInt32;
    typedef signed long long   Int64;
    typedef unsigned long long UInt64;
    typedef float              Float32;
    typedef double             Float64;
#endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

//
// 32 bits ARGB color value.
//
typedef UInt32 Color32;

// Pack each byte into an integer Color32.
// 0x00-00-00-00
//   aa-rr-gg-bb
// Order will be ARGB, but APIs like OpenGL read it right-to-left as BGRA (GL_BGRA).
inline Color32 packColor(const UInt8 r, const UInt8 g, const UInt8 b, const UInt8 a = 255)
{
    return static_cast<Color32>((a << 24) | (r << 16) | (g << 8) | b);
}

// Undo the work of packColor().
inline void unpackColor(const Color32 color, UInt8 & r, UInt8 & g, UInt8 & b, UInt8 & a)
{
    b = static_cast<UInt8>((color & 0x000000FF) >> 0);
    g = static_cast<UInt8>((color & 0x0000FF00) >> 8);
    r = static_cast<UInt8>((color & 0x00FF0000) >> 16);
    a = static_cast<UInt8>((color & 0xFF000000) >> 24);
}

// Byte in [0,255] range to Float32 in [0,1] range.
// Used for color space conversions.
inline Float32 byteToFloat(const UInt8 b)
{
    return static_cast<Float32>(b) * (1.0f / 255.0f);
}

// Float in [0,1] range to byte in [0,255] range.
// Used for color space conversions. Note that 'f' is not clamped!
inline UInt8 floatToByte(const Float32 f)
{
    return static_cast<UInt8>(f * 255.0f);
}

//
// Displayed numerical bases for number variables.
//
struct NumberFormat
{
    enum
    {
        Binary      = 2,
        Octal       = 8,
        Decimal     = 10,
        Hexadecimal = 16
    };
    typedef Int8 Enum;
};

//
// List of allowed constant values for enum variables.
//
struct EnumConstant NTB_FINAL_CLASS
{
    const char * name;
    const Int64  value;

    EnumConstant(const char * n, const Int64 v)
        : name(n), value(v)
    { }

    template<typename EnumType>
    EnumConstant(const char * n, const EnumType v)
        : name(n), value(static_cast<Int64>(v))
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
inline int lengthOfArray(const T (&)[Length])
{
    return Length;
}

// Remaps the value 'x' from one arbitrary min,max range to another.
template<typename T>
inline T remap(const T x, const T inMin, const T inMax, const T outMin, const T outMax)
{
    return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

// Clamp 'x' between min/max bounds.
template<typename T>
inline T clamp(const T x, const T minimum, const T maximum)
{
    return (x < minimum) ? minimum : (x > maximum) ? maximum : x;
}

// ========================================================
// Input helpers:
// ========================================================

struct MouseButton
{
    enum
    {
        Left,
        Right,
        Middle
    };
    typedef Int8 Enum;
    static const char * toString(MouseButton::Enum button);
};

// Following KeyModifiers can be ORed together.
typedef UInt32 KeyModFlags;

struct KeyModifiers
{
    enum
    {
        Shift = 1 << 0,
        Ctrl  = 1 << 1,
        Cmd   = 1 << 2,
    };
    static const char * toString(KeyModFlags modifiers);
};

// Keyboard keys:
// - Lowercase ASCII keys + 0-9 and digits;
// - Special keys as the following enum.
typedef UInt32 KeyCode;

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
    static const char * toString(KeyCode keyCode);
};

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
    virtual void * memAlloc(UInt32 sizeInBytes);
    virtual void memFree(void * ptrToFree);

    // Get the current time in milliseconds for things like
    // cursor animation and other UI effects. This methods
    // is required and must be implemented.
    virtual Int64 getTimeMilliseconds() const = 0;
};

// ========================================================
// class RenderInterface and helpers:
// ========================================================

// Opaque handle to a texture type, implemented by the user.
struct OpaqueTextureType;
typedef OpaqueTextureType * TextureHandle;

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
                                 const UInt16 * indexes, int indexCount,
                                 TextureHandle texture, int frameMaxZ);

    // Similar to draw2DTriangles(), but takes an array of DrawClippedInfos to apply additional
    // clipping and custom viewports to the primitives. Each DrawClippedInfo corresponds to a
    // separate draw call.
    virtual void drawClipped2DTriangles(const VertexPTC * verts, int vertCount,
                                        const UInt16 * indexes, int indexCount,
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

// This is based on a similar trick presented in the book
// "Modern C++ Design: Generic Programming and Design Patterns"
// to avoid relying on the alignas() operator, only available from
// C++11. The union is a mix of types with different alignments
// and sizes, so it will select the largest alignment required.
template<typename T, typename U = T, typename V = T>
struct MaxAlign
{
    struct DummyS;
    typedef void (DummyS::*MemFunc)();
    typedef int  (*CFunc)(DummyS *);
    union Blob
    {
        UInt8    userDataT[sizeof(T)];
        UInt8    userDataU[sizeof(U)];
        UInt8    userDataV[sizeof(V)];
        DummyS * dummy0;
        CFunc    dummy1;
        MemFunc  dummy2;
        char     dummy3;
        short    dummy4;
        int      dummy5;
        long     dummy6;
        float    dummy7;
        double   dummy8;
        bool     dummy9;
        void *   dummy10;
        Int64    dummy11;
    } blob;
};

template<typename T> struct StripPtrRefConst            { typedef T Type; };
template<typename T> struct StripPtrRefConst<T *>       { typedef T Type; };
template<typename T> struct StripPtrRefConst<T &>       { typedef T Type; };
template<typename T> struct StripPtrRefConst<const T *> { typedef T Type; };
template<typename T> struct StripPtrRefConst<const T &> { typedef T Type; };

// These are needed for Panel::addPointer(). We must preserve the pointer qualifier
// for the special case of a var storing the raw pointer value of a void*.
template<> struct StripPtrRefConst<      void *> { typedef       void * Type; };
template<> struct StripPtrRefConst<const void *> { typedef const void * Type; };

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
class VarCallbacksMemFuncByValOrRef NTB_FINAL_CLASS
    : public VarCallbacksInterface
{
public:
    typedef typename StripPtrRefConst<OT>::Type ObjType;
    typedef typename StripPtrRefConst<VT>::Type VarType;

    typedef VT (OT::*GetCBType)() const;
    typedef RT (OT::*SetCBType)(VT);

    VarCallbacksMemFuncByValOrRef(const ObjType * o, GetCBType getCb, SetCBType setCb)
        : obj(const_cast<ObjType *>(o))
        , getter(getCb)
        , setter(setCb)
    { }
    void callGetter(void * valueOut) const NTB_OVERRIDE
    {
        NTB_ASSERT(obj    != NTB_NULL);
        NTB_ASSERT(getter != NTB_NULL);
        *static_cast<VarType *>(valueOut) = (obj->*getter)();
    }
    void callSetter(const void * valueIn) NTB_OVERRIDE
    {
        NTB_ASSERT(obj    != NTB_NULL);
        NTB_ASSERT(setter != NTB_NULL);
        (obj->*setter)(*static_cast<const VarType *>(valueIn));
    }
    VarCallbacksInterface * clone(void * where) const NTB_OVERRIDE
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
class VarCallbacksMemFuncByPointer NTB_FINAL_CLASS
    : public VarCallbacksInterface
{
public:
    typedef typename StripPtrRefConst<OT>::Type ObjType;
    typedef typename StripPtrRefConst<VT>::Type VarType;

    typedef void (OT::*GetCBType)(VarType *) const;
    typedef void (OT::*SetCBType)(const VarType *);

    VarCallbacksMemFuncByPointer(const ObjType * o, GetCBType getCb, SetCBType setCb)
        : obj(const_cast<ObjType *>(o))
        , getter(getCb)
        , setter(setCb)
    { }
    void callGetter(void * valueOut) const NTB_OVERRIDE
    {
        NTB_ASSERT(obj    != NTB_NULL);
        NTB_ASSERT(getter != NTB_NULL);
        (obj->*getter)(static_cast<VarType *>(valueOut));
    }
    void callSetter(const void * valueIn) NTB_OVERRIDE
    {
        NTB_ASSERT(obj    != NTB_NULL);
        NTB_ASSERT(setter != NTB_NULL);
        (obj->*setter)(static_cast<const VarType *>(valueIn));
    }
    VarCallbacksInterface * clone(void * where) const NTB_OVERRIDE
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
class VarCallbacksCFuncPtr NTB_FINAL_CLASS
    : public VarCallbacksInterface
{
public:
    typedef typename StripPtrRefConst<OT>::Type ObjType;
    typedef typename StripPtrRefConst<VT>::Type VarType;

    typedef void (*GetCBType)(const ObjType *, VarType *);
    typedef void (*SetCBType)(ObjType *, const VarType *);

    VarCallbacksCFuncPtr(const ObjType * o, GetCBType getCb, SetCBType setCb)
        : obj(const_cast<ObjType *>(o))
        , getter(getCb)
        , setter(setCb)
    { }
    void callGetter(void * valueOut) const NTB_OVERRIDE
    {
        NTB_ASSERT(getter != NTB_NULL);
        getter(obj, static_cast<VarType *>(valueOut));
    }
    void callSetter(const void * valueIn) NTB_OVERRIDE
    {
        NTB_ASSERT(setter != NTB_NULL);
        setter(obj, static_cast<const VarType *>(valueIn));
    }
    VarCallbacksInterface * clone(void * where) const NTB_OVERRIDE
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

class VarCallbacksAny NTB_FINAL_CLASS
{
public:

    template<typename OT, typename VT, typename RT>
    VarCallbacksAny(const detail::VarCallbacksMemFuncByValOrRef<OT, VT, RT> & cbs)
    {
        #if NEO_TWEAK_BAR_CXX11_SUPPORTED
        static_assert(DataSizeBytes >= sizeof(cbs), "Size mismatch!");
        #endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

        clear();
        callbacks = cbs.clone(getDataPtr());
    }

    template<typename OT, typename VT>
    VarCallbacksAny(const detail::VarCallbacksMemFuncByPointer<OT, VT> & cbs)
    {
        #if NEO_TWEAK_BAR_CXX11_SUPPORTED
        static_assert(DataSizeBytes >= sizeof(cbs), "Size mismatch!");
        #endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

        clear();
        callbacks = cbs.clone(getDataPtr());
    }

    template<typename OT, typename VT>
    VarCallbacksAny(const detail::VarCallbacksCFuncPtr<OT, VT> & cbs)
    {
        #if NEO_TWEAK_BAR_CXX11_SUPPORTED
        static_assert(DataSizeBytes >= sizeof(cbs), "Size mismatch!");
        #endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

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

    // Inplace storage for a VarCallbacks implementation:
    struct S;
    typedef detail::MaxAlign
    <
        detail::VarCallbacksMemFuncByValOrRef<S, Int64>,
        detail::VarCallbacksMemFuncByPointer<S, Int64>,
        detail::VarCallbacksCFuncPtr<S, Int64>
    > DataBlob;

    // The pointer will hold our placement-new constructed callback impl,
    // the data blob is where we place it, so heap allocations are avoided.
    detail::VarCallbacksInterface * callbacks;
    DataBlob blob;

    // We use the raw pointer to placement-new callbacks into it.
    void * getDataPtr() { return &blob; }
    enum { DataSizeBytes = sizeof(DataBlob) };
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
    // we still get a compiler error, but it might be a
    // little harder to figure out where is the offending method.
    #if NEO_TWEAK_BAR_CXX11_SUPPORTED
    static_assert(!std::is_pointer<VT>::value, "Variable cannot be a pointer for this type of callback!");
    #endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

    return detail::VarCallbacksMemFuncByValOrRef<OT, VT>(obj, getCb, NTB_NULL);
}

template<typename OT, typename VT, typename RT = void>
inline detail::VarCallbacksMemFuncByValOrRef<OT, VT, RT> callbacks(OT * obj, VT (OT::*getCb)() const, RT (OT::*setCb)(VT))
{
    // No pointers allowed here!
    // See the comment above in the other function.
    #if NEO_TWEAK_BAR_CXX11_SUPPORTED
    static_assert(!std::is_pointer<VT>::value, "Variable cannot be a pointer for this type of callbacks!");
    #endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

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
    return detail::VarCallbacksMemFuncByPointer<OT, VT>(obj, getCb, NTB_NULL);
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
    return detail::VarCallbacksCFuncPtr<OT, VT>(obj, getCb, NTB_NULL);
}

template<typename OT, typename VT>
inline detail::VarCallbacksCFuncPtr<OT, VT> callbacks(OT * obj, void (*getCb)(const OT *, VT *), void (*setCb)(OT *, const VT *))
{
    return detail::VarCallbacksCFuncPtr<OT, VT>(obj, getCb, setCb);
}

// ========================================================
// class Variable:
// ========================================================

class Variable
{
public:

    virtual ~Variable();

    // Get name/title and name hash code:
    virtual const char * getName() const = 0;
    virtual UInt32 getHashCode() const = 0;

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
typedef bool (*VariableEnumerateCallback)(Variable *, void *);

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

    virtual Variable * addNumberRO(const char * name, const Int8 * var) = 0;
    virtual Variable * addNumberRO(Variable * parent, const char * name, const Int8 * var) = 0;
    virtual Variable * addNumberRW(const char * name, Int8 * var) = 0;
    virtual Variable * addNumberRW(Variable * parent, const char * name, Int8 * var) = 0;

    virtual Variable * addNumberRO(const char * name, const UInt8 * var) = 0;
    virtual Variable * addNumberRO(Variable * parent, const char * name, const UInt8 * var) = 0;
    virtual Variable * addNumberRW(const char * name, UInt8 * var) = 0;
    virtual Variable * addNumberRW(Variable * parent, const char * name, UInt8 * var) = 0;

    virtual Variable * addNumberRO(const char * name, const Int16 * var) = 0;
    virtual Variable * addNumberRO(Variable * parent, const char * name, const Int16 * var) = 0;
    virtual Variable * addNumberRW(const char * name, Int16 * var) = 0;
    virtual Variable * addNumberRW(Variable * parent, const char * name, Int16 * var) = 0;

    virtual Variable * addNumberRO(const char * name, const UInt16 * var) = 0;
    virtual Variable * addNumberRO(Variable * parent, const char * name, const UInt16 * var) = 0;
    virtual Variable * addNumberRW(const char * name, UInt16 * var) = 0;
    virtual Variable * addNumberRW(Variable * parent, const char * name, UInt16 * var) = 0;

    virtual Variable * addNumberRO(const char * name, const Int32 * var) = 0;
    virtual Variable * addNumberRO(Variable * parent, const char * name, const Int32 * var) = 0;
    virtual Variable * addNumberRW(const char * name, Int32 * var) = 0;
    virtual Variable * addNumberRW(Variable * parent, const char * name, Int32 * var) = 0;

    virtual Variable * addNumberRO(const char * name, const UInt32 * var) = 0;
    virtual Variable * addNumberRO(Variable * parent, const char * name, const UInt32 * var) = 0;
    virtual Variable * addNumberRW(const char * name, UInt32 * var) = 0;
    virtual Variable * addNumberRW(Variable * parent, const char * name, UInt32 * var) = 0;

    virtual Variable * addNumberRO(const char * name, const Int64 * var) = 0;
    virtual Variable * addNumberRO(Variable * parent, const char * name, const Int64 * var) = 0;
    virtual Variable * addNumberRW(const char * name, Int64 * var) = 0;
    virtual Variable * addNumberRW(Variable * parent, const char * name, Int64 * var) = 0;

    virtual Variable * addNumberRO(const char * name, const UInt64 * var) = 0;
    virtual Variable * addNumberRO(Variable * parent, const char * name, const UInt64 * var) = 0;
    virtual Variable * addNumberRW(const char * name, UInt64 * var) = 0;
    virtual Variable * addNumberRW(Variable * parent, const char * name, UInt64 * var) = 0;

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
    virtual Variable * addColorRO(const char * name, const UInt8 * color, int size) = 0;
    virtual Variable * addColorRO(Variable * parent, const char * name, const UInt8 * color, int size) = 0;
    virtual Variable * addColorRW(const char * name, UInt8 * color, int size) = 0;
    virtual Variable * addColorRW(Variable * parent, const char * name, UInt8 * color, int size) = 0;

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
    virtual bool destroyVariable(Variable * variable) = 0;
    virtual void destroyAllVariables() = 0;
    virtual int getVariablesCount() const = 0;
    virtual void enumerateAllVariables(VariableEnumerateCallback enumCallback, void * userContext) = 0;

    // Miscellaneous accessors:
    virtual const char * getName() const = 0;
    virtual UInt32 getHashCode() const = 0;

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
typedef bool (*PanelEnumerateCallback)(Panel *, void *);

// ========================================================
// class GUI:
// ========================================================

class GUI
{
public:

    virtual ~GUI();

    // Panel creation and management:
    virtual Panel * findPanel(const char * panelName) const = 0;
    virtual Panel * createPanel(const char * panelName) = 0;
    virtual bool destroyPanel(Panel * panel) = 0;
    virtual void destroyAllPanels() = 0;
    virtual int getPanelCount() const = 0;
    virtual void enumerateAllPanels(PanelEnumerateCallback enumCallback, void * userContext) = 0;

    // Input events:
    virtual bool onKeyPressed(KeyCode key, KeyModFlags modifiers) = 0;
    virtual bool onMouseButton(MouseButton::Enum button, int clicks) = 0;
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
    virtual UInt32 getHashCode() const = 0;
};

// Callback for ntb::enumerateAllGUIs().
// First argument is the GUI being iterated, second is the
// user context/data passed to the enumerate function. Returning
// true continues the enumeration, retuning false stops it.
typedef bool (*GUIEnumerateCallback)(GUI *, void *);

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
ShellInterface  * getShellInterface();
RenderInterface * getRenderInterface();

// Find existing GUI by name. Returns null if not found.
// If more than one GUI with the same name exits, the
// first one found is returned.
GUI * findGUI(const char * guiName);

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
typedef void (*ErrorHandlerCallback)(const char *, void *);

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
