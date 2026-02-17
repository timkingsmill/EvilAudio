#pragma once
#define EVILAUDIO_AUDIO_DEVICES_H_INCLUDED

/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.

 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 evilaudio_audio_devices
  vendor:             evilaudio
  version:            8.0.10
  name:               Evil Audio Audio Devices
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       juce_audio_basics, juce_events

  OSXFrameworks:      CoreAudio CoreMIDI AudioToolbox
  iOSFrameworks:      CoreAudio CoreMIDI AudioToolbox AVFoundation
  linuxPackages:      alsa

 END_JUCE_MODULE_DECLARATION

****************************************************************************/

