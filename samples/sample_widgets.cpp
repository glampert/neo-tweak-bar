
// ================================================================================================
// -*- C++ -*-
// File: sample_widgets.cpp
// Author: Guilherme R. Lampert
// Created on: 02/10/16
//
// Brief:
//  Sample and testbed for the internal Widget types used by NTB.
//  Arguments:
//   --gl-core:   Runs in OpenGL Core Profile mode (GL 3+);
//   --gl-legacy: Runs in Legacy mode (OpenGL 2.0 or lower);
//  If no command line arguments are given, defaults to legacy mode.
// ================================================================================================

#include "ntb.hpp"
#include "ntb_widgets.hpp"
#include "sample_app_lib.hpp"

#include <string>
#include <cstdlib>

#if !defined(NEO_TWEAK_BAR_STD_STRING_INTEROP)
    #error "NEO_TWEAK_BAR_STD_STRING_INTEROP is required for this sample!"
#endif // NEO_TWEAK_BAR_STD_STRING_INTEROP

// ========================================================

static void myAppEventCallback(const AppEvent & event, void * userContext)
{
    auto widgets = static_cast<const ntb::PODArray *>(userContext);

    switch (event.type)
    {
    case AppEvent::MouseMotion :
        widgets->forEach<ntb::Widget *>(
            [](ntb::Widget * widget, const AppEvent * ev)
            {
                widget->onMouseMotion(ev->data.pos[0], ev->data.pos[1]);
                return true;
            }, &event);
        break;

    case AppEvent::MouseScroll :
        widgets->forEach<ntb::Widget *>(
            [](ntb::Widget * widget, const AppEvent * ev)
            {
                widget->onMouseScroll(ev->data.scroll[1]);
                return true;
            }, &event);
        break;

    case AppEvent::MouseClickLeft :
        widgets->forEach<ntb::Widget *>(
            [](ntb::Widget * widget, const AppEvent * ev)
            {
                widget->onMouseButton(ntb::MouseButton::Left, ev->data.clicks);
                return true;
            }, &event);
        break;

    case AppEvent::MouseClickRight :
        widgets->forEach<ntb::Widget *>(
            [](ntb::Widget * widget, const AppEvent * ev)
            {
                widget->onMouseButton(ntb::MouseButton::Right, ev->data.clicks);
                return true;
            }, &event);
        break;

    default :
        break;
    } // switch (event.type)
}

// ========================================================

struct MyButtonEventListener final
    : public ntb::ButtonWidget::EventListener
{
    bool onButtonDown(ntb::ButtonWidget & button) override;
};

bool MyButtonEventListener::onButtonDown(ntb::ButtonWidget & button)
{
    std::printf("Clicked button widget %p\n", reinterpret_cast<void *>(&button));
    return true;
}

// ========================================================

int main(const int argc, const char * argv[])
{
    AppContext ctx;
    if (!appInit(argc, argv, "NTB Widgets Test", 1024, 768, &ctx))
    {
        std::fprintf(stderr, "[APP_ERROR]: Failed to initialize sample app!\n");
        return EXIT_FAILURE;
    }

    ntb::initialize(ctx.shellInterface, ctx.renderInterface);
    {
        bool done = false;
        ntb::GeometryBatch geoBatch;
        ntb::PODArray widgets{ sizeof(ntb::Widget *) };
        ntb::GUI * gui = ntb::createGUI("Sample GUI");

        // Basic blank widget:
        {
            auto w = new ntb::Widget{};
            w->init(gui, nullptr, ntb::Rectangle{ 20, 20, 300, 300 }, true);
            widgets.pushBack(w);
        }

        // A set of buttons:
        {
            MyButtonEventListener buttonEventListener;
            const int buttonIconCount = static_cast<int>(ntb::ButtonWidget::Icon::Count);

            constexpr float btnScale = 1.6f;
            constexpr int btnSize = 50;
            constexpr int xStart  = 350;
            constexpr int yStart  = 20;

            int x = xStart;
            for (int i = 1; i < buttonIconCount; ++i) // Skip fist (Icon::None/0)
            {
                auto btn = new ntb::ButtonWidget{};
                btn->init(gui, nullptr, ntb::Rectangle{ x, yStart, x + btnSize, yStart + btnSize },
                          true, ntb::ButtonWidget::Icon(i), &buttonEventListener);

                btn->setTextScaling(btnScale);
                btn->setState(true);
                x += btnSize + 20; // gap between each (20)

                widgets.pushBack(btn);
            }
        }

        // Title bar & Info bar widgets:
        {
            constexpr int btnOffsX   = 20;
            constexpr int btnOffsY   = 4;
            constexpr int btnSize    = 40;
            constexpr int btnSpacing = 12;

            auto tb = new ntb::TitleBarWidget{};
            tb->init(gui, nullptr, ntb::Rectangle{ 350, 120, 900, 170 }, true,
                     "A title bar - drag me!", true, true, btnOffsX, btnOffsY, btnSize, btnSpacing);

            tb->setTextScaling(1.6f);       // Title bar text
            tb->setButtonTextScaling(1.5f); // Button icon text
            widgets.pushBack(tb);

            auto ib = new ntb::InfoBarWidget{};
            ib->init(gui, nullptr, ntb::Rectangle{ 350, 200, 900, 250 }, true, "Info bar");
            ib->setTextScaling(1.6f);
            widgets.pushBack(ib);
        }

        // List widget:
        {
            auto l = new ntb::ListWidget{};
            l->init(gui, nullptr, ntb::Rectangle{ 20, 350, 300, 500 }, true);
            l->setTextScaling(1.5f);

            l->allocEntries(4);
            l->addEntryText(0, "Hello");
            l->addEntryText(1, "World");
            l->addEntryText(2, "A longer string");
            l->addEntryText(3, "And this one is even longer");

            widgets.pushBack(l);
        }

        // Scrollbar widget:
        {
            auto sb = new ntb::ScrollBarWidget{};
            sb->init(gui, nullptr, ntb::Rectangle{ 550, 300, 600, 600 }, true, 30);
            sb->updateLineScrollState(10, 5);
            widgets.pushBack(sb);
        }

        // Color Picker widget:
        {
            constexpr int colorPickerWidth  = 360;
            constexpr int colorPickerHeight = 500;
            constexpr int xStart = 20;
            constexpr int yStart = 600;

            const ntb::Rectangle rect{ xStart, yStart, xStart + colorPickerWidth, yStart + colorPickerHeight };

            auto cp = new ntb::ColorPickerWidget{};
            cp->init(gui, nullptr, rect, true, 40, 28, 40, 25, 40);
            cp->setTextScaling(1.5f);
            cp->setButtonTextScaling(1.0f);

            widgets.pushBack(cp);
        }

        // 3D view widgets:
        {
            constexpr int view3dWidth  = 450;
            constexpr int view3dHeight = 500;
            constexpr int xStart = 500;
            constexpr int yStart = 650;

            ntb::View3DWidget::ProjectionParameters projParams;
            projParams.fovYRadians      = ntb::degToRad(60.0f);
            projParams.aspectRatio      = 0.0f; // auto computed
            projParams.zNear            = 0.5f;
            projParams.zFar             = 100.0f;
            projParams.autoAdjustAspect = true;

            const int objCount = static_cast<int>(ntb::View3DWidget::ObjectType::Count);

            int x = xStart;
            for (int i = 1; i < objCount; ++i)
            {
                const ntb::Rectangle rect{ x, yStart, x + view3dWidth, yStart + view3dHeight };

                auto v3d = new ntb::View3DWidget{};
                v3d->init(gui, nullptr, rect, true, "3D View Widget", 40, 28, 10, projParams, ntb::View3DWidget::ObjectType(i));
                v3d->setTextScaling(1.5f);
                v3d->setButtonTextScaling(1.0f);
                x += view3dWidth + 50;

                widgets.pushBack(v3d);
            }
        }

        // Var data display widgets inside a window/panel:
        {
            auto varWindow = new ntb::WindowWidget{};
            varWindow->init(gui, nullptr, ntb::Rectangle{ 1000, 20, 1500, 600 }, true, false, "Variables Test", 40, 28, 40, 25);
            varWindow->setTextScaling(1.5f);
            varWindow->setButtonTextScaling(1.0f);

            constexpr int varStartX = 1100;
            constexpr int varStartY = 90;
            constexpr int varWidth  = 300;
            constexpr int varHeight = 50;
            constexpr int varOffsY  = 8;

            ntb::Rectangle rect;
            int y = varStartY;

            auto var0 = new ntb::VarDisplayWidget{};
            rect.set(varStartX, y, varStartX + varWidth, y + varHeight); y += varHeight + varOffsY;
            var0->init(gui, nullptr, rect, true, varWindow, "Var 0");
            var0->setTextScaling(1.5f);
            var0->setButtonTextScaling(1.5f);

            auto var1 = new ntb::VarDisplayWidget{};
            rect.set(varStartX, y, varStartX + varWidth, y + varHeight); y += varHeight + varOffsY;
            var1->init(gui, var0, rect, true, varWindow, "Var 1");
            var1->setTextScaling(1.5f);

            auto var2 = new ntb::VarDisplayWidget{};
            rect.set(varStartX, y, varStartX + varWidth, y + varHeight); y += varHeight + varOffsY;
            var2->init(gui, var0, rect, true, varWindow, "Var 2");
            var2->setTextScaling(1.5f);

            // Change sizes so child vars look nested under the parent
            int cX = varStartX + var0->getExpandCollapseButtonSize();
            int cW = varWidth  - var0->getExpandCollapseButtonSize();

            auto var3 = new ntb::VarDisplayWidget{};
            rect.set(cX, y, cX + cW, y + varHeight); y += varHeight + varOffsY;
            var3->init(gui, var0, rect, true, varWindow, "Var 3");
            var3->setTextScaling(1.5f);
            var3->setButtonTextScaling(1.5f);

            auto var4 = new ntb::VarDisplayWidget{};
            rect.set(cX, y, cX + cW, y + varHeight); y += varHeight + varOffsY;
            var4->init(gui, var3, rect, true, varWindow, "Var 4");
            var4->setTextScaling(1.5f);

            cX += var0->getExpandCollapseButtonSize();
            cW -= var0->getExpandCollapseButtonSize();

            auto var5 = new ntb::VarDisplayWidget{};
            rect.set(cX, y, cX + cW, y + varHeight); y += varHeight + varOffsY;
            var5->init(gui, var3, rect, true, varWindow, "Var 5");
            var5->setTextScaling(1.5f);
            var5->setButtonTextScaling(1.5f);

            auto var6 = new ntb::VarDisplayWidget{};
            rect.set(cX, y, cX + cW, y + varHeight); y += varHeight + varOffsY;
            var6->init(gui, var5, rect, true, varWindow, "Var 6");
            var6->setTextScaling(1.5f);

            auto var7 = new ntb::VarDisplayWidget{};
            rect.set(cX, y, cX + cW, y + varHeight); y += varHeight + varOffsY;
            var7->init(gui, var5, rect, true, varWindow, "Var 7");
            var7->setTextScaling(1.5f);

            #if NEO_TWEAK_BAR_DEBUG
            varWindow->printHierarchy();
            std::cout << "\n";
            #endif // NEO_TWEAK_BAR_DEBUG

            // Only have to add the window, since each var widget is a child, directly or indirectly.
            widgets.pushBack(varWindow);
        }

        // Console/terminal window:
        {
            constexpr int maxLines   = 1024;
            constexpr int bufferSize = 2048;

            auto con = new ntb::ConsoleWindowWidget{};
            con->init(gui, nullptr, ntb::Rectangle{ 1550, 20, 2000, 420 }, true, true,
                      "Console Window", 40, 28, 40, 25, maxLines, bufferSize);

            con->setTextScaling(1.3f);
            con->setButtonTextScaling(1.0f);

            ntb::SmallStr line;
            for (ntb::Int64 i = 0; i < 15; ++i)
            {
                line = "Test line ";
                line += ntb::SmallStr::fromNumber(i);
                con->pushLine(line.c_str(), line.getLength());
            }
            con->onAdjustLayout(); // Update the scroll bar for lines out of view

            widgets.pushBack(con);
        }

        // To forward window input events to the widget list.
        ctx.setAppCallback(&ctx, &myAppEventCallback, &widgets);

        while (!done)
        {
            ctx.frameUpdate(&ctx, &done);
            geoBatch.beginDraw();

            // Slider helper (not an actual widget, but used by some widgets):
            {
                static ntb::Float64 sliderPercent = 0.0;

                ntb::ValueSlider slider;
                slider.setRange(0, 100);
                slider.setCurrentValue(sliderPercent);

                slider.drawSelf(geoBatch, ntb::Rectangle{ 650, 350, 950, 400 },
                                ntb::packColor(255, 255, 255), ntb::packColor(255, 100, 0));

                slider.drawSelf(geoBatch, ntb::Rectangle{ 650, 450, 950, 500 },
                                ntb::packColor(255, 255, 255), ntb::packColor(0, 200, 200));

                sliderPercent += 0.2;
                if (sliderPercent > 100.0)
                {
                    sliderPercent = 0.0;
                }
            }

            // Render our widgets:
            widgets.forEach<ntb::Widget *>(
                [](ntb::Widget * widget, ntb::GeometryBatch * batch)
                {
                    widget->onDraw(*batch);
                    return true;
                }, &geoBatch);

            geoBatch.endDraw();
            ctx.framePresent(&ctx);
        }

        widgets.forEach<ntb::Widget *>(
            [](ntb::Widget * widget, void * /*unused*/)
            {
                delete widget;
                return true;
            }, nullptr);
    }
    ctx.shutdown(&ctx);
    ntb::shutdown(); // This will also free the GUI instance.
}

