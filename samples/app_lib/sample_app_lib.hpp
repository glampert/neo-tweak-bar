
// ================================================================================================
// -*- C++ -*-
// File: sample_app_lib.hpp
// Author: Guilherme R. Lampert
// Created on: 25/09/16
// Brief: A tiny helper library just for the NTB samples.
// ================================================================================================

#ifndef NTB_SAMPLE_APP_LIB_HPP
#define NTB_SAMPLE_APP_LIB_HPP

#include "ntb.hpp"

struct AppWindowHandle;
struct AppContext
{
    AppWindowHandle      * windowHandle;
    ntb::RenderInterface * renderInterface;
    ntb::ShellInterface  * shellInterface;

    int                    windowWidth;
    int                    windowHeight;
    bool                   isGlCoreProfile;

    void (*frameUpdate)    (AppContext * ctx, bool * outIsDone);
    void (*framePresent)   (AppContext * ctx);
    void (*shutdown)       (AppContext * ctx);
};

bool appInit(int argc, const char * argv[], const char * windowTitle,
             int windowWidth, int windowHeight, AppContext * outCtx);

#endif // NTB_SAMPLE_APP_LIB_HPP
