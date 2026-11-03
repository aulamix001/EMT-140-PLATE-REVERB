#pragma once
#include <JuceHeader.h>
#include "DSP/PlateEngine.h"
#include "Presets/FactoryPresets.h"
class PlateProcessor : public juce::AudioProcessor {
public:
 PlateProcessor();
 juce::AudioProcessorValueTreeState state;
 std::atomic<float> meter{0};
 void prepareToPlay(double,int) override;
 void releaseResources() override;
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor()const override{return true;}
 const juce::String getName()const override{return "PLATE 140";}
 bool acceptsMidi()const override{return false;} bool producesMidi()const override{return false;}
 double getTailLengthSeconds()const override{return 6;}
 int getNumPrograms()override{return platebank::count;}int getCurrentProgram()override{return currentProgram.load();}void setCurrentProgram(int n)override{loadPreset(n);}
 const juce::String getProgramName(int n)override{return n>=0&&n<platebank::count?platebank::presets[n].name:juce::String{}; }void changeProgramName(int,const juce::String&)override{}
 void getStateInformation(juce::MemoryBlock&)override;void setStateInformation(const void*,int)override;
 void set(const juce::String&,float); float get(const juce::String&)const;
 void loadPreset(int);void reset();void restoreState(const juce::ValueTree&);
 static juce::StringArray presetNames();
private:
 std::atomic<int> currentProgram{0};
 plate::Engine engine; double sampleRate=48000, vu=0;
 bool prepared=false;
 static juce::AudioProcessorValueTreeState::ParameterLayout layout();
 std::array<std::atomic<float>*,21> raw{};
};
