#pragma once
#include "PluginProcessor.h"
#include "GUI/RackLookAndFeel.h"
#include "Presets/PresetBrowser.h"
#include "Common/EditorGraphics.h"
#include "Common/EditorLifetime.h"
#include "GUI/ControlMapping.h"
class PlateEditor:public juce::AudioProcessorEditor,private juce::Timer {
public:
 explicit PlateEditor(PlateProcessor&);~PlateEditor()override;
 void setSoftwareGraphics(bool);
 void mouseDown(const juce::MouseEvent&)override;void paint(juce::Graphics&)override;void resized()override;
private:
 PlateProcessor&p;plateui::EditorGraphics graphics;vintage::RackLook look;juce::Component panel;
 std::array<plateui::PanelKnob,10> knobs;
 std::array<vintage::RackTimeMeter,3> times;
 std::array<juce::TextButton,3> letters;std::array<juce::TextButton,6> dampers;
 juce::TextButton power{"ON"},eq{"IN"},solo{"WET SOLO"},prev{"<"},next{">"},ab{"A / B"},reset{"RESET"},save{"SAVE"},load{"OPEN"};
 juce::Slider filter;juce::ComboBox presets;
 std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliders;
 std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttons;
 vintage::SoftwareLookAndFeel toolbarLook;juce::TextButton browse{"Default"},inButton{"IN"},aButton{"A"},bButton{"B"},copyButton{"COPY"},pasteButton{"PASTE"},learnButton{"MAP"},moreButton{"..."};
 std::unique_ptr<PresetWindow> presetWindow;juce::ValueTree stateA,stateB;bool usingB=false,closing=false;
 void openPresets();void updatePresetCaption();
 std::unique_ptr<juce::FileChooser> chooser;juce::ValueTree alternate;float vu=0;bool bState=false;int presetIndex=-1;
 void timerCallback()override;void chooseFile(bool saving);void drawPanel(juce::Graphics&);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlateEditor)
};
