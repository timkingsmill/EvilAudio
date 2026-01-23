/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 evil_audio_eq
  vendor:             evil_audio
  version:            8.0.10
  name:               Evil Audio Equaliser
  minimumCppStandard: 17

  dependencies:       juce_gui_basics
  OSXFrameworks:      Accelerate
  iOSFrameworks:      Accelerate

 END_JUCE_MODULE_DECLARATION
****************************************************************************/


#pragma once

#define EVIL_AUDIO_EQ_H_INCLUDED

#include "eq/ParametricEqualiserEditor.h"
#include "eq/ParametricEqualiserProcessor.h"
