#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace evil
{


    // This file defines the command IDs for the Evil DAW application.
    // Command IDs are used to identify specific actions or commands within the application.
    // Command IDs are typically defined as constants or enumerations.
    // Example of defining command IDs using an enumeration

    constexpr int FileCommandsIDBase = 0x030000; // Base value for file command IDs
    constexpr int EditCommandsIDBase = 0x040000; // Base value for edit command IDs
    constexpr int OptionCommandsIDBase = 0x050000; // Base value for options command IDs

    namespace juceCommandIDs = juce::StandardApplicationCommandIDs;

    enum class CommandID
    {
        // File operations
        NewProject    = FileCommandsIDBase + 0x00,
        OpenProject   = FileCommandsIDBase + 0x01,
        SaveProject   = FileCommandsIDBase + 0x02,
        SaveProjectAs = FileCommandsIDBase + 0x03,

        Exit          = juceCommandIDs::quit,  

        // Edit operations
        Undo,
        Redo,
        Cut           = juceCommandIDs::cut,
        Copy          = juceCommandIDs::copy,
        Paste         = juceCommandIDs::paste,
        
        // View operations

        // Options operations
        AudioSettings = OptionCommandsIDBase + 0x00,

    };
}