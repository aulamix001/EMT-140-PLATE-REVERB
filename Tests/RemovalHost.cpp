#define DONT_SET_USING_JUCE_NAMESPACE 1
#include <JuceHeader.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>
// Loads the actual VST3 in a separate host executable: no Plate140 static linkage.
class RemovalHost:public juce::JUCEApplication,private juce::Timer {
 juce::VST3PluginFormat format;juce::PluginDescription description;juce::Component window;
 std::unique_ptr<juce::AudioPluginInstance> plugin;std::unique_ptr<juce::AudioProcessorEditor> editor;
 std::thread audio;std::atomic<bool> running{false},badAudio{false};std::atomic<int> blocks{0};
 int cycle=0,phase=0,blocksAtStart=0;
public:
 const juce::String getApplicationName()override{return "PlateRemovalHost";}
 const juce::String getApplicationVersion()override{return "1";}
 void initialise(const juce::String&)override{
  auto path=juce::SystemStats::getEnvironmentVariable("PLATE140_TEST_VST3",{});
  if(path.isEmpty()){fail("PLATE140_TEST_VST3 is required");return;}
  std::cout<<"SCAN "<<path<<std::endl;
  juce::OwnedArray<juce::PluginDescription> types;format.findAllTypesForFile(types,path);
  if(types.isEmpty()){fail("VST3 scan returned no plugin");return;}
  description=*types[0];window.setBounds(0,0,1565,597);window.addToDesktop(0);
  if(auto*peer=window.getPeer())peer->setCurrentRenderingEngine(0);
  window.setVisible(true);
  startTimer(30);
 }
 void shutdown()override{stopTimer();removeInstance();window.removeFromDesktop();}
private:
 void fail(const char*message){std::cerr<<"FAIL: "<<message<<std::endl;setApplicationReturnValue(1);quit();}
 void removeInstance(){
  if(editor){std::cout<<"REMOVE EDITOR "<<cycle<<std::endl;plugin->editorBeingDeleted(editor.get());editor.reset();}
  running=false;if(audio.joinable())audio.join();
  if(plugin){std::cout<<"RELEASE PROCESSOR "<<cycle<<std::endl;plugin->releaseResources();plugin.reset();}
 }
 void timerCallback()override{
  if(phase==0){
   stopTimer();std::cout<<"CREATE "<<cycle<<std::endl;
   format.createPluginInstanceAsync(description,48000,128,[this](auto instance,const auto&error){
    if(!instance){std::cerr<<error<<std::endl;fail("VST3 creation failed");return;}
    plugin=std::move(instance);plugin->prepareToPlay(48000,128);
    plugin->setCurrentProgram((cycle*7)%plugin->getNumPrograms());
    editor.reset(plugin->createEditorIfNeeded());if(!editor){fail("editor unavailable");return;}
    editor->setBounds(window.getLocalBounds());window.addAndMakeVisible(*editor);
    blocksAtStart=blocks;running=true;
    audio=std::thread([this]{
     juce::AudioBuffer<float> buffer(2,128);juce::MidiBuffer midi;
     while(running){buffer.clear();buffer.setSample(0,0,.1f);
      {const juce::ScopedLock lock(plugin->getCallbackLock());plugin->processBlock(buffer,midi);}
      for(int c=0;c<2;++c)for(int i=0;i<128;++i)if(!std::isfinite(buffer.getSample(c,i)))badAudio=true;
      ++blocks;std::this_thread::sleep_for(std::chrono::milliseconds(1));
     }
    });phase=1;startTimer(50);
   });return;
  }
  if(phase==1){
   plugin->setCurrentProgram((cycle*11+3)%plugin->getNumPrograms());
   phase=2;return;
  }
  if(phase==2){
   if(blocks<=blocksAtStart||badAudio){fail("audio did not run cleanly alongside editor");return;}
   juce::Component::SafePointer<juce::Component> oldEditor(editor.get());
   removeInstance();if(oldEditor!=nullptr){fail("editor survived removal");return;}
   std::cout<<"REMOVED "<<cycle<<std::endl;phase=3;return;
  }
  // Require another message-thread timer dispatch after removal, not only a destructor return.
  std::cout<<"UI RESPONSIVE "<<cycle<<std::endl;
  if(++cycle==12){std::cout<<"PASS: 12 actual VST3 insert/preset/play/remove cycles; UI responsive; audio finite"<<std::endl;quit();return;}
  phase=0;
 }
};
START_JUCE_APPLICATION(RemovalHost)
