#include "PluginProcessor.h"
#include "PluginEditor.h"
using namespace juce;
AudioProcessorValueTreeState::ParameterLayout PlateProcessor::layout(){
 AudioProcessorValueTreeState::ParameterLayout l;
 auto f=[&](const char*id,const char*name,float a,float b,float step,float def,float skew=1.f){NormalisableRange<float> range(a,b,step,skew);l.add(std::make_unique<AudioParameterFloat>(ParameterID{id,1},name,range,def));};
 auto choice=[&](const char*id,const char*name,StringArray opts,int def){l.add(std::make_unique<AudioParameterChoice>(ParameterID{id,1},name,opts,def));};
 auto flag=[&](const char*id,const char*name,bool def){l.add(std::make_unique<AudioParameterBool>(ParameterID{id,1},name,def));};
 choice("plate_select","Plate",{"A","B","C"},0);
 f("reverb_time_a","Reverb A",.5,5.5,.1,2);f("reverb_time_b","Reverb B",.5,5.5,.1,2);f("reverb_time_c","Reverb C",.5,5.5,.1,2);
 choice("input_filter_model","Input model",{"EMT","MARTECH","OFF"},0);choice("input_filter_emt_db","EMT attenuation",{"-4 dB","-10 dB","-16 dB"},1);choice("input_filter_martech_hz","Martech shelf",{"90 Hz","125 Hz","180 Hz","250 Hz","270 Hz","360 Hz"},2);
 f("width","Width",0,100,.1,100);f("balance","Balance",-100,100,.1,0);flag("eq_enable","Equalizer",true);
 f("eq_low_freq","Low frequency",20,2000,1,120,.3);f("eq_low_gain","Low gain",-12,12,.5,0);f("eq_high_freq","High frequency",200,20000,1,8000,.3);f("eq_high_gain","High gain",-12,12,.5,0);
 f("mod_rate","Mod rate",0,1,.01,0);f("mod_depth","Mod depth",0,10,.1,0);
 f("predelay","Predelay",0,250,.1,50,float(std::log(.5)/std::log(.2)));f("mix","Mix",0,100,.1,15,float(std::log(.5)/std::log(.15)));
 flag("wet_solo","Wet solo",true);flag("power","Power",true);return l;
}
PlateProcessor::PlateProcessor():AudioProcessor(BusesProperties().withInput("Input",AudioChannelSet::stereo(),true).withOutput("Output",AudioChannelSet::stereo(),true)),state(*this,nullptr,"PLATE140",layout()){
 const char*ids[]={"plate_select","reverb_time_a","reverb_time_b","reverb_time_c","input_filter_model","input_filter_emt_db","input_filter_martech_hz","width","balance","eq_enable","eq_low_freq","eq_low_gain","eq_high_freq","eq_high_gain","mod_rate","mod_depth","predelay","mix","wet_solo","power"};
 for(int i=0;i<20;++i)raw[i]=state.getRawParameterValue(ids[i]);
}
void PlateProcessor::prepareToPlay(double sr,int){
 prepared=false;vu=0;meter.store(0);
 if(!std::isfinite(sr)||sr<=0)return;
 sampleRate=sr;engine.prepare(sr);resetRequested.store(false);audioParameters();setLatencySamples(0);prepared=true;
}
void PlateProcessor::releaseResources(){prepared=false;vu=0;meter.store(0);}
bool PlateProcessor::isBusesLayoutSupported(const BusesLayout& b)const{return b.getMainInputChannelSet()==b.getMainOutputChannelSet()&&(b.getMainOutputChannelSet()==AudioChannelSet::stereo()||b.getMainOutputChannelSet()==AudioChannelSet::mono());}
plate::Parameters PlateProcessor::audioParameters()noexcept{
 auto candidate=audioValues;
 if(parameterTransactions.capture(raw,candidate)){
  constexpr float lo[]={0,.5,.5,.5,0,0,0,0,-100,0,20,-12,200,-12,0,0,0,0,0,0};
  constexpr float hi[]={2,5.5,5.5,5.5,2,2,5,100,100,1,2000,12,20000,12,1,10,250,100,1,1};
  for(size_t i=0;i<audioValues.size();++i)if(std::isfinite(candidate[i]))audioValues[i]=std::clamp(candidate[i],lo[i],hi[i]);
 }
 const auto&v=audioValues;plate::Parameters p;p.plate=int(v[0]);for(int i=0;i<3;++i)p.rt[i]=v[size_t(i+1)];p.filterModel=int(v[4]);p.emt=int(v[5]);p.martech=int(v[6]);p.width=v[7];p.balance=v[8];p.eq=v[9]>.5;p.lowFreq=v[10];p.lowGain=v[11];p.highFreq=v[12];p.highGain=v[13];p.rate=v[14];p.depth=v[15];p.predelay=v[16];p.mix=v[17];p.wetSolo=v[18]>.5;p.power=v[19]>.5;return p;
}
void PlateProcessor::reset(){resetRequested.store(true,std::memory_order_release);}
void PlateProcessor::processBlock(AudioBuffer<float>&buffer,MidiBuffer&){
 if(buffer.getNumChannels()==0||buffer.getNumSamples()==0)return;
 if(!prepared){buffer.clear();meter.store(0);return;}
 ScopedNoDenormals nd;
 if(resetRequested.exchange(false,std::memory_order_acquire)){engine.reset();vu=0;}
 const auto p=audioParameters();
 auto*l=buffer.getWritePointer(0);auto*r=buffer.getNumChannels()>1?buffer.getWritePointer(1):l;double k=1-std::exp(-1/(sampleRate*.3));
 for(int i=0;i<buffer.getNumSamples();++i){auto out=engine.tick(l[i],r[i],p);l[i]=float(out[0]);r[i]=buffer.getNumChannels()>1?float(out[1]):float((out[0]+out[1])*.5);double level=p.power?(out[0]*out[0]+out[1]*out[1])*.5:0;vu+=k*(level-vu);}
 meter.store(float(std::sqrt(std::max(0.,vu))));
}
void PlateProcessor::getStateInformation(MemoryBlock&b){auto v=state.copyState();v.setProperty("version",1,nullptr);v.setProperty("graphicsMode",softwareGraphics.load()?0:1,nullptr);auto xml=v.createXml();copyXmlToBinary(*xml,b);}
void PlateProcessor::restoreState(const ValueTree&v){if(!PresetLibrary::validState(v))return;const plate::ParameterTransactions::Write writing(parameterTransactions);softwareGraphics.store(int(v.getProperty("graphicsMode",PLATE_COMPATIBLE_GRAPHICS ? 0 : 1))==0);state.replaceState(v.createCopy());
 // Explicitly restore each host-facing value: APVTS raw bool values may already be
 // snapped to 0/1 while AudioParameterBool still holds a fractional host value.
 for(auto child:v){auto*parameter=state.getParameter(child.getProperty("id").toString());if(parameter)parameter->setValueNotifyingHost(parameter->convertTo0to1(float(child.getProperty("value"))));}
 auto key=v.getProperty("preset_key").toString();currentProgram.store(platebank::count);if(key.startsWith("factory:"))currentProgram.store(jlimit(0,platebank::count-1,key.fromFirstOccurrenceOf(":",false,false).getIntValue()-1));updateHostDisplay(ChangeDetails().withProgramChanged(true).withNonParameterStateChanged(true));}
void PlateProcessor::setStateInformation(const void*d,int size){auto xml=getXmlFromBinary(d,size);if(xml)restoreState(ValueTree::fromXml(*xml));}
void PlateProcessor::set(const String&id,float value){if(auto*p=state.getParameter(id)){p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(value));p->endChangeGesture();}}
float PlateProcessor::get(const String&id)const{return state.getRawParameterValue(id)->load();}
void PlateProcessor::resetToDefaults(){const plate::ParameterTransactions::Write writing(parameterTransactions);currentProgram.store(platebank::count);for(auto*p:getParameters())p->setValueNotifyingHost(p->getDefaultValue());state.state.removeProperty("preset_name",nullptr);state.state.removeProperty("preset_key",nullptr);updateHostDisplay(ChangeDetails().withProgramChanged(true).withNonParameterStateChanged(true));}
StringArray PlateProcessor::presetNames(){StringArray names;for(const auto& preset:platebank::presets)names.add(preset.name);return names;}
void PlateProcessor::loadPreset(int n){if(n<0||n>=platebank::count)return;const plate::ParameterTransactions::Write writing(parameterTransactions);const auto& preset=platebank::presets[n];for(int i=0;i<20;++i)set(platebank::ids[i],preset.values[size_t(i)]);currentProgram.store(n);state.state.setProperty("preset_name",preset.name,nullptr);state.state.setProperty("preset_key","factory:"+String(n+1),nullptr);updateHostDisplay(ChangeDetails().withProgramChanged(true).withNonParameterStateChanged(true));}
AudioProcessorEditor*PlateProcessor::createEditor(){return new PlateEditor(*this);}
AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new PlateProcessor();}
