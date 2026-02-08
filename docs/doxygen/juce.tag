<?xml version='1.0' encoding='UTF-8' standalone='yes' ?>
<tagfile doxygen_version="1.15.0" doxygen_gitid="7cca38ba5185457e6d9495bf963d4cdeacebc25a">
  <compound kind="file">
    <name>AudioSettingsComponent.h</name>
    <path>C:/source/EvilAudio/source/applications/EvilDAW/include/</path>
    <filename>AudioSettingsComponent_8h.html</filename>
    <class kind="class">AudioSettingsComponent</class>
  </compound>
  <compound kind="file">
    <name>LiveScrollingAudioVisualiser.h</name>
    <path>C:/source/EvilAudio/source/applications/EvilDAW/include/</path>
    <filename>LiveScrollingAudioVisualiser_8h.html</filename>
    <class kind="class">LiveScrollingAudioVisualiser</class>
  </compound>
  <compound kind="file">
    <name>MainWindowContent.h</name>
    <path>C:/source/EvilAudio/source/applications/EvilDAW/include/</path>
    <filename>EvilDAW_2include_2MainWindowContent_8h.html</filename>
    <includes id="AudioSettingsComponent_8h" name="AudioSettingsComponent.h" local="yes" import="no" module="no" objc="no">AudioSettingsComponent.h</includes>
    <includes id="LiveScrollingAudioVisualiser_8h" name="LiveScrollingAudioVisualiser.h" local="yes" import="no" module="no" objc="no">LiveScrollingAudioVisualiser.h</includes>
    <class kind="class">MainWindowContent</class>
  </compound>
  <compound kind="file">
    <name>MainWindowContent.h</name>
    <path>C:/source/EvilAudio/source/applications/EvilLookAndFeel/include/content/</path>
    <filename>EvilLookAndFeel_2include_2content_2MainWindowContent_8h.html</filename>
    <class kind="class">LookAndFeelMainWindowContent</class>
  </compound>
  <compound kind="file">
    <name>MainWindowContent.h</name>
    <path>C:/source/EvilAudio/source/applications/EvilPluginHost/gui/</path>
    <filename>EvilPluginHost_2gui_2MainWindowContent_8h.html</filename>
    <class kind="class">MainWindowContent</class>
  </compound>
  <compound kind="file">
    <name>EvilDAW.cpp</name>
    <path>C:/source/EvilAudio/source/applications/EvilDAW/source/</path>
    <filename>EvilDAW_8cpp.html</filename>
  </compound>
  <compound kind="file">
    <name>EvilEQApplication.cpp</name>
    <path>C:/source/EvilAudio/source/applications/EvilEQ/source/</path>
    <filename>EvilEQApplication_8cpp.html</filename>
    <member kind="function">
      <type>juce::AudioProcessor *JUCE_CALLTYPE</type>
      <name>createPluginFilter</name>
      <anchorfile>EvilEQApplication_8cpp.html</anchorfile>
      <anchor>a62f69a86baff96d3ee5c287e100adc66</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>EvilLookAndFeelApplication.cpp</name>
    <path>C:/source/EvilAudio/source/applications/EvilLookAndFeel/source/</path>
    <filename>EvilLookAndFeelApplication_8cpp.html</filename>
    <includes id="EvilLookAndFeel_2include_2content_2MainWindowContent_8h" name="MainWindowContent.h" local="yes" import="no" module="no" objc="no">content/MainWindowContent.h</includes>
  </compound>
  <compound kind="file">
    <name>EvilPluginHost.cpp</name>
    <path>C:/source/EvilAudio/source/applications/EvilPluginHost/</path>
    <filename>EvilPluginHost_8cpp.html</filename>
    <includes id="EvilPluginHost_2gui_2MainWindowContent_8h" name="MainWindowContent.h" local="yes" import="no" module="no" objc="no">gui/MainWindowContent.h</includes>
  </compound>
  <compound kind="file">
    <name>MainWindowContent.cpp</name>
    <path>C:/source/EvilAudio/source/applications/EvilPluginHost/gui/</path>
    <filename>MainWindowContent_8cpp.html</filename>
    <includes id="EvilPluginHost_2gui_2MainWindowContent_8h" name="MainWindowContent.h" local="yes" import="no" module="no" objc="no">MainWindowContent.h</includes>
  </compound>
  <compound kind="class">
    <name>AudioSettingsComponent</name>
    <filename>classAudioSettingsComponent.html</filename>
    <member kind="function">
      <type></type>
      <name>AudioSettingsComponent</name>
      <anchorfile>classAudioSettingsComponent.html</anchorfile>
      <anchor>a7b6695f20787706b074436f513418ce0</anchor>
      <arglist>(juce::AudioDeviceManager &amp;audioDeviceManager)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>~AudioSettingsComponent</name>
      <anchorfile>classAudioSettingsComponent.html</anchorfile>
      <anchor>a5ae28cdeda314d6768c56fcd7af122b3</anchor>
      <arglist>() override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>paint</name>
      <anchorfile>classAudioSettingsComponent.html</anchorfile>
      <anchor>a9a55ec3a68dd086b1e887730390e2d67</anchor>
      <arglist>(juce::Graphics &amp;graphics) override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>resized</name>
      <anchorfile>classAudioSettingsComponent.html</anchorfile>
      <anchor>a38396b904ea83ce8c47cda63f3d3f0b0</anchor>
      <arglist>() override</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>LiveScrollingAudioVisualiser</name>
    <filename>classLiveScrollingAudioVisualiser.html</filename>
    <member kind="function">
      <type></type>
      <name>LiveScrollingAudioVisualiser</name>
      <anchorfile>classLiveScrollingAudioVisualiser.html</anchorfile>
      <anchor>a328073f809132b31aafe01c1e256654f</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>~LiveScrollingAudioVisualiser</name>
      <anchorfile>classLiveScrollingAudioVisualiser.html</anchorfile>
      <anchor>a452b09e6efea48e4c528a338ebb158dc</anchor>
      <arglist>() override=default</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>audioDeviceIOCallbackWithContext</name>
      <anchorfile>classLiveScrollingAudioVisualiser.html</anchorfile>
      <anchor>a9874f465dbac2c92736d4ac87722ea96</anchor>
      <arglist>(const float *const *inputChannelData, int numInputChannels, float *const *outputChannelData, int numOutputChannels, int numSamples, const juce::AudioIODeviceCallbackContext &amp;context) override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>audioDeviceAboutToStart</name>
      <anchorfile>classLiveScrollingAudioVisualiser.html</anchorfile>
      <anchor>a993672c011b14091ce31535ec20046f0</anchor>
      <arglist>(juce::AudioIODevice *device) override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>audioDeviceStopped</name>
      <anchorfile>classLiveScrollingAudioVisualiser.html</anchorfile>
      <anchor>ab1b9f19c173f403a42f486c67d8d7cd2</anchor>
      <arglist>() override</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>LookAndFeelMainWindowContent</name>
    <filename>classLookAndFeelMainWindowContent.html</filename>
    <member kind="function">
      <type></type>
      <name>LookAndFeelMainWindowContent</name>
      <anchorfile>classLookAndFeelMainWindowContent.html</anchorfile>
      <anchor>a6ea7855d7551fb9e110b2518404df613</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>~LookAndFeelMainWindowContent</name>
      <anchorfile>classLookAndFeelMainWindowContent.html</anchorfile>
      <anchor>a891fbfaf3f05e4265eeed916befd0289</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>paint</name>
      <anchorfile>classLookAndFeelMainWindowContent.html</anchorfile>
      <anchor>a4b083818fb9f5569636b95b03d4e8357</anchor>
      <arglist>(juce::Graphics &amp;graphics) override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>resized</name>
      <anchorfile>classLookAndFeelMainWindowContent.html</anchorfile>
      <anchor>ab1be39001b5c38c6c89444b9d3d3453f</anchor>
      <arglist>() override</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>MainWindowContent</name>
    <filename>classMainWindowContent.html</filename>
    <member kind="enumeration">
      <type></type>
      <name>CommandIDs</name>
      <anchorfile>classMainWindowContent.html</anchorfile>
      <anchor>aa7d70ee1b204a78b2c91048d65723e60</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>audioSetupCommandID</name>
      <anchorfile>classMainWindowContent.html</anchorfile>
      <anchor>aa7d70ee1b204a78b2c91048d65723e60a4165a98c852d536dac9e6b4e9bad2c9c</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>exitAppCommandID</name>
      <anchorfile>classMainWindowContent.html</anchorfile>
      <anchor>aa7d70ee1b204a78b2c91048d65723e60a158940001b4e8bdeec2907242c5fb6f2</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>audioSetupCommandID</name>
      <anchorfile>classMainWindowContent.html</anchorfile>
      <anchor>aa7d70ee1b204a78b2c91048d65723e60a4165a98c852d536dac9e6b4e9bad2c9c</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>exitAppCommandID</name>
      <anchorfile>classMainWindowContent.html</anchorfile>
      <anchor>aa7d70ee1b204a78b2c91048d65723e60a158940001b4e8bdeec2907242c5fb6f2</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>MainWindowContent</name>
      <anchorfile>classMainWindowContent.html</anchorfile>
      <anchor>a4e1fbe2e1bf6a9ad1aa0425aabbe4359</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>~MainWindowContent</name>
      <anchorfile>classMainWindowContent.html</anchorfile>
      <anchor>acc237adc998fa904efa1c4271791c403</anchor>
      <arglist>() override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>paint</name>
      <anchorfile>classMainWindowContent.html</anchorfile>
      <anchor>aba634c15cd81d7161c855c279bfa8127</anchor>
      <arglist>(juce::Graphics &amp;graphics) override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>resized</name>
      <anchorfile>classMainWindowContent.html</anchorfile>
      <anchor>a19f55ef1d5ae2bb0707c940a98bdb4f2</anchor>
      <arglist>() override</arglist>
    </member>
  </compound>
  <compound kind="dir">
    <name>C:/source/EvilAudio/source/applications</name>
    <path>C:/source/EvilAudio/source/applications/</path>
    <filename>dir_0ba5dfdf3cd06b2886d521eaefeab5e8.html</filename>
    <dir>C:/source/EvilAudio/source/applications/EvilDAW</dir>
    <dir>C:/source/EvilAudio/source/applications/EvilEQ</dir>
    <dir>C:/source/EvilAudio/source/applications/EvilLookAndFeel</dir>
    <dir>C:/source/EvilAudio/source/applications/EvilPluginHost</dir>
  </compound>
  <compound kind="dir">
    <name>C:</name>
    <path>C:/</path>
    <filename>dir_e6bb53534ac0e427887cf7a94c0c004e.html</filename>
    <dir>C:/source</dir>
  </compound>
  <compound kind="dir">
    <name>C:/source/EvilAudio/source/applications/EvilLookAndFeel/include/content</name>
    <path>C:/source/EvilAudio/source/applications/EvilLookAndFeel/include/content/</path>
    <filename>dir_e4c2b5f1318f30dbb2644b833df31a5b.html</filename>
    <file>MainWindowContent.h</file>
  </compound>
  <compound kind="dir">
    <name>C:/source/EvilAudio</name>
    <path>C:/source/EvilAudio/</path>
    <filename>dir_2d85542102cf64157553b6fe3a0f88d8.html</filename>
    <dir>C:/source/EvilAudio/source</dir>
  </compound>
  <compound kind="dir">
    <name>C:/source/EvilAudio/source/applications/EvilDAW</name>
    <path>C:/source/EvilAudio/source/applications/EvilDAW/</path>
    <filename>dir_9b0f4c96c03d27eeafd3e5540b24db1b.html</filename>
    <dir>C:/source/EvilAudio/source/applications/EvilDAW/include</dir>
    <dir>C:/source/EvilAudio/source/applications/EvilDAW/source</dir>
  </compound>
  <compound kind="dir">
    <name>C:/source/EvilAudio/source/applications/EvilEQ</name>
    <path>C:/source/EvilAudio/source/applications/EvilEQ/</path>
    <filename>dir_c1274bcdbac7b71681e11a68ad874cec.html</filename>
    <dir>C:/source/EvilAudio/source/applications/EvilEQ/source</dir>
  </compound>
  <compound kind="dir">
    <name>C:/source/EvilAudio/source/applications/EvilLookAndFeel</name>
    <path>C:/source/EvilAudio/source/applications/EvilLookAndFeel/</path>
    <filename>dir_ee89954b6e997baf01512c696da110c2.html</filename>
    <dir>C:/source/EvilAudio/source/applications/EvilLookAndFeel/include</dir>
    <dir>C:/source/EvilAudio/source/applications/EvilLookAndFeel/source</dir>
  </compound>
  <compound kind="dir">
    <name>C:/source/EvilAudio/source/applications/EvilPluginHost</name>
    <path>C:/source/EvilAudio/source/applications/EvilPluginHost/</path>
    <filename>dir_3f5ba2803823199c185f2253896dbbb9.html</filename>
    <dir>C:/source/EvilAudio/source/applications/EvilPluginHost/gui</dir>
    <file>EvilPluginHost.cpp</file>
  </compound>
  <compound kind="dir">
    <name>C:/source/EvilAudio/source/applications/EvilPluginHost/gui</name>
    <path>C:/source/EvilAudio/source/applications/EvilPluginHost/gui/</path>
    <filename>dir_4ce70ffefa6f00954ae4d5f02e77f70b.html</filename>
    <file>MainWindowContent.cpp</file>
    <file>MainWindowContent.h</file>
  </compound>
  <compound kind="dir">
    <name>C:/source/EvilAudio/source/applications/EvilDAW/include</name>
    <path>C:/source/EvilAudio/source/applications/EvilDAW/include/</path>
    <filename>dir_16ecb5cd82b6ee6a70e3beda06afceb7.html</filename>
    <file>AudioSettingsComponent.h</file>
    <file>LiveScrollingAudioVisualiser.h</file>
    <file>MainWindowContent.h</file>
  </compound>
  <compound kind="dir">
    <name>C:/source/EvilAudio/source/applications/EvilLookAndFeel/include</name>
    <path>C:/source/EvilAudio/source/applications/EvilLookAndFeel/include/</path>
    <filename>dir_ab115f3f261f71a2cc750b6e79ffc0d9.html</filename>
    <dir>C:/source/EvilAudio/source/applications/EvilLookAndFeel/include/content</dir>
  </compound>
  <compound kind="dir">
    <name>C:/source</name>
    <path>C:/source/</path>
    <filename>dir_dfc43348b79d93be412a5e4458cb0840.html</filename>
    <dir>C:/source/EvilAudio</dir>
  </compound>
  <compound kind="dir">
    <name>C:/source/EvilAudio/source</name>
    <path>C:/source/EvilAudio/source/</path>
    <filename>dir_ff885548ef1464f598f3b45b44d0aeea.html</filename>
    <dir>C:/source/EvilAudio/source/applications</dir>
  </compound>
  <compound kind="dir">
    <name>C:/source/EvilAudio/source/applications/EvilDAW/source</name>
    <path>C:/source/EvilAudio/source/applications/EvilDAW/source/</path>
    <filename>dir_7a07f2b40f3ad5b2d8573c66d2dcb762.html</filename>
    <file>EvilDAW.cpp</file>
  </compound>
  <compound kind="dir">
    <name>C:/source/EvilAudio/source/applications/EvilEQ/source</name>
    <path>C:/source/EvilAudio/source/applications/EvilEQ/source/</path>
    <filename>dir_0abe38157eba183f2228c0285b89e9f1.html</filename>
    <file>EvilEQApplication.cpp</file>
  </compound>
  <compound kind="dir">
    <name>C:/source/EvilAudio/source/applications/EvilLookAndFeel/source</name>
    <path>C:/source/EvilAudio/source/applications/EvilLookAndFeel/source/</path>
    <filename>dir_8bb74b92a99bd0d43c1d24cd724f6641.html</filename>
    <file>EvilLookAndFeelApplication.cpp</file>
  </compound>
</tagfile>
