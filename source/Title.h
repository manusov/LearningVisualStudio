/* ----------------------------------------------------------------------------------------
Application title strings definitions.
---------------------------------------------------------------------------------------- */

#pragma once
#ifndef TITLE_H
#define TITLE_H

// Select emulated or system scan mode: uncomment this for debug.
// Emulated mode means use constant emulated system configuration info
// WITHOUT get system information.
// Two lists (device list view or resource map view) contains
// different tree levels count if emulated mode used.
// #define _EMULATED_MODE

// Select GUI implementation version: uncomment this for v2.
// v1 use vertical and horizontal scroll bars as window properties.
// v2 use vertical and horizontal scroll bars as separate elements.
// v2 is UNDER CONSTRUCTION.
#define _GUI_V2

#define BUILD_VERSION  "0.12.2.0"
#define BUILD_NAME_MAX 160

#if _WIN64
#ifndef _EMULATED_MODE
#define RESOURCE_DESCRIPTION  "System view. Win64 edition."
#define RESOURCE_VERSION      BUILD_VERSION
#define RESOURCE_COMPANY      "https://github.com/manusov"
#define RESOURCE_COPYRIGHT    "(C) 2023 Ilya Manusov."
#define PROGRAM_NAME_TEXT     "System view for Win64."
#define ABOUT_TEXT_1          "System view."
#define ABOUT_TEXT_2_1        "v"
#define ABOUT_TEXT_2_2        BUILD_VERSION
#define ABOUT_TEXT_2_3        "."
#define ABOUT_TEXT_2_4        "x64."
#define ABOUT_TEXT_3          RESOURCE_COPYRIGHT
#else
#define RESOURCE_DESCRIPTION  "System view. Win64 edition. Emulated mode for debug."
#define RESOURCE_VERSION      BUILD_VERSION
#define RESOURCE_COMPANY      "https://github.com/manusov"
#define RESOURCE_COPYRIGHT    "(C) 2023 Ilya Manusov."
#define PROGRAM_NAME_TEXT     "System view for Win64."
#define ABOUT_TEXT_1          "System view. Emulated mode."
#define ABOUT_TEXT_2_1        "v"
#define ABOUT_TEXT_2_2        BUILD_VERSION
#define ABOUT_TEXT_2_3        "."
#define ABOUT_TEXT_2_4        "x64."
#define ABOUT_TEXT_3          RESOURCE_COPYRIGHT
#endif
#elif _WIN32
#ifndef _EMULATED_MODE
#define RESOURCE_DESCRIPTION  "System view. Win32 edition."
#define RESOURCE_VERSION      BUILD_VERSION
#define RESOURCE_COMPANY      "https://github.com/manusov"
#define RESOURCE_COPYRIGHT    "(C) 2023 Ilya Manusov."
#define PROGRAM_NAME_TEXT     "System view for Win32."
#define ABOUT_TEXT_1          "System view."
#define ABOUT_TEXT_2_1        "v"
#define ABOUT_TEXT_2_2        BUILD_VERSION
#define ABOUT_TEXT_2_3        "."
#define ABOUT_TEXT_2_4        "ia32."
#define ABOUT_TEXT_3          RESOURCE_COPYRIGHT
#else
#define RESOURCE_DESCRIPTION  "System view. Win32 edition. Emulated mode for debug."
#define RESOURCE_VERSION      BUILD_VERSION
#define RESOURCE_COMPANY      "https://github.com/manusov"
#define RESOURCE_COPYRIGHT    "(C) 2023 Ilya Manusov."
#define PROGRAM_NAME_TEXT     "System view for Win32."
#define ABOUT_TEXT_1          "System view. Emulated mode."
#define ABOUT_TEXT_2_1        "v"
#define ABOUT_TEXT_2_2        BUILD_VERSION
#define ABOUT_TEXT_2_3        "."
#define ABOUT_TEXT_2_4        "ia32."
#define ABOUT_TEXT_3          RESOURCE_COPYRIGHT
#endif
#else
#define RESOURCE_DESCRIPTION  "UNKNOWN BUILD MODE."
#define RESOURCE_VERSION      BUILD_VERSION
#define RESOURCE_COMPANY      "https://github.com/manusov"
#define RESOURCE_COPYRIGHT    "(C) 2023 Ilya Manusov."
#define PROGRAM_NAME_TEXT     "UNKNOWN BUILD MODE."
#define ABOUT_TEXT_1          "System view."
#define ABOUT_TEXT_2_1        "UNKNOWN BUILD MODE. v"
#define ABOUT_TEXT_2_2        BUILD_VERSION
#define ABOUT_TEXT_2_3        "."
#define ABOUT_TEXT_2_4        "?"
#define ABOUT_TEXT_3          RESOURCE_COPYRIGHT
#endif

#endif  // TITLE_H
