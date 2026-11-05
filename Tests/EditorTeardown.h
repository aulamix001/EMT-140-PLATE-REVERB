#pragma once
#include "PluginEditor.h"
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>

class EditorTeardownRegression:private juce::Timer {
 PlateProcessor processor;std::unique_ptr<PlateEditor> editor;
 std::atomic<bool> running{true},badAudio{false};std::atomic<int> blocks{0};std::thread audio;
 std::function<void(bool)> finished;int cycle=0,phase=0,blocksAtStart=0;
 static juce::Component* find(juce::Component&root,const juce::String&name){
  if(root.getName()==name)return &root;
  for(auto*child:root.getChildren())if(auto*result=find(*child,name))return result;
  return nullptr;
 }
 void finish(bool ok,const char*message){stopTimer();editor.reset();running=false;if(audio.joinable())audio.join();processor.releaseResources();std::cout<<(ok?"PASS: ":"FAIL: ")<<message<<std::endl;finished(ok);}
public:
 explicit EditorTeardownRegression(std::function<void(bool)> callback):finished(std::move(callback)){
  processor.prepareToPlay(48000,128);
  audio=std::thread([this]{juce::AudioBuffer<float> buffer(2,128);juce::MidiBuffer midi;
   while(running){buffer.clear();buffer.setSample(0,0,.1f);processor.processBlock(buffer,midi);
    for(int c=0;c<2;++c)for(int i=0;i<128;++i)if(!std::isfinite(buffer.getSample(c,i)))badAudio=true;
    ++blocks;std::this_thread::sleep_for(std::chrono::milliseconds(1));}
  });startTimer(50);
 }
 ~EditorTeardownRegression()override{stopTimer();editor.reset();running=false;if(audio.joinable())audio.join();processor.releaseResources();}
private:
 void timerCallback()override{
  if(phase==0){
   processor.loadPreset((cycle*7)%platebank::count);blocksAtStart=blocks;
   editor=std::make_unique<PlateEditor>(processor);editor->addToDesktop(0);editor->setVisible(true);
   if(cycle%2){auto*button=dynamic_cast<juce::Button*>(find(*editor,"preset-browser-open"));if(!button){finish(false,"browser button missing");return;}button->onClick();}
   #if JUCE_WINDOWS
   if(!editor->getPeer()||editor->getPeer()->getCurrentRenderingEngine()!=0){finish(false,"editor is not using software rendering");return;}
   #endif
   if(vintage::rackBackground().getPixelData()->createType()->getTypeID()!=juce::SoftwareImageType{}.getTypeID()){
    finish(false,"cached artwork retains native GPU image resources");return;}
   phase=1;return;
  }
  if(phase==1){
   juce::Component::SafePointer<juce::Component> browser;
   auto&desktop=juce::Desktop::getInstance();
   for(int i=0;i<desktop.getNumComponents();++i)if(auto*w=dynamic_cast<PresetWindow*>(desktop.getComponent(i))){
    browser=w;auto*search=dynamic_cast<juce::TextEditor*>(find(*w,"preset-search"));
    if(search)search->setText("Vocal"); // Leave an async search notification queued during removal.
    #if JUCE_WINDOWS
    if(w->getPeer()->getCurrentRenderingEngine()!=0){finish(false,"browser is not using software rendering");return;}
    #endif
   }
   if((cycle%2)&&!browser){finish(false,"preset window missing before removal");return;}
   juce::Component::SafePointer<juce::Component> oldEditor(editor.get());
   std::cout<<"CLOSE EDITOR/BROWSER "<<cycle<<std::endl;editor.reset();
   if(oldEditor||browser){finish(false,"UI survived editor removal");return;}
   phase=2;return;
  }
  // Another timer dispatch after deletion proves that the UI thread remains live.
  if(blocks<=blocksAtStart||badAudio){finish(false,"audio failed during removal");return;}
  if(++cycle==12){finish(true,"12 editor/browser removal cycles with pending search updates; live finite audio; UI responsive; CPU artwork");return;}
  phase=0;
 }
};
