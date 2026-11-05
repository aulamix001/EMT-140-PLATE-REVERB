#pragma once
#include "PluginProcessor.h"
#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <limits>
#include <vector>
inline void runAudioContinuityAudit(){
 auto require=[](bool ok,const char* message){if(!ok)throw std::runtime_error(message);};
 juce::MidiBuffer midi;PlateProcessor p;p.loadPreset(7);p.prepareToPlay(48000,64);
 juce::AudioBuffer<float> audio(2,64);
 for(int b=0;b<100;++b){for(int c=0;c<2;++c)for(int i=0;i<64;++i)audio.setSample(c,i,.02f);p.processBlock(audio,midi);}
 const auto before=p.state.copyState();const int program=p.getCurrentProgram();juce::AudioProcessor& host=p;host.reset();
 require(before.isEquivalentTo(p.state.copyState()),"HOST RESET CHANGED PRESET/PARAMETERS");require(p.getCurrentProgram()==program,"host reset changed current program");
 audio.clear();p.processBlock(audio,midi);for(int c=0;c<2;++c)for(int i=0;i<64;++i)require(audio.getSample(c,i)==0,"host reset did not flush DSP history");
 std::cout<<"PASS: actual AudioProcessor::reset preserves preset/program and clears DSP history\n";
 for(bool bypass:{false,true}){p.resetToDefaults();p.set("mix",0);p.set("wet_solo",0);p.set("power",bypass?0.f:1.f);p.prepareToPlay(48000,64);for(int c=0;c<2;++c)for(int i=0;i<64;++i)audio.setSample(c,i,.2f);p.processBlock(audio,midi);for(int c=0;c<2;++c)for(int i=0;i<64;++i)require(audio.getSample(c,i)==.2f,"processor startup muted dry MIX0/bypass");}
 auto input=[](int sample,int channel){return float(.02*std::sin(2*plate::pi*(channel?733:997)*sample/48000)+.003);};
 auto render=[&](const std::vector<int>& sizes){PlateProcessor engine;engine.loadPreset(7);engine.prepareToPlay(48000,2048);juce::AudioBuffer<float> storage(2,2048);std::vector<float> result;result.reserve(96000);int sample=0;size_t step=0;while(sample<48000){int count=std::min(sizes[step++%sizes.size()],48000-sample);juce::AudioBuffer<float> block(storage.getArrayOfWritePointers(),2,count);for(int c=0;c<2;++c)for(int i=0;i<count;++i)block.setSample(c,i,input(sample+i,c));engine.processBlock(block,midi);for(int i=0;i<count;++i)for(int c=0;c<2;++c)result.push_back(block.getSample(c,i));sample+=count;}return result;};
 require(render({512})==render({0,32,100,127,1,512,520,1024,2048,7}),"output depends on host block partition");
 std::cout<<"PASS: bit-identical processor output with zero/odd/1/32/100/512/520/1024/2048-sample blocks\n";
 // Interrupt the real preset loader after the first parameter has changed. Audio
 // must use its preceding snapshot, not partially loaded settings, without waiting.
 PlateProcessor control;p.resetToDefaults();p.loadPreset(0);control.loadPreset(0);p.prepareToPlay(48000,64);control.prepareToPlay(48000,64);juce::AudioBuffer<float> reference(2,64);
 int sample=0;auto fill=[&](){for(int c=0;c<2;++c)for(int i=0;i<64;++i){float x=input(sample+i,c);audio.setSample(c,i,x);reference.setSample(c,i,x);}sample+=64;};
 for(int b=0;b<400;++b){fill();p.processBlock(audio,midi);control.processBlock(reference,midi);}
 struct Gate:juce::AudioProcessorParameter::Listener{std::atomic<bool> entered{false},release{false};void parameterGestureChanged(int,bool)override{}void parameterValueChanged(int,float)override{if(entered.exchange(true))return;while(!release.load())std::this_thread::yield();}} gate;
 auto*parameter=p.state.getParameter("reverb_time_a");parameter->addListener(&gate);std::thread writer([&]{p.loadPreset(3);});
 const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);while(!gate.entered.load()&&std::chrono::steady_clock::now()<deadline)std::this_thread::sleep_for(std::chrono::milliseconds(1));
 const bool intercepted=gate.entered.load()&&p.get("plate_select")==1;bool same=true;
 if(intercepted)for(int b=0;b<8;++b){fill();p.processBlock(audio,midi);control.processBlock(reference,midi);for(int c=0;c<2;++c)for(int i=0;i<64;++i)same=same&&(audio.getSample(c,i)==reference.getSample(c,i));}
 gate.release=true;writer.join();parameter->removeListener(&gate);require(intercepted,"preset concurrency barrier was not reached");require(same,"audio consumed a partially loaded preset");require(p.getCurrentProgram()==3,"preset write did not complete");
 std::cout<<"PASS: audio continues bit-identically while the actual preset loader is suspended mid-write\n";
 const char* ids[]={"eq_low_freq","eq_high_gain","predelay","mod_rate"};std::array<float,4> saved{};
 for(size_t i=0;i<4;++i){auto*raw=p.state.getRawParameterValue(ids[i]);saved[i]=raw->load();raw->store(std::numeric_limits<float>::quiet_NaN());}
 fill();audio.setSample(0,0,std::numeric_limits<float>::infinity());audio.setSample(1,7,std::numeric_limits<float>::quiet_NaN());p.processBlock(audio,midi);
 for(int c=0;c<2;++c)for(int i=0;i<64;++i)require(std::isfinite(audio.getSample(c,i)),"invalid host value poisoned audio");
 for(size_t i=0;i<4;++i)p.state.getRawParameterValue(ids[i])->store(saved[i]);
 std::cout<<"PASS: invalid host parameter/audio values are contained\n";
}
