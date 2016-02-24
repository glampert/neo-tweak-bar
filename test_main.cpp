
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

// --------------------------------------------------------------------

enum TestEnum
{
    TE_CONST1,
    TE_CONST2,
    TE_CONST3,
    TE_CONST4
};
const ntb::EnumConstant<TestEnum> testEnumConsts[] =
{
    { "TE_CONST1", TE_CONST1 },
    { "TE_CONST2", TE_CONST2 },
    { "TE_CONST3", TE_CONST3 },
    { "TE_CONST4", TE_CONST4 }
};

enum class TestEnumClass
{
    Const1,
    Const2,
    Const3,
    Const4
};
const ntb::EnumConstant<TestEnumClass> testEnumClassConsts[] =
{
    { "TestEnumClass::Const1", TestEnumClass::Const1 },
    { "TestEnumClass::Const2", TestEnumClass::Const2 },
    { "TestEnumClass::Const3", TestEnumClass::Const3 },
    { "TestEnumClass::Const4", TestEnumClass::Const4 }
};

struct Test : public ntb::ListNode
{
    int num;
     Test() { std::cout << " Test()" << std::endl; }
    ~Test() { std::cout << "~Test()" << std::endl; }

    // NOTE this has to fail compilation!
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
void c_getFoo3(const void * userData, char * fooOut)
{ }
void c_setFoo3(void * userData, const char * fooIn)
{ }
//------

void print_sizes()
{
    std::cout << "sizeof(Widget)             = " << sizeof(ntb::Widget) << std::endl;
    std::cout << "sizeof(ListNode)           = " << sizeof(ntb::ListNode) << std::endl;
    std::cout << "sizeof(IntrusiveList)      = " << sizeof(ntb::IntrusiveList) << std::endl;
    std::cout << "sizeof(PODArray)           = " << sizeof(ntb::PODArray) << std::endl;
    std::cout << "sizeof(SmallStr)           = " << sizeof(ntb::SmallStr) << std::endl;
    std::cout << "sizeof(Point)              = " << sizeof(ntb::Point) << std::endl;
    std::cout << "sizeof(Rectangle)          = " << sizeof(ntb::Rectangle) << std::endl;
    std::cout << "sizeof(VarDisplayWidget)   = " << sizeof(ntb::VarDisplayWidget) << std::endl;
    std::cout << "sizeof(Variable)           = " << sizeof(ntb::Variable) << std::endl;
    std::cout << "sizeof(VarHierarchyParent) = " << sizeof(ntb::detail::VarHierarchyParent) << std::endl;
    std::cout << "sizeof(View3DWidget)       = " << sizeof(ntb::View3DWidget) << std::endl;
    std::cout << "sizeof(Panel)              = " << sizeof(ntb::Panel) << std::endl;
    std::cout << "sizeof(GUI)                = " << sizeof(ntb::GUI) << std::endl;
    std::cout << "sizeof(EditField)          = " << sizeof(ntb::EditField) << std::endl;
    std::cout << "sizeof(ColorEx)            = " << sizeof(ntb::detail::ColorEx) << std::endl;
    std::cout << "sizeof(Float4Ex)           = " << sizeof(ntb::detail::Float4Ex) << std::endl;
    std::cout << "sizeof(NumberEx)           = " << sizeof(ntb::detail::NumberEx) << std::endl;
    std::cout << "sizeof(BoolEx)             = " << sizeof(ntb::detail::BoolEx) << std::endl;
}

void print_panel_vars(const ntb::Panel & panel)
{
    std::cout << "---- all vars from panel ----\n";
    panel.enumerateAllVariables(
            [](const ntb::Variable & var)
            {
                const char * ams[] = { "ReadOnly", "ReadWrite" };
                std::cout << "Var: '" << var.getName().c_str() << "' (" << ams[var.getAccessMode()] << ")\n";
            });
}

void print_gui_panels(const ntb::GUI & gui)
{
    std::cout << "---- all panels from GUI ----\n";
    gui.enumerateAllPanels(
            [](const ntb::Panel & panel)
            {
                std::cout << "Panel: '" << panel.getName().c_str() << "'\n";
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
    run_glfw_test_app();

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

        ntb::detail::copyString(czstr, sizeof(czstr), "**** FOOBAR ****");

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
