
// ================================================================================================
// -*- C++ -*-
// File: sample_null_renderer.cpp
// Author: Guilherme R. Lampert
// Created on: 26/07/16
//
// Brief:
//  Minimal NTB usage sample that does nothing (with a null renderer and null shell).
//  The goal of this sample is to make sure the different types of variable callbacks
//  and Panel::addXYZ() overloads will compile as expected.
// ================================================================================================

#include "ntb.hpp"
#include <string>

#if !defined(NEO_TWEAK_BAR_STD_STRING_INTEROP)
    #error "NEO_TWEAK_BAR_STD_STRING_INTEROP is required for this sample!"
#endif // NEO_TWEAK_BAR_STD_STRING_INTEROP

// ========================================================

class MyNTBShellInterfaceNull : public ntb::ShellInterface
{
public:
    ~MyNTBShellInterfaceNull();
    std::int64_t getTimeMilliseconds() const override { return 0; }
};
MyNTBShellInterfaceNull::~MyNTBShellInterfaceNull()
{ }

// ========================================================

class MyNTBRenderInterfaceNull : public ntb::RenderInterface
{
public:
    ~MyNTBRenderInterfaceNull();
};
MyNTBRenderInterfaceNull::~MyNTBRenderInterfaceNull()
{ }

// ========================================================

//
// Enums to test user-defined enum variables:
//
enum TestEnum
{
    TE_CONST1,
    TE_CONST2,
    TE_CONST3,
    TE_CONST4
};
static const ntb::EnumConstant testEnumConsts[] =
{
    ntb::EnumTypeDecl<TestEnum>(),
    ntb::EnumConstant("TE_CONST1", TE_CONST1),
    ntb::EnumConstant("TE_CONST2", TE_CONST2),
    ntb::EnumConstant("TE_CONST3", TE_CONST3),
    ntb::EnumConstant("TE_CONST4", TE_CONST4)
};

enum class TestEnumClass
{
    Const1,
    Const2,
    Const3,
    Const4
};
static const ntb::EnumConstant testEnumClassConsts[] =
{
    ntb::EnumTypeDecl<TestEnumClass>(),
    ntb::EnumConstant("TestEnumClass::Const1", TestEnumClass::Const1),
    ntb::EnumConstant("TestEnumClass::Const2", TestEnumClass::Const2),
    ntb::EnumConstant("TestEnumClass::Const3", TestEnumClass::Const3),
    ntb::EnumConstant("TestEnumClass::Const4", TestEnumClass::Const4)
};

//
// Methods to test the variable callback:
//
struct Test
{
    // Dummy data:
    bool        b;
    char        c;
    int         i;
    float       f;
    std::string s;
    void *      p;

    //
    // By value:
    //

    bool getBoolVal() const { return b; }
    void setBoolVal(bool val) { b = val; }

    char getCharVal() const { return c; }
    void setCharVal(char val) { c = val; }

    int getIntVal() const { return i; }
    void setIntVal(int val) { i = val; }

    float getFloatVal() const { return f; }
    void setFloatVal(float val) { f = val; }

    std::string getStdStringVal() const { return s; }
    void setStdStringVal(std::string val) { s = val; }

    //
    // By reference:
    //

    const bool & getBoolRef() const { return b; }
    void setBoolRef(const bool & val) { b = val; }

    const char & getCharRef() const { return c; }
    void setCharRef(const char & val) { c = val; }

    const int & getIntRef() const { return i; }
    void setIntRef(const int & val) { i = val; }

    const float & getFloatRef() const { return f; }
    void setFloatRef(const float & val) { f = val; }

    const std::string & getStdStringRef() const { return s; }
    void setStdStringRef(const std::string & val) { s = val; }

    //
    // By pointer:
    //

    void getBoolPtr(bool * outVal) const { *outVal = b; }
    void setBoolPtr(const bool * inVal) { b = *inVal; }

    void getIntPtr(int * outVal) const { *outVal = i; }
    void setIntPtr(const int * inVal) { i = *inVal; }

    void getFloatPtr(float * outVal) const { *outVal = f; }
    void setFloatPtr(const float * inVal) { f = *inVal; }

    void getCharPtr(char * outVal) const { *outVal = c; }
    void setCharPtr(const char * inVal) { c = *inVal; }

    void getStdStringPtr(std::string * outVal) const { *outVal = s; }
    void setStdStringPtr(const std::string * inVal) { s = *inVal; }

    void getVoidPtr(void ** outVal) const { *outVal = p; }
    void setVoidPtr(void * const * inVal) { p = *inVal; }

    //
    // Special cases:
    //

    // Return by pointer (invalid as a var callback).
    const float * badGetFloatPtr() const { return &f; }

    // Setter with non-void return type, OK.
    long getLongValue() const { return i; }
    bool setLongValue(long newValue) { i = int(newValue); return true; }

    // Enum type, OK.
    TestEnumClass getEnumVal() const { return TestEnumClass(i); }
    void setEnumVal(TestEnumClass val) { i = int(val); }
};

//
// C-style variable callbacks:
//
static float  g_float;
static bool   g_bool;
static void * g_ptr;

static void c_getFloat(const void * /*userData*/, float * outVal)
{
    *outVal = g_float;
}
static void c_setFloat(void * /*userData*/, const float * inVal)
{
    g_float = *inVal;
}

static void c_getBool(const void * /*userData*/, bool * outVal)
{
    *outVal = g_bool;
}
static void c_setBool(void * /*userData*/, const bool * inVal)
{
    g_bool = *inVal;
}

static void c_getVoidPtr(const void * /*userData*/, void ** outPtr)
{
    *outPtr = g_ptr;
}
static void c_setVoidPtr(void * /*userData*/, void * const * inPtr)
{
    g_ptr = *inPtr;
}

// ========================================================

int main()
{
    MyNTBShellInterfaceNull  shellInterface;
    MyNTBRenderInterfaceNull renderInterface;

    ntb::initialize(&shellInterface, &renderInterface);

    ntb::GUI   * gui    = ntb::createGUI("Null GUI");
    ntb::Panel * panel1 = gui->createPanel("Null panel 1");
    ntb::Panel * panel2 = gui->createPanel("Null panel 2");

    //
    // Direct pointers to variables:
    //
    bool          b    = true;
    int           i    = 42;
    float         f    = 3.14f;
    TestEnum      e    = TE_CONST1;
    const char *  s    = "the variable value";
    float         v[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
    unsigned char c[3] = { 0, 128, 255 };
    char        buf[8] = { 0 };

    // Read-write
    panel1->addBoolRW("a boolean", &b);
    panel1->addNumberRW("a float", &f);
    panel1->addFloatVecRW("a vec4", v, 4);
    panel1->addStringRW("a writable str", buf, ntb::lengthOfArray(buf));

    // Read-only
    panel2->addNumberRO("an int", &i);
    panel2->addStringRO("a string", s);
    panel2->addColorRO("a color", c, 3);
    panel2->addEnumRO("an enum", &e, testEnumConsts, ntb::lengthOfArray(testEnumConsts));

    //
    // Var callbacks:
    //
    Test obj;
    void * vpObj = &obj;

    // By value:
    panel1->addBoolRW("a",   ntb::callbacks(&obj, &Test::getBoolVal,      &Test::setBoolVal));
    panel1->addCharRW("b",   ntb::callbacks(&obj, &Test::getCharVal,      &Test::setCharVal));
    panel1->addNumberRW("c", ntb::callbacks(&obj, &Test::getIntVal,       &Test::setIntVal));
    panel1->addNumberRW("d", ntb::callbacks(&obj, &Test::getFloatVal,     &Test::setFloatVal));
    panel1->addStringRW("e", ntb::callbacks(&obj, &Test::getStdStringVal, &Test::setStdStringVal));

    // By reference:
    panel1->addBoolRW("f",   ntb::callbacks(&obj, &Test::getBoolRef,      &Test::setBoolRef));
    panel1->addCharRW("g",   ntb::callbacks(&obj, &Test::getCharRef,      &Test::setCharRef));
    panel1->addNumberRW("h", ntb::callbacks(&obj, &Test::getIntRef,       &Test::setIntRef));
    panel1->addNumberRW("i", ntb::callbacks(&obj, &Test::getFloatRef,     &Test::setFloatRef));
    panel1->addStringRW("j", ntb::callbacks(&obj, &Test::getStdStringRef, &Test::setStdStringRef));

    // By pointer:
    panel1->addBoolRW("k",   ntb::callbacks(&obj, &Test::getBoolPtr,      &Test::setBoolPtr));
    panel1->addCharRW("l",   ntb::callbacks(&obj, &Test::getCharPtr,      &Test::setCharPtr));
    panel1->addNumberRW("m", ntb::callbacks(&obj, &Test::getIntPtr,       &Test::setIntPtr));
    panel1->addNumberRW("n", ntb::callbacks(&obj, &Test::getFloatPtr,     &Test::setFloatPtr));
    panel1->addStringRW("o", ntb::callbacks(&obj, &Test::getStdStringPtr, &Test::setStdStringPtr));

    // The setter callback return bool (unlike the defaults that return void).
    panel2->addNumberRO("l1", ntb::callbacks(&obj, &Test::getLongValue));
    panel2->addNumberRW("l2", ntb::callbacks(&obj, &Test::getLongValue, &Test::setLongValue));

    // Enum class with list of allowed constants.
    panel2->addEnumRO("e1", ntb::callbacks(&obj, &Test::getEnumVal),
                      testEnumClassConsts, ntb::lengthOfArray(testEnumClassConsts));
    panel2->addEnumRW("e2", ntb::callbacks(&obj, &Test::getEnumVal, &Test::setEnumVal),
                      testEnumClassConsts, ntb::lengthOfArray(testEnumClassConsts));

    // Read-write C-style callbacks.
    panel2->addBoolRW("b1",    ntb::callbacks(vpObj, &c_getBool,    &c_setBool));
    panel2->addNumberRW("f1",  ntb::callbacks(vpObj, &c_getFloat,   &c_setFloat));
    panel2->addPointerRW("v1", ntb::callbacks(vpObj, &c_getVoidPtr, &c_setVoidPtr));

    // Read-only C-style callbacks.
    panel2->addBoolRO("b2",    ntb::callbacks(vpObj, &c_getBool));
    panel2->addNumberRO("f2",  ntb::callbacks(vpObj, &c_getFloat));
    panel2->addPointerRO("v2", ntb::callbacks(vpObj, &c_getVoidPtr));

    // Getter function returns a pointer -- invalid.
    // error: static_assert failed "Variable cannot be a pointer for this type of callbacks!"
    //panel2->addNumberRO("p", ntb::callbacks(&obj, &Test::badGetFloatPtr));

    // All GUIs are destroyed, also freeing any panels and variables linked to them.
    ntb::shutdown();
}

