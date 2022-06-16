
// ================================================================================================
// -*- C++ -*-
// File: sample_app_lib.cpp
// Author: Guilherme R. Lampert
// Created on: 25/09/16
// Brief: Sample helper lib common.
// ================================================================================================

#include "sample_app_lib.hpp"

#include <string>
#include <cstdio>
#include <cstring>

#if !defined(NEO_TWEAK_BAR_STD_STRING_INTEROP)
    #error "NEO_TWEAK_BAR_STD_STRING_INTEROP is required for the samples!"
#endif // NEO_TWEAK_BAR_STD_STRING_INTEROP

// ========================================================

extern bool appGlCoreInit(const int glVersionMajor, const int glVersionMinor, const char * const windowTitle,
                          const int windowWidth, const int windowHeight, AppContext * outCtx);

extern bool appGlLegacyInit(const int glVersionMajor, const int glVersionMinor, const char * const windowTitle,
                            const int windowWidth, const int windowHeight, AppContext * outCtx);

// ========================================================

bool appInit(int argc, const char * argv[], const char * windowTitle,
             int windowWidth, int windowHeight, AppContext * outCtx)
{
    std::memset(outCtx, 0, sizeof(*outCtx));

    // Defaults to legacy profile.
    bool coreProfile   = false;
    int glVersionMajor = 2;
    int glVersionMinor = 0;
    std::string fullTitle = windowTitle;

    for (int i = 0; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--gl-core") == 0)
        {
            coreProfile     = true;
            glVersionMajor  = 3;
            glVersionMinor  = 2;
            fullTitle      += " - Core OpenGL";
        }
        else if (std::strcmp(argv[i], "--gl-legacy") == 0)
        {
            coreProfile     = false;
            glVersionMajor  = 2;
            glVersionMinor  = 0;
            fullTitle      += " - Legacy OpenGL";
        }
        // Command line override for window size:
        else if (std::strncmp(argv[i], "--window-width=", std::strlen("--window-width=")) == 0)
        {
            int value = 0;
            if (std::sscanf(argv[i], "--window-width=%i", &value) == 1)
            {
                windowWidth = value;
            }
        }
        else if (std::strncmp(argv[i], "--window-height=", std::strlen("--window-height=")) == 0)
        {
            int value = 0;
            if (std::sscanf(argv[i], "--window-height=%i", &value) == 1)
            {
                windowHeight = value;
            }
        }
        else if (std::strcmp(argv[i], "--help") == 0)
        {
            std::printf("\nUsage:\n  $ %s [--gl-code | --gl-legacy | --help]\n", argv[0]);
        }
    }

    std::printf("\nNTB sample \"%s\" starting up...\n", windowTitle);

    bool success;
    if (coreProfile)
    {
        success = appGlCoreInit(glVersionMajor, glVersionMinor, windowTitle,
                                windowWidth, windowHeight, outCtx);
    }
    else
    {
        success = appGlLegacyInit(glVersionMajor, glVersionMinor, windowTitle,
                                  windowWidth, windowHeight, outCtx);
    }

    std::printf("Done!\n\n");
    return success;
}
