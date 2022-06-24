
// ================================================================================================
// -*- C++ -*-
// File: sample_basic.cpp
// Author: Guilherme R. Lampert
// Created on: 17/06/2022
//
// Brief:
//  Basic sample using GUI, Panels and Variables.
//  Arguments:
//   --gl-core:   Runs in OpenGL Core Profile mode (GL 3+);
//   --gl-legacy: Runs in Legacy mode (OpenGL 2.0 or lower);
//  If no command line arguments are given, defaults to legacy mode.
// ================================================================================================

#include "ntb.hpp"
#include "sample_app_lib.hpp"

#include <string>
#include <cstdlib>

#if !defined(NEO_TWEAK_BAR_STD_STRING_INTEROP)
    #error "NEO_TWEAK_BAR_STD_STRING_INTEROP is required for this sample!"
#endif // NEO_TWEAK_BAR_STD_STRING_INTEROP

// ========================================================

enum class TestEnumClass
{
    Const1,
    Const2,
    Const3,
    Const4
};
static const ntb::EnumConstant testEnumConsts[] =
{
    ntb::EnumTypeDecl<TestEnumClass>(),
    ntb::EnumConstant("TestEnumClass::Const1", TestEnumClass::Const1),
    ntb::EnumConstant("TestEnumClass::Const2", TestEnumClass::Const2),
    ntb::EnumConstant("TestEnumClass::Const3", TestEnumClass::Const3),
    ntb::EnumConstant("TestEnumClass::Const4", TestEnumClass::Const4)
};

// ========================================================

static void myAppEventCallback(const AppEvent & event, void * userContext)
{
    auto gui = static_cast<ntb::GUI *>(userContext);

    switch (event.type)
    {
    case AppEvent::MouseMotion :
        gui->onMouseMotion(event.data.pos[0], event.data.pos[1]);
        break;

    case AppEvent::MouseScroll :
        gui->onMouseScroll(event.data.scroll[1]);
        break;

    case AppEvent::MouseClickLeft :
        gui->onMouseButton(ntb::MouseButton::Left, event.data.clicks);
        break;

    case AppEvent::MouseClickRight :
        gui->onMouseButton(ntb::MouseButton::Right, event.data.clicks);
        break;

    default :
        break;
    } // switch (event.type)
}

// ========================================================

int main(const int argc, const char * argv[])
{
    AppContext ctx;
    if (!appInit(argc, argv, "NTB Basic Sample", 1024, 768, &ctx))
    {
        std::fprintf(stderr, "[APP_ERROR]: Failed to initialize sample app!\n");
        return EXIT_FAILURE;
    }

    ntb::initialize(ctx.shellInterface, ctx.renderInterface);
    {
        ntb::GUI   * gui    = ntb::createGUI("Sample GUI");
        ntb::Panel * panel1 = gui->createPanel("Sample panel 1 (RW)");
        ntb::Panel * panel2 = gui->createPanel("Sample panel 2 (RO)");
        ntb::Panel * panel3 = gui->createPanel("Sample panel 3 (CB)");

        panel1->setPosition(10, 10)->setSize(500, 500);
        panel2->setPosition(600, 10)->setSize(500, 500);
        panel3->setPosition(10, 550)->setSize(400, 550);

        bool          b       = true;
        int           i       = 42;
        float         f       = 3.14f;
        TestEnumClass e       = TestEnumClass::Const1;
        const char *  s       = "the variable value";
        float         v[4]    = { 1.0f, 2.0f, 3.0f, 4.0f };
        unsigned char c[3]    = { 0, 128, 255 };
        char          buf[16] = "hello!";
        void *        ptr     = (void *)UINT64_C(0xCAFED00DDEADBEEF);
        ntb::Color32  c32     = ntb::packColor(255, 0, 0);
        float         quat[4] = { 1.1f, 1.2f, 1.3f, 1.4f };

        // Read-write variables
        auto var0 = panel1->addBoolRW("a boolean", &b);
        auto var1 = panel1->addFloatVecRW(var0, "a vec4", v, 4);
        auto var2 = panel1->addNumberRW("a float", &f);
        auto var3 = panel1->addStringRW(var2, "a writable str", buf, ntb::lengthOfArray(buf));
        auto var4 = panel1->addPointerRW(var2, "a ptr", &ptr);
        auto var5 = panel1->addNumberRW(var2, "an int", &i);

        // Read-only variables
        auto var6  = panel2->addNumberRO("an int", &i);
        auto var7  = panel2->addStringRO(var6, "a string", s);
        auto var8  = panel2->addPointerRO(var7, "a pointer", &ptr);
        auto var9  = panel2->addColorRO("a color", c, 3);
        auto var10 = panel2->addEnumRO(var9, "an enum", &e, testEnumConsts, ntb::lengthOfArray(testEnumConsts));
        auto var11 = panel2->addColorRO(var9, "a color32", &c32);
        auto var12 = panel2->addRotationQuatRO(var9, "a quaternion", quat);
        auto var13 = panel2->addBoolRO("a bool", &b);

        struct Test
        {
            bool          b      = false;
            int           i      = 1234;
            std::string   s      = "Test";
            ntb::Color32  c32    = ntb::packColor(255, 0, 255);
            float         c4f[4] = { 0.5f, 0.2f, 0.2f, 0.5f };
            std::uint8_t  c8b[4] = { 0, 255, 0, 255 };
            void *        p      = (void *)UINT64_C(0xCAFED00DDEADBEEF);
            char          ch     = 'X';
            char          cs[64] = "Hello again";
            TestEnumClass en     = TestEnumClass::Const2;

            bool getBoolVal() const { return b; }
            void setBoolVal(bool val) { b = val; }

            int getIntVal() const { return i; }
            void setIntVal(int val) { i = val; }

            const std::string & getStdStringRef() const { return s; }
            void setStdStringRef(const std::string & val) { s = val; }

            ntb::Color32 getColor32Val() const { return c32; }
            void setColor32Val(ntb::Color32 val) { c32 = val; }

            void getColor4FPtr(float outVal[4]) const { std::memcpy(outVal, c4f, sizeof(c4f)); }
            void setColor4FPtr(const float inVal[4])  { std::memcpy(c4f, inVal, sizeof(c4f)); }

            void getColor8BPtr(std::uint8_t outVal[4]) const { std::memcpy(outVal, c8b, sizeof(c8b)); }
            void setColor8BPtr(const std::uint8_t inVal[4])  { std::memcpy(c8b, inVal, sizeof(c8b)); }

            void getVoidPtr(void ** outVal) const { *outVal = p; }
            void setVoidPtr(void * const * inVal) { p = *inVal; }

            char getCharVal() const { return ch; }
            void setCharVal(char val) { ch = val; }

            void getCStringPtr(char * outVal) const { strncpy_s(outVal, sizeof(cs), cs, sizeof(cs)); }
            void setCStringPtr(const char * inVal)  { strcpy_s(cs, inVal); }

            TestEnumClass getEnumVal() const { return en; }
            void setEnumVal(TestEnumClass val) { en = val; }
        } testObj;

        const char ch = 'G';
        panel3->addCharRO("a char", &ch);

        const std::string str = "hello world";
        panel3->addStringRO("std string", &str);

        // Variables with callbacks
        panel3->addBoolRW("Test::b",    ntb::callbacks(&testObj, &Test::getBoolVal,      &Test::setBoolVal));
        panel3->addNumberRW("Test::i",  ntb::callbacks(&testObj, &Test::getIntVal,       &Test::setIntVal));
        panel3->addStringRW("Test::s",  ntb::callbacks(&testObj, &Test::getStdStringRef, &Test::setStdStringRef));
        panel3->addColorRW("Test::c32", ntb::callbacks(&testObj, &Test::getColor32Val,   &Test::setColor32Val), 4);
        panel3->addColorRW("Test::c4f", ntb::callbacks(&testObj, &Test::getColor4FPtr,   &Test::setColor4FPtr), 4);
        panel3->addColorRW("Test::c8b", ntb::callbacks(&testObj, &Test::getColor8BPtr,   &Test::setColor8BPtr), 4);
        panel3->addPointerRW("Test::p", ntb::callbacks(&testObj, &Test::getVoidPtr,      &Test::setVoidPtr));
        panel3->addCharRW("Test::ch",   ntb::callbacks(&testObj, &Test::getCharVal,      &Test::setCharVal));
        panel3->addStringRW("Test::cs", ntb::callbacks(&testObj, &Test::getCStringPtr,   &Test::setCStringPtr));
        panel3->addEnumRW("Test::en",   ntb::callbacks(&testObj, &Test::getEnumVal,      &Test::setEnumVal), testEnumConsts, ntb::lengthOfArray(testEnumConsts));

        // To forward window input events to the GUI.
        ctx.setAppCallback(&ctx, &myAppEventCallback, gui);

        bool done = false;
        while (!done)
        {
            ctx.frameUpdate(&ctx, &done);

            const bool forceRefresh = false;
            gui->onFrameRender(forceRefresh);

            ctx.framePresent(&ctx);
        }
    }
    ntb::shutdown(); // This will also free the GUI instance.
    ctx.shutdown(&ctx);
}
