#pragma once
#include "PluginEditor.h"
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>
inline void runControlAudit(){
 auto require=[](bool ok,const juce::String&msg){if(!ok)throw std::runtime_error(msg.toStdString());};
 auto find=[](juce::Component&root,const juce::String&key,auto&&self)->juce::Component*{if(root.getName()==key||root.getComponentID()==key)return &root;for(auto*c:root.getChildren())if(auto*r=self(*c,key,self))return r;return nullptr;};
 PlateProcessor processor;
 // Open after a non-default preset: reset must still use declared defaults.
 processor.loadPreset(7);PlateEditor editor(processor);editor.setSize(1043,398);editor.setVisible(true);
 for(const auto&spec:plateui::dials){
  auto*slider=dynamic_cast<juce::Slider*>(find(editor,spec.id,find));require(slider!=nullptr,juce::String("Missing dial ")+spec.id);
  require(editor.getComponentAt(editor.getLocalPoint(slider,slider->getLocalBounds().getCentre()))==slider,juce::String("Obstructed dial hit area: ")+spec.id);
  auto*parameter=processor.state.getParameter(spec.id);const auto range=parameter->getNormalisableRange();
  require(std::abs(slider->getDoubleClickReturnValue()-parameter->convertFrom0to1(parameter->getDefaultValue()))<.001,"Incorrect double-click default");
  for(double proportion:{0.,.25,.5,.75,1.}){
   const auto value=slider->proportionOfLengthToValue(proportion);require(std::abs(slider->valueToProportionOfLength(value)-proportion)<1e-6,"Noninvertible physical dial scale");
   const auto before=processor.state.copyState();slider->setValue(value,juce::sendNotificationSync);
   require(std::abs(processor.get(spec.id)-range.snapToLegalValue(float(value)))<.002,juce::String("Dial attachment mismatch: ")+spec.id);
   for(const auto&child:before){auto id=child.getProperty("id").toString();if(id!=spec.id)require(processor.get(id)==float(child.getProperty("value")),"Dial changed another parameter");}
  }
  processor.set(spec.id,range.snapToLegalValue(range.convertFrom0to1(.37f)));
  // On the message thread the genuine parameter attachment updates synchronously.
  require(std::abs(slider->getValue()-processor.get(spec.id))<.002,juce::String("Host-to-dial mismatch: ")+spec.id);
  std::cout<<"PASS dial "<<spec.id<<": UI -> APVTS, host -> UI, isolation, endpoints, default\n";
 }
 for(auto pair:{std::pair<const char*,double>{"eq_low_freq",200},{"eq_high_freq",2000},{"mod_rate",.1},{"eq_low_gain",0},{"eq_high_gain",0},{"predelay",50},{"mix",15}}){
  auto*slider=dynamic_cast<juce::Slider*>(find(editor,pair.first,find));require(std::abs(slider->proportionOfLengthToValue(.5)-pair.second)<.15,"Printed noon value mismatch");
 }
 for(int i=0;i<3;++i){auto id=juce::String("reverb_time_")+juce::String::charToString(char('a'+i));auto*slider=dynamic_cast<juce::Slider*>(find(editor,id,find));require(slider!=nullptr,"Missing RT meter");for(double value:{.5,2.,5.5}){slider->setValue(value,juce::sendNotificationSync);require(std::abs(processor.get(id)-value)<.001,"RT attachment mismatch");}auto*button=dynamic_cast<juce::Button*>(find(editor,"plate-"+juce::String(i),find));require(button!=nullptr,"Missing plate selector");button->onClick();require(processor.get("plate_select")==i,"Wrong selected plate");}
 auto*filter=dynamic_cast<juce::Slider*>(find(editor,"input-filter",find));require(filter!=nullptr,"Missing input filter");
 for(int i=0;i<10;++i){filter->setValue(i,juce::sendNotificationSync);require(processor.get("input_filter_model")==float(i==0?2:i<7?1:0),"Wrong filter model");if(i>0&&i<7)require(processor.get("input_filter_martech_hz")==i-1,"Wrong MARTECH index");if(i>=7)require(processor.get("input_filter_emt_db")==i-7,"Wrong EMT attenuation index");}
 for(const char*id:{"power","eq_enable","wet_solo"}){auto*button=dynamic_cast<juce::Button*>(find(editor,id,find));require(button!=nullptr,"Missing toggle");for(bool value:{false,true}){button->setToggleState(value,juce::sendNotificationSync);require(processor.get(id)==float(value),"Toggle attachment mismatch");}}
 auto dial=[&](const char*id,float value){auto*s=dynamic_cast<juce::Slider*>(find(editor,id,find));require(s!=nullptr,"Missing audio test dial");s->setValue(value,juce::sendNotificationSync);};
 auto render=[&](double frequency,float lowGain,float highGain,bool equalizer,bool wet,float mix){
  processor.resetToDefaults();processor.set("input_filter_model",2);processor.set("wet_solo",wet?1.f:0.f);processor.set("eq_enable",equalizer?1.f:0.f);
  // Use the same live controls as the user, not direct raw DSP parameter assignment.
  dial("eq_low_freq",200);dial("eq_high_freq",2000);dial("eq_low_gain",lowGain);dial("eq_high_gain",highGain);dial("predelay",0);dial("mix",mix);
  processor.prepareToPlay(48000,256);juce::AudioBuffer<float> block(2,256);juce::MidiBuffer midi;double energy=0;
  for(int b=0;b<1125;++b){for(int i=0;i<256;++i){float x=float(.015*std::sin(2*plate::pi*frequency*double(b*256+i)/48000));block.setSample(0,i,x);block.setSample(1,i,x);}processor.processBlock(block,midi);if(b>=750)for(int c=0;c<2;++c)for(int i=0;i<256;++i){auto v=block.getSample(c,i);require(std::isfinite(v),"Nonfinite EQ output");energy+=double(v)*v;}}
  processor.releaseResources();return energy;
 };
 for(bool high:{false,true}){
  const auto frequency=high?12000.:40.;const auto flat=render(frequency,0,0,true,true,100);
  for(float gain:{-12.f,12.f}){double energy=render(frequency,high?0:gain,high?gain:0,true,true,100);double db=10*std::log10(energy/flat);std::cout<<(high?"High":"Low")<<" GAIN "<<gain<<" dB -> measured "<<db<<" dB\n";require(std::abs(db-gain)<.4,"EQ gain did not reach requested shelf gain");}
  auto bypass=render(frequency,0,0,false,true,100);auto ignored=render(frequency,high?0:12,high?12:0,false,true,100);require(std::abs(10*std::log10(ignored/bypass))<.001,"EQ OUT must bypass both gains");
  auto dry=render(frequency,0,0,true,false,0);auto dryGain=render(frequency,high?0:12,high?12:0,true,false,0);require(std::abs(10*std::log10(dryGain/dry))<.001,"Wet EQ must not change dry at MIX 0");
 }
 // Every parameter must also affect actual processor audio in its active context.
 // Inactive parameters (e.g. MIX under WET SOLO) are deliberately tested separately.
 const std::array<std::pair<float,float>,20> extremes{{{0,2},{.6f,5.5f},{.6f,5.5f},{.6f,5.5f},{2,0},{0,2},{0,5},{0,100},{-100,100},{0,1},{20,2000},{-12,12},{200,20000},{-12,12},{0,1},{0,10},{0,250},{0,100},{0,1},{0,1}}};
 auto impulse=[&](int index,float value){
  processor.resetToDefaults();processor.set("input_filter_model",2);processor.set("predelay",0);
  if(index>=1&&index<=3)processor.set("plate_select",float(index-1));
  if(index==5)processor.set("input_filter_model",0);if(index==6)processor.set("input_filter_model",1);
  if(index==9){processor.set("eq_low_gain",12);processor.set("eq_high_gain",-12);}
  if(index==10)processor.set("eq_low_gain",12);if(index==12)processor.set("eq_high_gain",12);
  if(index==11)processor.set("eq_low_freq",200);if(index==13)processor.set("eq_high_freq",2000);
  if(index==14)processor.set("mod_depth",10);if(index==15)processor.set("mod_rate",.8f);
  if(index==17)processor.set("wet_solo",0);if(index==18)processor.set("mix",15);
  processor.set(platebank::ids[index],value);processor.prepareToPlay(48000,256);
  juce::AudioBuffer<float> block(2,256);juce::MidiBuffer midi;
  for(int b=0;b<188;++b){block.clear();processor.processBlock(block,midi);}
  std::vector<float> output;output.reserve(256*2*113);
  for(int b=0;b<113;++b){block.clear();if(b==0){block.setSample(0,0,.1f);block.setSample(1,0,.07f);}processor.processBlock(block,midi);for(int c=0;c<2;++c)for(int n=0;n<256;++n)output.push_back(block.getSample(c,n));}
  processor.releaseResources();return output;
 };
 for(int i=0;i<20;++i){auto a=impulse(i,extremes[size_t(i)].first),b=impulse(i,extremes[size_t(i)].second);double difference=0,energy=0;for(size_t n=0;n<a.size();++n){double d=double(a[n])-b[n];difference+=d*d;energy+=double(a[n])*a[n]+double(b[n])*b[n];}double relative=std::sqrt(difference/std::max(energy,1e-30));require(relative>.01,juce::String("Parameter has no measurable audio effect: ")+platebank::ids[i]);std::cout<<"PASS audio "<<platebank::ids[i]<<" relative IR difference "<<relative<<'\n';}
 std::cout<<"PASS: all 20 parameters, 10 dial attachments, 3 RTs, A/B/C, all filter positions, toggles and real processor low/high gain audio\n";
}
