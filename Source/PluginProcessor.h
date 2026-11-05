#pragma once
#include <JuceHeader.h>
#include "DSP/PlateEngine.h"
#include "Presets/FactoryPresets.h"
#include "Common/ParameterTransactions.h"
class PlateProcessor : public juce::AudioProcessor {
public:
 PlateProcessor();
 juce::AudioProcessorValueTreeState state;
 std::atomic<float> meter{0};
 std::atomic<bool> softwareGraphics{PLATE_COMPATIBLE_GRAPHICS != 0};
 void prepareToPlay(double,int) override;
 void releaseResources() override;
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor()const override{return true;}
 const juce::String getName()const override{return "PLATE 140";}
 bool acceptsMidi()const override{return false;} bool producesMidi()const override{return false;}
 double getTailLengthSeconds()const override{return 7;}
 int getNumPrograms()override{return platebank::count+1;}int getCurrentProgram()override{return currentProgram.load();}void setCurrentProgram(int n)override{if(n==platebank::count)resetToDefaults();else if(n>=0&&n<platebank::count)loadPreset(n);}
 const juce::String getProgramName(int n)override{return n==platebank::count?juce::String("Default"):(n>=0&&n<platebank::count?juce::String(platebank::presets[n].name):juce::String{}); }void changeProgramName(int,const juce::String&)override{}
 void getStateInformation(juce::MemoryBlock&)override;void setStateInformation(const void*,int)override;
 void set(const juce::String&,float); float get(const juce::String&)const;
 void loadPreset(int);void reset()override;void resetToDefaults();void restoreState(const juce::ValueTree&);
 static juce::StringArray presetNames();
private:
 std::atomic<int> currentProgram{platebank::count};
 plate::Engine engine; double sampleRate=48000, vu=0;
 bool prepared=false;
 plate::ParameterTransactions parameterTransactions;
 std::atomic<bool> resetRequested{false};
 std::array<float,20> audioValues{{0,2,2,2,0,1,2,100,0,1,120,0,8000,0,0,0,50,15,1,1}};
 plate::Parameters audioParameters()noexcept;
 static juce::AudioProcessorValueTreeState::ParameterLayout layout();
 std::array<std::atomic<float>*,20> raw{};
};
