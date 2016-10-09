
// ================================================================================================
// -*- C++ -*-
// File: sample_widgets_gl.cpp
// Author: Guilherme R. Lampert
// Created on: 02/10/16
// Brief: Sample and testbed for the internal Widget types used by NTB.
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
    if (!appInit(argc, argv, "NTB Widgets Tests", 1024, 768, &ctx))
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

        // Basic widget:
        {
            auto w = new ntb::Widget{};
            w->init(gui, nullptr, ntb::Rectangle{ 20, 20, 300, 300 }, true);
            widgets.pushBack(w);
        }

        // A set of buttons:
        {
            MyButtonEventListener buttonEventListener;
            const int buttonIconCount = static_cast<int>(ntb::ButtonWidget::Icon::Count);

            constexpr float btnScale  = 1.6f;
            constexpr int   btnSize   = 50;
            constexpr int   btnStartX = 350;
            constexpr int   btnStartY = 20;

            int x = btnStartX;
            for (int i = 1; i < buttonIconCount; ++i) // Skip fist (Icon::None/0)
            {
                auto btn = new ntb::ButtonWidget{};
                btn->init(gui, nullptr, ntb::Rectangle{ x, btnStartY, x + btnSize, btnStartY + btnSize },
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

        // To forward window input events to the widget list.
        ctx.setAppCallback(&ctx, &myAppEventCallback, &widgets);

        while (!done)
        {
            ctx.frameUpdate(&ctx, &done);
            geoBatch.beginDraw();

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

