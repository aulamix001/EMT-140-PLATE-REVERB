#pragma once
#include "PluginProcessor.h"
#include <iostream>
#include <limits>
#include <stdexcept>
inline void runHostLifecycleRegression(){
 auto check=[](bool ok,const char* message){if(!ok)throw std::runtime_error(message);};
 juce::MidiBuffer midi;
 PlateProcessor processor;processor.prepareToPlay(48000,64);
 // Hosts can flush a processor with deactivated audio buses.
 std::cout<<"CHECK: zero audio channels, 64 samples"<<std::endl;
 juce::AudioBuffer<float> noChannels(0,64);processor.processBlock(noChannels,midi);
 juce::AudioBuffer<float> noSamples(2,0);processor.processBlock(noSamples,midi);
 PlateProcessor unprepared;juce::AudioBuffer<float> audio(2,64);
 auto fill=[&]{for(int c=0;c<2;++c)for(int i=0;i<64;++i)audio.setSample(c,i,.25f);};
 auto silent=[&]{for(int c=0;c<2;++c)for(int i=0;i<64;++i)check(audio.getSample(c,i)==0,"inactive processor must output silence");};
 fill();unprepared.processBlock(audio,midi);silent();
 unprepared.prepareToPlay(48000,64);unprepared.releaseResources();
 fill();unprepared.processBlock(audio,midi);silent();
 for(double rate:{0.,-1.,std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity()}){
  unprepared.prepareToPlay(rate,64);fill();unprepared.processBlock(audio,midi);silent();
 }
 for(double rate:{44100.,48000.,96000.,192000.})for(int channels:{1,2}){
  unprepared.prepareToPlay(rate,64);audio.setSize(channels,64);audio.clear();audio.setSample(0,0,.25f);
  unprepared.processBlock(audio,midi);
  for(int c=0;c<channels;++c)for(int i=0;i<64;++i)check(std::isfinite(audio.getSample(c,i)),"reactivated audio must be finite");
  unprepared.releaseResources();
 }
 std::cout<<"PASS: empty/deactivated buses, zero samples, pre-prepare/post-release callbacks, mono/stereo and repeated activation"<<std::endl;
}
