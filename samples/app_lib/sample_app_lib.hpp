#pragma once
// ================================================================================================
// -*- C++ -*-
// File: sample_app_lib.hpp
// Author: Guilherme R. Lampert
// Created on: 25/09/16
// Brief: A tiny helper library just for the NTB samples.
// ================================================================================================

#include "ntb.hpp"

struct AppEvent
{
    enum Type
    {
        None = 0,

        // Mouse events:
        MouseMotion,
        MouseScroll,
        MouseClickLeft,
        MouseClickRight,

        // Number of entries; internal use only.
        Count
    };

    union Data
    {
        int clicks;
        int pos[2];
        int scroll[2];
    };

    Data data;
    Type type;
};

struct AppWindowHandle;
using AppEventCallback = void (*)(const AppEvent &, void *);

struct AppContext
{
    AppWindowHandle      * windowHandle;
    ntb::RenderInterface * renderInterface;
    ntb::ShellInterface  * shellInterface;

    int                    windowWidth;
    int                    windowHeight;
    int                    framebufferWidth;
    int                    framebufferHeight;
    bool                   isGlCoreProfile;

    void (*setAppCallback) (AppContext * ctx, AppEventCallback cb, void * userContext);
    void (*frameUpdate)    (AppContext * ctx, bool * outIsDone);
    void (*framePresent)   (AppContext * ctx);
    void (*shutdown)       (AppContext * ctx);
};

bool appInit(int argc, const char * argv[], const char * windowTitle,
             int windowWidth, int windowHeight, AppContext * outCtx);
