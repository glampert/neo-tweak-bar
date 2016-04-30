
// ================================================================================================
// -*- C++ -*-
// File: main.cpp
// Author: Guilherme R. Lampert
// Created on: 18/12/15
// ================================================================================================

#include "neo_tweak_bar.hpp"
extern void run_glfw_test_app();

//TEMP
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-const-variable"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"

//
// MEMORY ALLOCATION NOTES:
//
// We can provide an optional (that can be disabled by #ifdefs) memory
// allocator that uses a stack (or a list of stacks). The allocator is
// held by the ntb::UIManager, which holds all panels. Common usage pattern
// is to create the UI (thus filling the stack) then just destroy everything
// at once on shutdown (just roll back the stack). Should look into this later...
//
// RENDERER NOTES:
//
// Could provide a couple default implementations for the RenderInterface.
// Something standalone, so people that just want a quick-n-dirty
// GL renderer or a D3D renderer don't have to copy-paste code from the samples.
// Maybe have a GL-Core, GL-Fixed and D3D11 set of default renderers, each
// in a separate .hpp file that you can include and use quickly if you just
// want an ASAP C++ UI for your GL/D3D app. I'll need to implement those for
// the samples anyways, so might as well separate them for reuse.
//
// UI "SCRIPT":
//
// Might provide a text-based way of defining and configuring a panel,
// similar to what ATB provided. The panel configs are a simple INI-style
// set of value pair strings:
//
// [Panel="MyPanle"] # creates a new panel and sets it as current
// width=100
// height=100
// color=rgb(1,2,3)
// etc...
//
// DROP-IN ATB REPLACEMENT:
//
// Provide a drop in ATB interface built on top of NTB?
// Might be interesting to attract new users and interfacing
// with C libraries...
//
// CLEANUP NOTES:
// Last step is to remove any and all duplicated code and unused code.
// Make this library LEAN AND MEAN!
//

// --------------------------------------------------------------------

#include <utility>
#include <memory>

using namespace ntb;

template<typename T>
T * construct(T * obj, const T & other) // Copy constructor
{
    return ::new(obj) T(other);
}

template<typename T, typename... Args>
T * construct(T * obj, Args&&... args) // Parameterized or default constructor
{
    return ::new(obj) T(std::forward<Args>(args)...);
}

template<typename T>
void destroy(T * obj)
{
    if (obj != NTB_NULL)
    {
        obj->~T();
    }
}

// This is based on a similar trick presented on the book
// "Modern C++ Design: Generic Programming and Design Patterns"
// to avoid relying on the alignas() operator, only available on
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
        UByte    userDataT[sizeof(T)];
        UByte    userDataU[sizeof(U)];
        UByte    userDataV[sizeof(V)];
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

// --------------------------------------------------------------------

namespace new_
{

////////////////////////////////

class VarCallbacksInterface
{
public:
    virtual ~VarCallbacksInterface();
    virtual void callGetter(void * valueOut) const = 0;
    virtual void callSetter(const void * valueIn) = 0;
    virtual VarCallbacksInterface * clone(void * where) const = 0;
};

VarCallbacksInterface::~VarCallbacksInterface() { }

////////////////////////////////

// ========================================================
// class VarCallbacksMemFuncByValOrRef:
// ========================================================

template<typename OT, typename VT>
class VarCallbacksMemFuncByValOrRef NTB_FINAL_CLASS
    : public VarCallbacksInterface
{
public:
    typedef typename StripPtrRefConst<OT>::Type ObjType;
    typedef typename StripPtrRefConst<VT>::Type VarType;

    typedef VT   (OT::*GetCBType)() const;
    typedef void (OT::*SetCBType)(VT);

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
        return ::new(where) VarCallbacksMemFuncByValOrRef<OT, VT>(*this);
    }

private:
    ObjType * obj;
    GetCBType getter;
    SetCBType setter;
};

// ========================================================
// Callbacks from member functions,
// dealing with references or values:
//
// void VT OT::getCallback() const;
// void OT::setCallback(VT valueIn);
//
// VT can be a reference, const reference or value.
// ========================================================

template<typename OT, typename VT>
VarCallbacksMemFuncByValOrRef<OT, VT> callbacks(const OT * obj, VT (OT::*getCb)() const)
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
    // little harder to figure out where is the wrong method.
    #if NEO_TWEAK_BAR_CXX11_SUPPORTED
    static_assert(!std::is_pointer<VT>::value, "Variable cannot be a pointer for this type of callback!");
    #endif // NEO_TWEAK_BAR_CXX11_SUPPORTED
    return VarCallbacksMemFuncByValOrRef<OT, VT>(obj, getCb, NTB_NULL);
}

template<typename OT, typename VT>
VarCallbacksMemFuncByValOrRef<OT, VT> callbacks(OT * obj, VT (OT::*getCb)() const, void (OT::*setCb)(VT))
{
    // No pointers allowed here!
    // See the comment above in the other function.
    #if NEO_TWEAK_BAR_CXX11_SUPPORTED
    static_assert(!std::is_pointer<VT>::value, "Variable cannot be a pointer for this type of callbacks!");
    #endif // NEO_TWEAK_BAR_CXX11_SUPPORTED
    return VarCallbacksMemFuncByValOrRef<OT, VT>(obj, getCb, setCb);
}

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
// Callbacks from member functions, dealing with pointers:
//
// void OT::getCallback(VT * valueOut) const;
// void OT::setCallback(const VT * valueIn);
//
// ========================================================

template<typename OT, typename VT>
VarCallbacksMemFuncByPointer<OT, VT> callbacks(const OT * obj, void (OT::*getCb)(VT *) const)
{
    return VarCallbacksMemFuncByPointer<OT, VT>(obj, getCb, NTB_NULL);
}

template<typename OT, typename VT>
VarCallbacksMemFuncByPointer<OT, VT> callbacks(OT * obj, void (OT::*getCb)(VT *) const, void (OT::*setCb)(const VT *))
{
    return VarCallbacksMemFuncByPointer<OT, VT>(obj, getCb, setCb);
}

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

// ========================================================
// Callbacks from C-style function pointers:
//
// void getCallback(const OT * obj, VT * valueOut);
// void setCallback(OT * obj, const VT * valueIn);
//
// ========================================================

template<typename OT, typename VT>
VarCallbacksCFuncPtr<OT, VT> callbacks(const OT * obj, void (*getCb)(const OT *, VT *))
{
    return VarCallbacksCFuncPtr<OT, VT>(obj, getCb, NTB_NULL);
}

template<typename OT, typename VT>
VarCallbacksCFuncPtr<OT, VT> callbacks(OT * obj, void (*getCb)(const OT *, VT *), void (*setCb)(OT *, const VT *))
{
    return VarCallbacksCFuncPtr<OT, VT>(obj, getCb, setCb);
}

////////////////////////////////

class VarCallbacksAny NTB_FINAL_CLASS
{
public:

    VarCallbacksAny()
    {
        clear(); // isNull() == true;
    }

    VarCallbacksAny(const VarCallbacksAny & other)
    {
        clear();
        NTB_ASSERT(other.callbacks != NTB_NULL);
        callbacks = other.callbacks->clone(getDataPtr());
    }

    VarCallbacksAny & operator = (const VarCallbacksAny & other)
    {
        clear();
        NTB_ASSERT(other.callbacks != NTB_NULL);
        callbacks = other.callbacks->clone(getDataPtr());
        return *this;
    }

    template<typename OT, typename VT>
    VarCallbacksAny(const VarCallbacksMemFuncByValOrRef<OT, VT> & cbs)
    {
        #if NEO_TWEAK_BAR_CXX11_SUPPORTED
        static_assert(getDataSize() >= sizeof(cbs), "Size mismatch!");
        #endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

        clear();
        callbacks = cbs.clone(getDataPtr());
    }

    template<typename OT, typename VT>
    VarCallbacksAny(const VarCallbacksMemFuncByPointer<OT, VT> & cbs)
    {
        #if NEO_TWEAK_BAR_CXX11_SUPPORTED
        static_assert(getDataSize() >= sizeof(cbs), "Size mismatch!");
        #endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

        clear();
        callbacks = cbs.clone(getDataPtr());
    }

    template<typename OT, typename VT>
    VarCallbacksAny(const VarCallbacksCFuncPtr<OT, VT> & cbs)
    {
        #if NEO_TWEAK_BAR_CXX11_SUPPORTED
        static_assert(getDataSize() >= sizeof(cbs), "Size mismatch!");
        #endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

        clear();
        callbacks = cbs.clone(getDataPtr());
    }

    void callGetter(void * valueOut) const
    {
        NTB_ASSERT(callbacks != NTB_NULL);
        callbacks->callGetter(valueOut);
    }

    void callSetter(const void * valueIn)
    {
        NTB_ASSERT(callbacks != NTB_NULL);
        callbacks->callSetter(valueIn);
    }

    void clear()
    {
        // NOTE: Cutting a corner here. The correct would be to call
        // 'callbacks' destructor before setting it to null, but assuming
        // the implementation classes are simple types that allocate no
        // memory, we can ignore that and dodge the virtual destructor call.
        callbacks = NTB_NULL;
        std::memset(getDataPtr(), 0, getDataSize());
    }

    bool isNull() const { return callbacks == NTB_NULL; }

private:

    // Inplace storage for a VarCallbacks implementation:
    struct S;
    typedef MaxAlign
    <
        VarCallbacksMemFuncByValOrRef<S, Int64>,
        VarCallbacksMemFuncByPointer<S, Int64>,
        VarCallbacksCFuncPtr<S, Int64>
    >
    DataBlob;

    // The pointer will hold our placement-new constructed callback impl,
    // the data blob is where we place it, so heap allocations are avoided.
    VarCallbacksInterface * callbacks;
    DataBlob blob;

    // We use the raw pointer to placement-new callbacks into it.
    void * getDataPtr() { return &blob; }
    NTB_CONSTEXPR_FUNC static std::size_t getDataSize() { return sizeof(DataBlob); }
};

////////////////////////////////

//TODO eventually rename these to the UI* version
typedef NumberEx UINumber;
typedef Float4Ex UIF32Vec;
typedef ColorEx  UIColor;
typedef BoolEx   UIBool;

////////////////////////////////

#if NEO_TWEAK_BAR_STD_STRING_INTEROP

class UIString NTB_FINAL_CLASS
{
public:

    UIString()
    {
        state = Uninitialized;
        std::memset(&blob, 0, sizeof(blob));
    }

    ~UIString()
    {
        switch (state)
        {
        case IsSmallStr : { destroy(cast<SmallStr>());    break; }
        case IsStdStr   : { destroy(cast<std::string>()); break; }
        default         : break; // Uninitialized, OK.
        } // switch (state)
    }

    //
    // SmallStr/std::string wrapper:
    //

    void setCString(const char * str)
    {
        setCString(str, lengthOf(str));
    }

    void setCString(const char * str, const int len)
    {
        switch (state)
        {
        case IsSmallStr : { cast<SmallStr>()->setCString(str, len); break; }
        case IsStdStr   : { cast<std::string>()->assign(str, len);  break; }
        default         : badStateErr();
        } // switch (state)
    }

    void append(const char * str, const int len)
    {
        switch (state)
        {
        case IsSmallStr : { cast<SmallStr>()->append(str, len);    break; }
        case IsStdStr   : { cast<std::string>()->append(str, len); break; }
        default         : badStateErr();
        } // switch (state)
    }

    void resize(const int newLength, const bool preserveOldStr = true, const char fillVal = '\0')
    {
        switch (state)
        {
        case IsSmallStr : { cast<SmallStr>()->resize(newLength, preserveOldStr, fillVal); break; }
        case IsStdStr   : { cast<std::string>()->resize(newLength, fillVal); break; }
        default         : badStateErr();
        } // switch (state)
    }

    void clear()
    {
        switch (state)
        {
        case IsSmallStr : { cast<SmallStr>()->clear();    break; }
        case IsStdStr   : { cast<std::string>()->clear(); break; }
        default         : badStateErr();
        } // switch (state)
    }

    void setMaxSize(const int numChars)
    {
        switch (state)
        {
        case IsSmallStr : { cast<SmallStr>()->setMaxSize(numChars); break; }
        case IsStdStr   : break; // Unsupported.
        default         : badStateErr();
        } // switch (state)
    }

    int getMaxSize() const
    {
        switch (state)
        {
        case IsSmallStr :
            {
                // If a MaxSize isn't specified assume the char
                // buffer is at least CStringMaxSize chars.
                const SmallStr * str = cast<SmallStr>();
                return ((str->getMaxSize() > 0) ? str->getMaxSize() : Panel::CStringMaxSize);
            }
        case IsStdStr : { return Panel::CStringMaxSize; } // Unsupported.
        default       : { badStateErr(); return 0; }
        } // switch (state)
    }

    bool isEmpty() const
    {
        switch (state)
        {
        case IsSmallStr : return cast<SmallStr>()->isEmpty();
        case IsStdStr   : return cast<std::string>()->empty();
        default         : { badStateErr(); return true; }
        } // switch (state)
    }

    int getLength() const
    {
        switch (state)
        {
        case IsSmallStr : return cast<SmallStr>()->getLength();
        case IsStdStr   : return static_cast<int>(cast<std::string>()->length());
        default         : { badStateErr(); return 0; }
        } // switch (state)
    }

    char getCharAt(const int index) const
    {
        switch (state)
        {
        case IsSmallStr : return (*cast<SmallStr>())[index];
        case IsStdStr   : return (*cast<std::string>())[index];
        default         : { badStateErr(); return '\0'; }
        } // switch (state)
    }

    void setCharAt(const int index, const char ch)
    {
        switch (state)
        {
        case IsSmallStr : (*cast<SmallStr>())[index] = ch;
        case IsStdStr   : (*cast<std::string>())[index] = ch;
        default         : badStateErr();
        } // switch (state)
    }

    //
    // Get the underlaying C-string-style char array:
    //
    const char * getAsCStr() const
    {
        switch (state)
        {
        case IsSmallStr : return cast<SmallStr>()->c_str();
        case IsStdStr   : return cast<std::string>()->c_str();
        default         : { badStateErr(); return NTB_NULL; }
        } // switch (state)
    }
    char * getAsCStr()
    {
        switch (state)
        {
        case IsSmallStr : return cast<SmallStr>()->c_str();
        default         : { badStateErr(); return NTB_NULL; }
        // std::string doesn't support the mutable version of c_str().
        } // switch (state)
    }

    //
    // Reinterpret as SmallStr:
    //
    void initSmallStr()
    {
        NTB_ASSERT(state == Uninitialized);
        construct(cast<SmallStr>());
        state = IsSmallStr;
    }
    const SmallStr * getAsSmallStr() const
    {
        NTB_ASSERT(state == IsSmallStr);
        return cast<SmallStr>();
    }
    SmallStr * getAsSmallStr()
    {
        NTB_ASSERT(state == IsSmallStr);
        return cast<SmallStr>();
    }

    //
    // Reinterpret as std::string:
    //
    void initStdStr()
    {
        NTB_ASSERT(state == Uninitialized);
        construct(cast<std::string>());
        state = IsStdStr;
    }
    const std::string * getAsStdStr() const
    {
        NTB_ASSERT(state == IsStdStr);
        return cast<std::string>();
    }
    std::string * getAsStdStr()
    {
        NTB_ASSERT(state == IsStdStr);
        return cast<std::string>();
    }

private:

    // Raw bytes for the string object we're allocating.
    MaxAlign<SmallStr, std::string> blob;

    // Tag to select from the above union.
    enum { Uninitialized, IsSmallStr, IsStdStr } state;

    // Casting helpers:
    template<typename T> const T * cast() const { return reinterpret_cast<const T *>(&blob); }
    template<typename T>       T * cast()       { return reinterpret_cast<      T *>(&blob); }

    static void badStateErr()
    {
        NTB_ERROR("Bad internal UIString state!");
        NTB_ASSERT(false); // Assuming NTB_ERROR didn't throw or abort...
    }
};

#else // !NEO_TWEAK_BAR_STD_STRING_INTEROP

//TODO UIString is just a wrapper for SmallStr

#endif // NEO_TWEAK_BAR_STD_STRING_INTEROP

////////////////////////////////

struct EnumConstant NTB_FINAL_CLASS
{
    const char * const name;
    const Int64        value;

    EnumConstant(const char * n, const Int64 v)
        : name(n)
        , value(v)
    { }

    template<typename EnumType>
    EnumConstant(const char * n, const EnumType v)
        : name(n)
        , value(static_cast<Int64>(v))
    { }
};

template<typename EnumType>
EnumConstant EnumTypeDecl()
{
    // Dummy constant with the size of a value of type EnumType
    // This is not displayed in the UI.
    return EnumConstant("(enum size bytes)", sizeof(EnumType));
}

class UIEnum NTB_FINAL_CLASS
{
public:

    Int64 value;

    UIEnum(const EnumConstant * consts, const int num)
        : constants(consts)
        , numOfConsts(num)
    { }

    const char * getConstName(const int index) const
    {
        NTB_ASSERT(constants != NTB_NULL);
        NTB_ASSERT(index >= 0 && index < numOfConsts);
        return constants[index].name;
    }
    Int64 getConstValue(const int index) const
    {
        NTB_ASSERT(constants != NTB_NULL);
        NTB_ASSERT(index >= 0 && index < numOfConsts);
        return constants[index].value;
    }

    int getEnumConstSize() const
    {
        const Int64 size = getConstValue(0);
        NTB_ASSERT(size > 0 && static_cast<std::size_t>(size) <= sizeof(Int64));
        return static_cast<int>(size);
    }
    int getNumOfConsts() const
    {
        return numOfConsts;
    }

private:

    // First constant should be the enum size dummy.
    // Following are the displayable constants.
    const EnumConstant * const constants;
    const int                  numOfConsts;
};

////////////////////////////////

class UserVar NTB_FINAL_CLASS
{
public:

    //
    // Type tags:
    //
    struct Type
    {
        enum
        {
            // Subtype::Number
            I8,
            U8,
            I16,
            U16,
            I32,
            U32,
            I64,
            U64,
            F32,
            F64,
            Ptr,

            // Subtype::F32Vec
            FVec2,
            FVec3,
            FVec4,
            FDir3,
            FQuat4,

            // Subtype::Color
            BColor3,
            BColor4,
            FColor3,
            FColor4,
            IColor4,

            // Subtype::String
            Char,
            CStr,
            #if NEO_TWEAK_BAR_STD_STRING_INTEROP
            StdStr,
            #endif // NEO_TWEAK_BAR_STD_STRING_INTEROP

            // Subtype::Bool
            BoolVal,

            // Subtype::UserEnum
            EnumVal,

            // Sentinel value; Used internally.
            Count
        };
        typedef Int8 Enum;
    };

    // Subtypes match the underlaying UIDisplayVar types.
    struct Subtype
    {
        enum
        {
            Number,   // UINumber
            F32Vec,   // UIF32Vec
            Color,    // UIColor
            String,   // UIString
            Bool,     // UIBool
            UserEnum, // UIEnum

            // Sentinel value; Used internally.
            Count
        };
        typedef Int8 Enum;
    };

    //
    // Value union:
    //
    union
    {
        void           * asAny;
        Int8           * asI8;
        UInt8          * asU8;
        Int16          * asI16;
        UInt16         * asU16;
        Int32          * asI32;
        UInt32         * asU32;
        Int64          * asI64;
        UInt64         * asU64;
        Float32        * asF32;
        Float64        * asF64;
        std::uintptr_t * asPtr;
        bool           * asBool;
        char           * asCStr;
        #if NEO_TWEAK_BAR_STD_STRING_INTEROP
        std::string    * asStdStr;
        #endif // NEO_TWEAK_BAR_STD_STRING_INTEROP
    };

    //
    // Members:
    //
    const Type::Enum    type;
    const Subtype::Enum subtype;
    VarCallbacksAny     callbacks;

    //
    // Constructors/helpers:
    //
    UserVar(void * ptr, const Type::Enum t, const Subtype::Enum sub, const VarCallbacksAny & cbs)
        : type(t)
        , subtype(sub)
        , callbacks(cbs)
    {
        asAny = ptr; // Null only if callbacks are provided.
        if (!callbacks.isNull())
        {
            if (asAny != NTB_NULL)
            {
                NTB_ERROR("User data pointer should be null when VarCallbacks are provided!");
            }
        }
        else
        {
            if (asAny == NTB_NULL)
            {
                NTB_ERROR("User data pointer must not be null when no VarCallbacks are provided!");
            }
        }
    }

    void get(void * dest) const
    {
        switch (subtype)
        {
        case Subtype::Number   : { getNumber   (static_cast<UINumber *>(dest)); break; }
        case Subtype::F32Vec   : { getF32Vec   (static_cast<UIF32Vec *>(dest)); break; }
        case Subtype::Color    : { getColor    (static_cast<UIColor  *>(dest)); break; }
        case Subtype::String   : { getString   (static_cast<UIString *>(dest)); break; }
        case Subtype::Bool     : { getBoolVal  (static_cast<UIBool   *>(dest)); break; }
        case Subtype::UserEnum : { getUserEnum (static_cast<UIEnum   *>(dest)); break; }
        default : NTB_ERROR("Invalid variable subtype tag!");
        } // switch (subtype)
    }

    void set(const void * src)
    {
        switch (subtype)
        {
        case Subtype::Number   : { setNumber  (static_cast<const UINumber *>(src)); break; }
        case Subtype::F32Vec   : { setF32Vec  (static_cast<const UIF32Vec *>(src)); break; }
        case Subtype::Color    : { setColor   (static_cast<const UIColor  *>(src)); break; }
        case Subtype::String   : { setString  (static_cast<const UIString *>(src)); break; }
        case Subtype::Bool     : { setBoolVal (static_cast<const UIBool   *>(src)); break; }
        case Subtype::UserEnum : { setUserEnum(static_cast<const UIEnum   *>(src)); break; }
        default : NTB_ERROR("Invalid variable subtype tag!");
        } // switch (subtype)
    }

private:

    //
    //TODO note: maybe some of this conversion logic ought to go into the UI types themselves...
    //

    //
    // Getters:
    //

    void getBoolVal(UIBool * dest) const
    {
        NTB_ASSERT(subtype == Subtype::Bool);
        NTB_ASSERT(type    == Type::BoolVal);
        if (!callbacks.isNull()) { callbacks.callGetter(&dest->value); }
        else { dest->value = *asBool; }
    }

    template<int Size>
    void getVecFloats(const Float32 * src, UIF32Vec * dest) const
    {
        NTB_ASSERT(Size == dest->getSize());
        if (!callbacks.isNull()) { callbacks.callGetter(dest->values); }
        else { dest->setFloats(src); }
    }

    template<int Size>
    void getColorChannels32(const UInt32 * src, UIColor * dest) const
    {
        NTB_ASSERT(Size == dest->numChannels);
        if (!callbacks.isNull()) { callbacks.callGetter(&dest->rgba32); }
        else { dest->setColor32(src); }
    }

    template<int Size>
    void getColorChannelsB(const UByte * src, UIColor * dest) const
    {
        NTB_ASSERT(Size == dest->numChannels);
        if (!callbacks.isNull()) { callbacks.callGetter(dest->rgbaB4); }
        else { dest->setColorB(src); }
    }

    template<int Size>
    void getColorChannelsF(const Float32 * src, UIColor * dest) const
    {
        NTB_ASSERT(Size == dest->numChannels);
        if (!callbacks.isNull()) { callbacks.callGetter(dest->rgbaF4); }
        else { dest->setColorF(src); }
    }

    void getUserEnum(UIEnum * dest) const
    {
        NTB_ASSERT(subtype == Subtype::UserEnum);
        NTB_ASSERT(type    == Type::EnumVal);

        if (!callbacks.isNull())
        {
            // The user enum type might be smaller, so clear first.
            dest->value = 0;
            callbacks.callGetter(&dest->value);
        }
        else
        {
            switch (dest->getEnumConstSize())
            {
            case sizeof(Int8)  : { dest->value = *asI8;  break; }
            case sizeof(Int16) : { dest->value = *asI16; break; }
            case sizeof(Int32) : { dest->value = *asI32; break; }
            case sizeof(Int64) : { dest->value = *asI64; break; }
            default : NTB_ERROR("Bad user enum constant size!");
            } // switch (dest->getEnumConstSize())
        }
    }

    template<typename SrcT, typename DestT>
    void getNum(const SrcT * src, DestT * dest) const
    {
        if (!callbacks.isNull())
        {
            SrcT temp;
            callbacks.callGetter(&temp);
            *dest = static_cast<DestT>(temp);
        }
        else
        {
            *dest = static_cast<DestT>(*src);
        }
    }

    void getPtr(const std::uintptr_t * src, void ** dest) const
    {
        if (!callbacks.isNull())
        {
            std::uintptr_t temp;
            callbacks.callGetter(&temp);
            *dest = reinterpret_cast<void *>(temp);
        }
        else
        {
            *dest = reinterpret_cast<void *>(*src);
        }
    }

    void getSingleChar(const char * src, UIString * dest) const
    {
        NTB_ASSERT(dest->getMaxSize() == 2); // The char + NUL-terminator
        if (!callbacks.isNull()) { callbacks.callGetter(dest->getAsCStr()); }
        else { dest->setCharAt(0, *src); }
    }

    void getCStr(const char * src, UIString * dest) const
    {
        if (!callbacks.isNull())
        {
            dest->resize(dest->getMaxSize());
            callbacks.callGetter(dest->getAsCStr());
        }
        else
        {
            dest->setCString(src);
        }
    }

    #if NEO_TWEAK_BAR_STD_STRING_INTEROP
    void getStdStr(const std::string * src, UIString * dest) const
    {
        if (!callbacks.isNull()) { callbacks.callGetter(dest->getAsStdStr()); }
        else { dest->setCString(src->c_str(), static_cast<int>(src->length())); }
    }
    #endif // NEO_TWEAK_BAR_STD_STRING_INTEROP

    void getNumber(UINumber * dest) const
    {
        NTB_ASSERT(subtype == Subtype::Number);
        switch (type)
        {
        case Type::I8  : { getNum(asI8,  &dest->asI64);  break; }
        case Type::U8  : { getNum(asU8,  &dest->asU64);  break; }
        case Type::I16 : { getNum(asI16, &dest->asI64);  break; }
        case Type::U16 : { getNum(asU16, &dest->asU64);  break; }
        case Type::I32 : { getNum(asI32, &dest->asI64);  break; }
        case Type::U32 : { getNum(asU32, &dest->asU64);  break; }
        case Type::I64 : { getNum(asI64, &dest->asI64);  break; }
        case Type::U64 : { getNum(asU64, &dest->asU64);  break; }
        case Type::F32 : { getNum(asF32, &dest->asF64);  break; }
        case Type::F64 : { getNum(asF64, &dest->asF64);  break; }
        case Type::Ptr : { getPtr(asPtr, &dest->asVPtr); break; }
        default : NTB_ERROR("Invalid variable type tag!");
        } // switch (type)
    }

    void getF32Vec(UIF32Vec * dest) const
    {
        NTB_ASSERT(subtype == Subtype::F32Vec);
        switch (type)
        {
        case Type::FVec2  : { getVecFloats<2>(asF32, dest); break; }
        case Type::FVec3  : { getVecFloats<3>(asF32, dest); break; }
        case Type::FVec4  : { getVecFloats<4>(asF32, dest); break; }
        case Type::FDir3  : { getVecFloats<3>(asF32, dest); break; }
        case Type::FQuat4 : { getVecFloats<4>(asF32, dest); break; }
        default : NTB_ERROR("Invalid variable type tag!");
        } // switch (type)
    }

    void getColor(UIColor * dest) const
    {
        NTB_ASSERT(subtype == Subtype::Color);
        switch (type)
        {
        case Type::BColor3 : { getColorChannelsB<3>(asU8, dest);   break; }
        case Type::BColor4 : { getColorChannelsB<4>(asU8, dest);   break; }
        case Type::FColor3 : { getColorChannelsF<3>(asF32, dest);  break; }
        case Type::FColor4 : { getColorChannelsF<4>(asF32, dest);  break; }
        case Type::IColor4 : { getColorChannels32<4>(asU32, dest); break; }
        default : NTB_ERROR("Invalid variable type tag!");
        } // switch (type)
    }

    void getString(UIString * dest) const
    {
        NTB_ASSERT(subtype == Subtype::String);
        switch (type)
        {
        case Type::Char :
            {
                getSingleChar(asCStr, dest);
                break;
            }
        case Type::CStr :
            {
                getCStr(asCStr, dest);
                break;
            }
        #if NEO_TWEAK_BAR_STD_STRING_INTEROP
        case Type::StdStr :
            {
                getStdStr(asStdStr, dest);
                break;
            }
        #endif // NEO_TWEAK_BAR_STD_STRING_INTEROP
        default : NTB_ERROR("Invalid variable type tag!");
        } // switch (type)
    }

    //
    // Setters:
    //

    void setBoolVal(const UIBool * src)
    {
        NTB_ASSERT(subtype == Subtype::Bool);
        NTB_ASSERT(type    == Type::BoolVal);
        if (!callbacks.isNull()) { callbacks.callSetter(&src->value); }
        else { *asBool = src->value; }
    }

    template<int Size>
    void setVecFloats(Float32 * dest, const UIF32Vec * src)
    {
        NTB_ASSERT(Size == src->getSize());
        if (!callbacks.isNull()) { callbacks.callSetter(src->values); }
        else { src->getFloats(dest); }
    }

    template<int Size>
    void setColorChannels32(UInt32 * dest, const UIColor * src)
    {
        NTB_ASSERT(Size == src->numChannels);
        if (!callbacks.isNull()) { callbacks.callSetter(&src->rgba32); }
        else { src->getColor32(dest); }
    }

    template<int Size>
    void setColorChannelsB(UByte * dest, const UIColor * src)
    {
        NTB_ASSERT(Size == src->numChannels);
        if (!callbacks.isNull()) { callbacks.callSetter(src->rgbaB4); }
        else { src->getColorB(dest); }
    }

    template<int Size>
    void setColorChannelsF(Float32 * dest, const UIColor * src)
    {
        NTB_ASSERT(Size == src->numChannels);
        if (!callbacks.isNull()) { callbacks.callSetter(src->rgbaF4); }
        else { src->getColorF(dest); }
    }

    void setUserEnum(const UIEnum * dest)
    {
        NTB_ASSERT(subtype == Subtype::UserEnum);
        NTB_ASSERT(type    == Type::EnumVal);

        if (!callbacks.isNull())
        {
            const Int64 temp = dest->value;
            callbacks.callSetter(&temp);
        }
        else
        {
            switch (dest->getEnumConstSize())
            {
            case sizeof(Int8)  : { *asI8  = static_cast<Int8 >(dest->value); break; }
            case sizeof(Int16) : { *asI16 = static_cast<Int16>(dest->value); break; }
            case sizeof(Int32) : { *asI32 = static_cast<Int32>(dest->value); break; }
            case sizeof(Int64) : { *asI64 = static_cast<Int64>(dest->value); break; }
            default : NTB_ERROR("Bad user enum constant size!");
            } // switch (dest->getEnumConstSize())
        }
    }

    template<typename DestT, typename SrcT>
    void setNum(DestT * dest, const SrcT * src)
    {
        if (!callbacks.isNull())
        {
            const DestT temp = static_cast<DestT>(*src);
            callbacks.callSetter(&temp);
        }
        else
        {
            *dest = static_cast<DestT>(*src);
        }
    }

    void setPtr(std::uintptr_t * dest, const void * src)
    {
        if (!callbacks.isNull())
        {
            const std::uintptr_t temp = reinterpret_cast<std::uintptr_t>(src);
            callbacks.callSetter(&temp);
        }
        else
        {
            *dest = reinterpret_cast<std::uintptr_t>(src);
        }
    }

    void setSingleChar(char * dest, const UIString * src)
    {
        NTB_ASSERT(src->getMaxSize() == 2); // The char + NUL-terminator
        if (!callbacks.isNull()) { callbacks.callSetter(src->getAsCStr()); }
        else { *dest = src->getCharAt(0); }
    }

    void setCStr(char * dest, const UIString * src)
    {
        if (!callbacks.isNull()) { callbacks.callSetter(src->getAsCStr()); }
        else { copyString(dest, src->getMaxSize(), src->getAsCStr()); }
    }

    #if NEO_TWEAK_BAR_STD_STRING_INTEROP
    void setStdStr(std::string * dest, const UIString * src)
    {
        if (!callbacks.isNull()) { callbacks.callSetter(src->getAsStdStr()); }
        else { *dest = *(src->getAsStdStr()); }
    }
    #endif // NEO_TWEAK_BAR_STD_STRING_INTEROP

    void setNumber(const UINumber * src)
    {
        NTB_ASSERT(subtype == Subtype::Number);
        switch (type)
        {
        case Type::I8  : { setNum(asI8,  &src->asI64);  break; }
        case Type::U8  : { setNum(asU8,  &src->asU64);  break; }
        case Type::I16 : { setNum(asI16, &src->asI64);  break; }
        case Type::U16 : { setNum(asU16, &src->asU64);  break; }
        case Type::I32 : { setNum(asI32, &src->asI64);  break; }
        case Type::U32 : { setNum(asU32, &src->asU64);  break; }
        case Type::I64 : { setNum(asI64, &src->asI64);  break; }
        case Type::U64 : { setNum(asU64, &src->asU64);  break; }
        case Type::F32 : { setNum(asF32, &src->asF64);  break; }
        case Type::F64 : { setNum(asF64, &src->asF64);  break; }
        case Type::Ptr : { setPtr(asPtr, src->asVPtr);  break; }
        default : NTB_ERROR("Invalid variable type tag!");
        } // switch (type)
    }

    void setF32Vec(const UIF32Vec * src)
    {
        NTB_ASSERT(subtype == Subtype::F32Vec);
        switch (type)
        {
        case Type::FVec2  : { setVecFloats<2>(asF32, src); break; }
        case Type::FVec3  : { setVecFloats<3>(asF32, src); break; }
        case Type::FVec4  : { setVecFloats<4>(asF32, src); break; }
        case Type::FDir3  : { setVecFloats<3>(asF32, src); break; }
        case Type::FQuat4 : { setVecFloats<4>(asF32, src); break; }
        default : NTB_ERROR("Invalid variable type tag!");
        } // switch (type)
    }

    void setColor(const UIColor * src)
    {
        NTB_ASSERT(subtype == Subtype::Color);
        switch (type)
        {
        case Type::BColor3 : { setColorChannelsB<3>(asU8, src);   break; }
        case Type::BColor4 : { setColorChannelsB<4>(asU8, src);   break; }
        case Type::FColor3 : { setColorChannelsF<3>(asF32, src);  break; }
        case Type::FColor4 : { setColorChannelsF<4>(asF32, src);  break; }
        case Type::IColor4 : { setColorChannels32<4>(asU32, src); break; }
        default : NTB_ERROR("Invalid variable type tag!");
        } // switch (type)
    }

    void setString(const UIString * src)
    {
        NTB_ASSERT(subtype == Subtype::String);
        switch (type)
        {
        case Type::Char :
            {
                setSingleChar(asCStr, src);
                break;
            }
        case Type::CStr :
            {
                setCStr(asCStr, src);
                break;
            }
        #if NEO_TWEAK_BAR_STD_STRING_INTEROP
        case Type::StdStr :
            {
                setStdStr(asStdStr, src);
                break;
            }
        #endif // NEO_TWEAK_BAR_STD_STRING_INTEROP
        default : NTB_ERROR("Invalid variable type tag!");
        } // switch (type)
    }
};

template<typename UIDisplayVar>
class VariableImpl NTB_FINAL_CLASS
    : public Variable
{
public:

    VariableImpl(Panel * owner, Variable * parent, const char * name, const Access access, const void * userDataPtr,
                 const UserVar::Type::Enum userDataType, const UserVar::Subtype::Enum userDataSubtype,
                 const VarCallbacksAny & callbacks, const UIDisplayVar & dispVarInit)
        : Variable(owner, parent, name)
        , userVar(const_cast<void *>(userDataPtr), userDataType, userDataSubtype, callbacks)
        , displayVar(dispVarInit)
        , accessMode(access)
    { }

    //TEMP
    Access getAccessMode() const NTB_OVERRIDE { return accessMode; }
    Variable * setNumberFormatting(int numericBase) NTB_OVERRIDE { (void)numericBase; return this; }
    Variable * setMaxStringSize(int maxSizeIncludingNulTerminator) NTB_OVERRIDE { (void)maxSizeIncludingNulTerminator; return this; }
    void onLinkedToPanel(Panel & owner) NTB_OVERRIDE { (void)owner; }
    void onDraw(GeometryBatch & geoBatch) const NTB_OVERRIDE { (void)geoBatch; }

    // Write userVar over displayVar
    void onUpdateDisplayValue() const NTB_OVERRIDE
    {
        userVar.get(&displayVar);
    }

    // Write displayVar over userVar
    void onUpdateUserValue() const NTB_OVERRIDE
    {
        if (accessMode == Variable::ReadWrite)
        {
            userVar.set(&displayVar);
        }
    }

private:

    mutable UserVar      userVar;
    mutable UIDisplayVar displayVar;
    const   Access       accessMode;
};

typedef VariableImpl<UINumber> VariableImplNumber;
typedef VariableImpl<UIF32Vec> VariableImplF32Vec;
typedef VariableImpl<UIColor>  VariableImplColor;
typedef VariableImpl<UIString> VariableImplString;
typedef VariableImpl<UIBool>   VariableImplBool;
typedef VariableImpl<UIEnum>   VariableImplEnum;

static Variable * newVarNumber(Panel * owner, Variable * parent, const char * name, const Variable::Access access,
                               const void * userDataPtr, const UserVar::Type::Enum userDataType, const VarCallbacksAny & callbacks)
{
    UINumber numberStore;
    switch (userDataType)
    {
    case UserVar::Type::I8  : { numberStore.type = UINumber::Type::SignedInt;     break; }
    case UserVar::Type::U8  : { numberStore.type = UINumber::Type::UnsignedInt;   break; }
    case UserVar::Type::I16 : { numberStore.type = UINumber::Type::SignedInt;     break; }
    case UserVar::Type::U16 : { numberStore.type = UINumber::Type::UnsignedInt;   break; }
    case UserVar::Type::I32 : { numberStore.type = UINumber::Type::SignedInt;     break; }
    case UserVar::Type::U32 : { numberStore.type = UINumber::Type::UnsignedInt;   break; }
    case UserVar::Type::I64 : { numberStore.type = UINumber::Type::SignedInt;     break; }
    case UserVar::Type::U64 : { numberStore.type = UINumber::Type::UnsignedInt;   break; }
    case UserVar::Type::F32 : { numberStore.type = UINumber::Type::FloatingPoint; break; }
    case UserVar::Type::F64 : { numberStore.type = UINumber::Type::FloatingPoint; break; }
    case UserVar::Type::Ptr : { numberStore.type = UINumber::Type::Pointer;       break; }
    default : NTB_ERROR("Invalid variable type tag!");
    } // switch (userDataType)

    VariableImplNumber * var = memAlloc<VariableImplNumber>(1);
    return construct(var, owner, parent, name, access, userDataPtr,
                     userDataType, UserVar::Subtype::Number, callbacks, numberStore);
}

static Variable * newVarF32Vec(Panel * owner, Variable * parent, const char * name, const Variable::Access access,
                               const void * userDataPtr, const UserVar::Type::Enum userDataType, const VarCallbacksAny & callbacks)
{
    UIF32Vec fvecStore;
    switch (userDataType)
    {
    case UserVar::Type::FVec2  : { fvecStore.type = UIF32Vec::Type::Vec2;  break; }
    case UserVar::Type::FVec3  : { fvecStore.type = UIF32Vec::Type::Vec3;  break; }
    case UserVar::Type::FVec4  : { fvecStore.type = UIF32Vec::Type::Vec4;  break; }
    case UserVar::Type::FDir3  : { fvecStore.type = UIF32Vec::Type::Dir3;  break; }
    case UserVar::Type::FQuat4 : { fvecStore.type = UIF32Vec::Type::Quat4; break; }
    default : NTB_ERROR("Invalid variable type tag!");
    } // switch (userDataType)

    VariableImplF32Vec * var = memAlloc<VariableImplF32Vec>(1);
    return construct(var, owner, parent, name, access, userDataPtr,
                     userDataType, UserVar::Subtype::F32Vec, callbacks, fvecStore);
}

static Variable * newVarColor(Panel * owner, Variable * parent, const char * name, const Variable::Access access,
                              const void * userDataPtr, const UserVar::Type::Enum userDataType, const VarCallbacksAny & callbacks)
{
    UIColor colorStore;
    switch (userDataType)
    {
    case UserVar::Type::BColor3 : { colorStore.numChannels = 3; colorStore.displayMode = UIColor::Display::CByte;  break; }
    case UserVar::Type::BColor4 : { colorStore.numChannels = 4; colorStore.displayMode = UIColor::Display::CByte;  break; }
    case UserVar::Type::FColor3 : { colorStore.numChannels = 3; colorStore.displayMode = UIColor::Display::CFloat; break; }
    case UserVar::Type::FColor4 : { colorStore.numChannels = 4; colorStore.displayMode = UIColor::Display::CFloat; break; }
    case UserVar::Type::IColor4 : { colorStore.numChannels = 4; colorStore.displayMode = UIColor::Display::CByte;  break; }
    default : NTB_ERROR("Invalid variable type tag!");
    } // switch (userDataType)

    VariableImplColor * var = memAlloc<VariableImplColor>(1);
    return construct(var, owner, parent, name, access, userDataPtr,
                     userDataType, UserVar::Subtype::Color, callbacks, colorStore);
}

static Variable * newVarString(Panel * owner, Variable * parent, const char * name, const Variable::Access access,
                               const void * userDataPtr, const UserVar::Type::Enum userDataType, const VarCallbacksAny & callbacks)
{
    UIString strStore;

    #if NEO_TWEAK_BAR_STD_STRING_INTEROP
    if (userDataType == UserVar::Type::StdStr)
    {
        strStore.initStdStr();
    }
    else
    {
        strStore.initSmallStr();
    }
    #endif // NEO_TWEAK_BAR_STD_STRING_INTEROP

    // Single-chars are also stored as strings.
    if (userDataType == UserVar::Type::Char)
    {
        strStore.setMaxSize(2);      // 1 for the char + a NUL terminator to make it a valid C-string.
        strStore.setCString("?", 1); // Set length to 1 char and initialize with a default '?'.
    }

    VariableImplString * var = memAlloc<VariableImplString>(1);
    return construct(var, owner, parent, name, access, userDataPtr,
                     userDataType, UserVar::Subtype::String, callbacks, strStore);
}

static Variable * newVarBool(Panel * owner, Variable * parent, const char * name, const Variable::Access access,
                             const void * userDataPtr, const UserVar::Type::Enum userDataType, const VarCallbacksAny & callbacks)
{
    VariableImplBool * var = memAlloc<VariableImplBool>(1);
    return construct(var, owner, parent, name, access, userDataPtr,
                     userDataType, UserVar::Subtype::Bool, callbacks, UIBool());
}

static Variable * newVarEnum(Panel * owner, Variable * parent, const char * name, const Variable::Access access,
                             const void * userDataPtr, const UserVar::Type::Enum userDataType, const VarCallbacksAny & callbacks,
                             const void * enumConsts, const int numOfConsts)
{
    NTB_ASSERT(enumConsts != NTB_NULL);
    UIEnum enumStore(static_cast<const EnumConstant *>(enumConsts), numOfConsts);

    VariableImplEnum * var = memAlloc<VariableImplEnum>(1);
    return construct(var, owner, parent, name, access, userDataPtr,
                     userDataType, UserVar::Subtype::UserEnum, callbacks, enumStore);
}

// --------------------------------------------------------------------

static Variable * newVarImpl(Panel * ownerPanel, Variable * parent, const char * name,
                             const Variable::Access access, const void * userDataPtr,
                             const UserVar::Type::Enum userDataType, const VarCallbacksAny & callbacks,
                             const void * extraData, const int extraDataSize)
{
    switch (userDataType)
    {
    // Numbers:
    case UserVar::Type::I8  :
    case UserVar::Type::U8  :
    case UserVar::Type::I16 :
    case UserVar::Type::U16 :
    case UserVar::Type::I32 :
    case UserVar::Type::U32 :
    case UserVar::Type::I64 :
    case UserVar::Type::U64 :
    case UserVar::Type::F32 :
    case UserVar::Type::F64 :
    case UserVar::Type::Ptr :
        {
            return newVarNumber(ownerPanel, parent, name, access,
                                userDataPtr, userDataType, callbacks);
        }
    // Small float vectors:
    case UserVar::Type::FVec2  :
    case UserVar::Type::FVec3  :
    case UserVar::Type::FVec4  :
    case UserVar::Type::FDir3  :
    case UserVar::Type::FQuat4 :
        {
            return newVarF32Vec(ownerPanel, parent, name, access,
                                userDataPtr, userDataType, callbacks);
        }
    // Colors:
    case UserVar::Type::BColor3 :
    case UserVar::Type::BColor4 :
    case UserVar::Type::FColor3 :
    case UserVar::Type::FColor4 :
    case UserVar::Type::IColor4 :
        {
            return newVarColor(ownerPanel, parent, name, access,
                               userDataPtr, userDataType, callbacks);
        }
    // Strings/single-char:
    case UserVar::Type::Char :
    case UserVar::Type::CStr :
    #if NEO_TWEAK_BAR_STD_STRING_INTEROP
    case UserVar::Type::StdStr :
    #endif // NEO_TWEAK_BAR_STD_STRING_INTEROP
        {
            return newVarString(ownerPanel, parent, name, access,
                                userDataPtr, userDataType, callbacks);
        }
    // C++ Boolean:
    case UserVar::Type::BoolVal :
        {
            return newVarBool(ownerPanel, parent, name, access,
                              userDataPtr, userDataType, callbacks);
        }
    // C/C++ Enum:
    case UserVar::Type::EnumVal :
        {
            return newVarEnum(ownerPanel, parent, name, access, userDataPtr,
                              userDataType, callbacks, extraData, extraDataSize);
        }
    default :
        {
            NTB_ERROR("Invalid variable type tag!");
            return NTB_NULL;
        }
    } // switch (userDataType)
}

} // namespace new_ {}

// --------------------------------------------------------------------

enum TestEnum
{
    TE_CONST1,
    TE_CONST2,
    TE_CONST3,
    TE_CONST4
};
const new_::EnumConstant testEnumConsts[] =
{
    new_::EnumTypeDecl<TestEnum>(),
    new_::EnumConstant("TE_CONST1", TE_CONST1),
    new_::EnumConstant("TE_CONST2", TE_CONST2),
    new_::EnumConstant("TE_CONST3", TE_CONST3),
    new_::EnumConstant("TE_CONST4", TE_CONST4)
};

enum class TestEnumClass
{
    Const1,
    Const2,
    Const3,
    Const4
};
const new_::EnumConstant testEnumClassConsts[] =
{
    new_::EnumTypeDecl<TestEnumClass>(),
    new_::EnumConstant("TestEnumClass::Const1", TestEnumClass::Const1),
    new_::EnumConstant("TestEnumClass::Const2", TestEnumClass::Const2),
    new_::EnumConstant("TestEnumClass::Const3", TestEnumClass::Const3),
    new_::EnumConstant("TestEnumClass::Const4", TestEnumClass::Const4)
};

struct Test : public ntb::ListNode
{
    int num;
     Test() { std::cout << " Test()" << std::endl; }
    ~Test() { std::cout << "~Test()" << std::endl; }

    const float * getFooPtr() const   { return NULL; }
    void setFooPtr(const float * foo) { }

    // by val:

    bool getBool() const { return false; }
    void setBool(bool b) { (void)b; }

    char getChar() const { return 0; }
    void setChar(char c) { (void)c; }

    const float & getFoo0() const   { static float foo; return foo; }
    void setFoo0(const float & foo) { }

    float getFoo1() const   { return 0; }
    void setFoo1(float foo) { }

    std::string getFoo2() const   { return ""; }
    void setFoo2(std::string foo) { }

    ntb::SmallStr getFoo3() const   { return ""; }
    void setFoo3(ntb::SmallStr foo) { }

    // by ptr:

    void getFoo4(float * fooOut) const { }
    void setFoo4(const float * fooIn)  { }

    void getFoo5(std::string * fooOut) const { }
    void setFoo5(const std::string * fooIn)  { }

    void getFoo6(ntb::SmallStr * fooOut) const { }
    void setFoo6(const ntb::SmallStr * fooIn)  { }

    void getFoo7(char * strOut) const { }
    void setFoo7(const char * strIn)  { }

    void getFoo8(ntb::Color32 * clrOut) const { }
    void setFoo8(const ntb::Color32 * clrIn)  { }

    void getVptr(void ** ptrOut) const { }
    void setVptr(void * const * ptrIn) { }
};

//------
static float g_float{42.0f};
void c_getFoo1(const void * userData, float * fooOut)
{
    *fooOut = g_float;
    printf("c_getFoo1: %f\n", g_float);
}
void c_setFoo1(void * userData, const float * fooIn)
{
    g_float = *fooIn;
    printf("c_setFoo1: %f\n", g_float);
}
//------
static std::string g_string{"Hello"};
void c_getFoo2(const Test * userData, std::string * fooOut)
{
    *fooOut = g_string;
    printf("c_getFoo2: %s\n", g_string.c_str());
}
void c_setFoo2(Test * userData, const std::string * fooIn)
{
    g_string = *fooIn;
    printf("c_setFoo2: %s\n", g_string.c_str());
}
//------
void c_getFoo3(const void * userData, char * fooOut) { }
void c_setFoo3(void * userData, const char * fooIn) { }
//------
void c_getVptr(const Test * userData, void ** ptrOut) { }
void c_setVptr(Test * userData, void * const * ptrIn) { }
//------

void print_sizes()
{
    #if defined(__GNUC__)
    std::cout << "__GNUC__ defined" << std::endl;
    #endif

    #if defined(__clang__)
    std::cout << "__clang__ defined" << std::endl;
    #endif

    std::cout << "sizeof(Widget)             = " << sizeof(ntb::Widget) << std::endl;
    std::cout << "sizeof(ListNode)           = " << sizeof(ntb::ListNode) << std::endl;
    std::cout << "sizeof(IntrusiveList)      = " << sizeof(ntb::IntrusiveList) << std::endl;
    std::cout << "sizeof(PODArray)           = " << sizeof(ntb::PODArray) << std::endl;
    std::cout << "sizeof(Point)              = " << sizeof(ntb::Point) << std::endl;
    std::cout << "sizeof(Rectangle)          = " << sizeof(ntb::Rectangle) << std::endl;
    std::cout << "sizeof(VarDisplayWidget)   = " << sizeof(ntb::VarDisplayWidget) << std::endl;
    std::cout << "sizeof(Variable)           = " << sizeof(ntb::Variable) << std::endl;
    std::cout << "sizeof(VarHierarchyParent) = " << sizeof(ntb::VarHierarchyParent) << std::endl;
    std::cout << "sizeof(View3DWidget)       = " << sizeof(ntb::View3DWidget) << std::endl;
    std::cout << "sizeof(Panel)              = " << sizeof(ntb::Panel) << std::endl;
    std::cout << "sizeof(GUI)                = " << sizeof(ntb::GUI) << std::endl;
    std::cout << "sizeof(EditField)          = " << sizeof(ntb::EditField) << std::endl;
    std::cout << "sizeof(std::string)        = " << sizeof(std::string) << std::endl;
    std::cout << "sizeof(SmallStr)           = " << sizeof(ntb::SmallStr) << std::endl;
    std::cout << "sizeof(UIString)           = " << sizeof(new_::UIString) << std::endl;
    std::cout << "sizeof(UIColor)            = " << sizeof(new_::UIColor) << std::endl;
    std::cout << "sizeof(UIF32Vec)           = " << sizeof(new_::UIF32Vec) << std::endl;
    std::cout << "sizeof(UINumber)           = " << sizeof(new_::UINumber) << std::endl;
    std::cout << "sizeof(UIBool)             = " << sizeof(new_::UIBool) << std::endl;
    std::cout << "sizeof(UIEnum)             = " << sizeof(new_::UIEnum) << std::endl;
    std::cout << "sizeof(UserVar)            = " << sizeof(new_::UserVar) << std::endl;
    std::cout << "sizeof(VarCallbacksAny)    = " << sizeof(new_::VarCallbacksAny) << std::endl;
}

void print_panel_vars(const ntb::Panel & panel)
{
    std::cout << "---- all vars from panel ----\n";
    panel.enumerateAllVariables(
            [](const ntb::Variable & var)
            {
                const char * ams[] = { "ReadOnly", "ReadWrite" };
                std::cout << "Var: '" << var.getName() << "' (" << ams[var.getAccessMode()] << ")\n";
            });
}

void print_gui_panels(const ntb::GUI & gui)
{
    std::cout << "---- all panels from GUI ----\n";
    gui.enumerateAllPanels(
            [](const ntb::Panel & panel)
            {
                std::cout << "Panel: '" << panel.getName() << "'\n";
            });
}

// ================================================================================================
//
//                                  main():
//
// ================================================================================================

int main()
{
    print_sizes();

    printf("\n");
    SmallStr s0("Hello World");
    s0.erase(0);
    s0.erase(s0.getLength() - 1);
    printf("s0: %s\n", s0.c_str());

    SmallStr s1("Hello*World");
    s1.erase(5);
    printf("s1: %s\n", s1.c_str());

    s0 = "Hello World";
    s0.insert(0, '#');
    s0.insert(s0.getLength() - 1, '#');
    printf("s0: %s\n", s0.c_str());

    s1 = "Hello*World";
    s1.insert(5, '#');
    printf("s1: %s\n", s1.c_str());

    run_glfw_test_app();

    /*
    Test test;
    void * vp = nullptr;

// INVALID
//    new_::callbacks(&test, &Test::getFooPtr);
//    new_::callbacks(&test, &Test::getFooPtr, &Test::setFooPtr);

    new_::callbacks(&test, &Test::getVptr);
    new_::callbacks(&test, &Test::getVptr, &Test::setVptr);

    new_::callbacks(&test, &c_getVptr);
    new_::callbacks(&test, &c_getVptr, &c_setVptr);

    new_::callbacks(&test, &Test::getBool);
    new_::callbacks(&test, &Test::getBool, &Test::setBool);

    new_::callbacks(&test, &Test::getChar);
    new_::callbacks(&test, &Test::getChar, &Test::setChar);

    new_::callbacks(&test, &Test::getFoo0);
    new_::callbacks(&test, &Test::getFoo0, &Test::setFoo0);

    new_::callbacks(&test, &Test::getFoo1);
    new_::callbacks(&test, &Test::getFoo1, &Test::setFoo1);

    new_::callbacks(&test, &Test::getFoo2);
    new_::callbacks(&test, &Test::getFoo2, &Test::setFoo2);

    new_::callbacks(&test, &Test::getFoo3);
    new_::callbacks(&test, &Test::getFoo3, &Test::setFoo3);

    new_::callbacks(&test, &Test::getFoo4);
    new_::callbacks(&test, &Test::getFoo4, &Test::setFoo4);

    new_::callbacks(&test, &Test::getFoo5);
    new_::callbacks(&test, &Test::getFoo5, &Test::setFoo5);

    new_::callbacks(&test, &Test::getFoo6);
    new_::callbacks(&test, &Test::getFoo6, &Test::setFoo6);

    new_::callbacks(&test, &Test::getFoo7);
    new_::callbacks(&test, &Test::getFoo7, &Test::setFoo7);

    new_::callbacks(&test, &Test::getFoo8);
    new_::callbacks(&test, &Test::getFoo8, &Test::setFoo8);

    new_::callbacks(vp, &c_getFoo1);
    new_::callbacks(vp, &c_getFoo1, &c_setFoo1);

    new_::callbacks(&test, &c_getFoo2);
    new_::callbacks(&test, &c_getFoo2, &c_setFoo2);

    new_::callbacks(vp, &c_getFoo3);
    new_::callbacks(vp, &c_getFoo3, &c_setFoo3);
    */

#if 0

    ntb::initialize(renderInterface, NTB_NULL, NTB_NULL);

    ntb::GUI * gui = ntb::createGUI("My Gooy");
    ntb::Panel * panel = gui->createPanel("Panel 1");

    bool b;
    panel->addBoolRO("b", &b);
    panel->addBoolRW("b", &b);
    char c;
    panel->addCharRO("c", &c);
    panel->addCharRW("c", &c);

    int ii;
    panel->addNumberRO("ii", &ii);
    panel->addNumberRW("ii", &ii);
    unsigned int ui;
    panel->addNumberRO("ui", &ui);
    panel->addNumberRW("ui", &ui);

    ntb::Int8 i8;
    panel->addNumberRO("i8", &i8);
    panel->addNumberRW("i8", &i8);
    ntb::Int16 i16;
    panel->addNumberRO("i16", &i16);
    panel->addNumberRW("i16", &i16);
    ntb::Int32 i32;
    panel->addNumberRO("i32", &i32);
    panel->addNumberRW("i32", &i32);
    ntb::Int64 i64;
    panel->addNumberRO("i64", &i64);
    panel->addNumberRW("i64", &i64);

    ntb::UInt8 u8;
    panel->addNumberRO("u8", &u8);
    panel->addNumberRW("u8", &u8);
    ntb::UInt16 u16;
    panel->addNumberRO("u16", &u16);
    panel->addNumberRW("u16", &u16);
    ntb::UInt32 u32;
    panel->addNumberRO("u32", &u32);
    panel->addNumberRW("u32", &u32);
    ntb::UInt64 u64;
    panel->addNumberRO("u64", &u64);
    panel->addNumberRW("u64", &u64);

    float ff;
    panel->addNumberRO("ff", &ff);
    panel->addNumberRW("ff", &ff);
    double dd;
    panel->addNumberRO("dd", &dd);
    panel->addNumberRW("dd", &dd);

    std::string stdstr;
    panel->addStringRO("stdstr", &stdstr);
    panel->addStringRW("stdstr", &stdstr);
    ntb::SmallStr ntbstr;
    panel->addStringRO("ntbstr", &ntbstr);
    panel->addStringRW("ntbstr", &ntbstr);

    char bufp[256];
    panel->addStringRO("bufp", bufp);
    panel->addStringRW<512>("bufp", bufp);

    float v4[4];
    panel->addFloatVecRO<4>("v4", v4);
    panel->addFloatVecRW<4>("v4", v4);

    float v3[3];
    panel->addDirectionVecRO("v3", v3);
    panel->addDirectionVecRW("v3", v3);

    float q4[4];
    panel->addRotationQuatRO("q4", q4);
    panel->addRotationQuatRW("q4", q4);

    ntb::UByte clrU8[3];
    panel->addColorRO<3>("clrU8", clrU8);
    panel->addColorRW<3>("clrU8", clrU8);

    float clrF32[4];
    panel->addColorRO<4>("clrF32", clrF32);
    panel->addColorRW<4>("clrF32", clrF32);

    ntb::Color32 clr32;
    panel->addColorRO("clr32", &clr32);
    panel->addColorRW("clr32", &clr32);

    TestEnum en;
    panel->addEnumRO("enum", &en, testEnumConsts, ntb::lengthOf(testEnumConsts));
    panel->addEnumRW("enum", &en, testEnumConsts, ntb::lengthOf(testEnumConsts));

    TestEnumClass enClass;
    panel->addEnumRO("enum", &enClass, testEnumClassConsts, ntb::lengthOf(testEnumClassConsts));
    panel->addEnumRW("enum", &enClass, testEnumClassConsts, ntb::lengthOf(testEnumClassConsts));

    void * pp = NTB_NULL;
    panel->addPointerRO("pp", &pp);
    panel->addPointerRW("pp", &pp);

    Test tst;

// INVALID
//    panel->addNumberRO("F00", &tst, ntb::callbacks(&Test::getFooPtr));
//    panel->addNumberRW("F00", &tst, ntb::callbacks(&Test::getFooPtr, &Test::setFooPtr));

    panel->addBoolRO("b1", &tst, ntb::callbacks(&Test::getBool));
    panel->addBoolRW("b1", &tst, ntb::callbacks(&Test::getBool, &Test::setBool));

    panel->addCharRO("c1", &tst, ntb::callbacks(&Test::getChar));
    panel->addCharRW("c1", &tst, ntb::callbacks(&Test::getChar, &Test::setChar));

    panel->addNumberRO("n0", &tst, ntb::callbacks(&Test::getFoo0));
    panel->addNumberRW("n0", &tst, ntb::callbacks(&Test::getFoo0, &Test::setFoo0));

    panel->addNumberRO("n1", &tst, ntb::callbacks(&Test::getFoo1));
    panel->addNumberRW("n1", &tst, ntb::callbacks(&Test::getFoo1, &Test::setFoo1));

    panel->addStringRO("s1", &tst, ntb::callbacks(&Test::getFoo2));
    panel->addStringRW("s1", &tst, ntb::callbacks(&Test::getFoo2, &Test::setFoo2));

    panel->addStringRO("s2", &tst, ntb::callbacks(&Test::getFoo3));
    panel->addStringRW("s2", &tst, ntb::callbacks(&Test::getFoo3, &Test::setFoo3));

    panel->addNumberRO("n2", &tst, ntb::callbacks(&Test::getFoo4));
    panel->addNumberRW("n2", &tst, ntb::callbacks(&Test::getFoo4, &Test::setFoo4));

    panel->addStringRO("s3", &tst, ntb::callbacks(&Test::getFoo5));
    panel->addStringRW("s3", &tst, ntb::callbacks(&Test::getFoo5, &Test::setFoo5));

    panel->addStringRO("s4", &tst, ntb::callbacks(&Test::getFoo6));
    panel->addStringRW("s4", &tst, ntb::callbacks(&Test::getFoo6, &Test::setFoo6));

    panel->addStringRO("s5", &tst, ntb::callbacks(&Test::getFoo7));
    panel->addStringRW("s5", &tst, ntb::callbacks(&Test::getFoo7, &Test::setFoo7));

    panel->addNumberRO("cf", &tst, ntb::callbacks(&c_getFoo1));
    panel->addNumberRW("cf", &tst, ntb::callbacks(&c_getFoo1, &c_setFoo1));

    panel->addStringRO("cs", &tst, ntb::callbacks(&c_getFoo2));
    panel->addStringRW("cs", &tst, ntb::callbacks(&c_getFoo2, &c_setFoo2));

    panel->addStringRO("cs2", &tst, ntb::callbacks(&c_getFoo3));
    panel->addStringRW("cs2", &tst, ntb::callbacks(&c_getFoo3, &c_setFoo3));

    panel->addFloatVecRO<2>("n2", &tst, ntb::callbacks(&Test::getFoo4));
    panel->addFloatVecRW<2>("n2", &tst, ntb::callbacks(&Test::getFoo4, &Test::setFoo4));

    panel->addColorRO<4>("n2", &tst, ntb::callbacks(&Test::getFoo8));
    panel->addColorRW<4>("n2", &tst, ntb::callbacks(&Test::getFoo8, &Test::setFoo8));

    {
        struct X
        {
            int a;
            float b;
            std::string c;
        } my_obj;

        ntb::Variable * structParent = panel->addHierarchyParent("my_obj");
        panel->addNumberRW(structParent, "a", &my_obj.a);
        panel->addNumberRO(structParent, "b", &my_obj.b);
        panel->addStringRW(structParent, "c", &my_obj.c);
    }
    {
        int numb = 42;
        ntb::Variable * var = panel->addNumberRW("numb", &numb);

        printf("numb: %d\n", numb);
        var->updateCachedValue();
        printf("numb: %d\n", numb);

        numb = 666;

        printf("numb: %d\n", numb);
        var->updateUserValue();
        printf("numb: %d\n", numb);
    }
    {
        char czstr[128] = "Hello World!";
        ntb::Variable * var = panel->addStringRW<128>("czstr", czstr);

        printf("czstr: %s\n", czstr);
        var->updateCachedValue();
        printf("czstr: %s\n", czstr);

        ntb::copyString(czstr, sizeof(czstr), "**** FOOBAR ****");

        printf("czstr: %s\n", czstr);
        var->updateUserValue();
        printf("czstr: %s\n", czstr);
    }
    {
        void * ptr = (void*)1;
        ntb::Variable * var = panel->addPointerRW("ptr", &ptr);

        printf("numb: %p\n", ptr);
        var->updateCachedValue();
        printf("numb: %p\n", ptr);

        ptr = (void*)2;

        printf("numb: %p\n", ptr);
        var->updateUserValue();
        printf("numb: %p\n", ptr);
    }

    //print_panel_vars(*panel);
    //print_gui_panels(*gui);

    ntb::shutdown();

#endif // 0
}
