
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
        ntb::Panel * panel1 = gui->createPanel("Sample panel 1");
        ntb::Panel * panel2 = gui->createPanel("Sample panel 2");

        panel1->setPosition(10, 10)->setSize(500, 500);
        panel2->setPosition(600, 10)->setSize(400, 500);;

        bool          b       = true;
        int           i       = 42;
        float         f       = 3.14f;
        TestEnumClass e       = TestEnumClass::Const1;
        const char *  s       = "the variable value";
        float         v[4]    = { 1.0f, 2.0f, 3.0f, 4.0f };
        unsigned char c[3]    = { 0, 128, 255 };
        char          buf[16] = { 0 };
        const void *  ptr     = (void *)UINT64_C(0xDEADBEEF);
        ntb::Color32 c32      = ntb::packColor(255, 0, 0);
        float        quat[4]  = { 1.0f, 1.0f, 1.0f, 1.0f };

        // Read-write variables
        auto var0 = panel1->addBoolRW("a boolean", &b);
        auto var1 = panel1->addFloatVecRW(var0, "a vec4", v, 4);
        auto var2 = panel1->addNumberRW("a float", &f);
        auto var3 = panel1->addStringRW(var2, "a writable str", buf, ntb::lengthOfArray(buf));

        // Read-only variables
        auto var4  = panel2->addNumberRO("an int", &i);
        auto var5  = panel2->addStringRO(var4, "a string", s);
        auto var6  = panel2->addPointerRO(var5, "a pointer", &ptr);
        auto var7  = panel2->addColorRO("a color", c, 3);
        auto var8  = panel2->addEnumRO(var7, "an enum", &e, testEnumConsts, ntb::lengthOfArray(testEnumConsts));
        auto var9  = panel2->addColorRO(var7, "a color32", &c32);
        auto var10 = panel2->addRotationQuatRO(var9, "a quaternion", quat);

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
