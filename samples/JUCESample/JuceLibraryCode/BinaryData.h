/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   FREQIMG_png;
    const int            FREQIMG_pngSize = 310;

    extern const char*   Amount2_png;
    const int            Amount2_pngSize = 296007;

    extern const char*   CC_Backplate2_png;
    const int            CC_Backplate2_pngSize = 577255;

    extern const char*   GradientStrip_png;
    const int            GradientStrip_pngSize = 1055834;

    extern const char*   Focus2_png;
    const int            Focus2_pngSize = 236988;

    extern const char*   VECTOR2_png;
    const int            VECTOR2_pngSize = 273804;

    extern const char*   knobRelease_png;
    const int            knobRelease_pngSize = 70674;

    extern const char*   CODAGUI_png;
    const int            CODAGUI_pngSize = 114246;

    extern const char*   CODAGUI2_png;
    const int            CODAGUI2_pngSize = 140154;

    extern const char*   GUIBACK_png;
    const int            GUIBACK_pngSize = 328626;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 10;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
