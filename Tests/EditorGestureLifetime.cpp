#include "PluginEditor.h"
#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <stdexcept>
#if JUCE_WINDOWS
// Keep this order: dbghelp requires Windows declarations.
#include <windows.h>
#include <dbghelp.h>
#endif

class LifetimeApp final : public juce::JUCEApplication, private juce::Timer,
                          private juce::AudioProcessorParameter::Listener {
    std::unique_ptr<PlateProcessor> processor;
    std::unique_ptr<PlateEditor> editor;
    std::thread audio, watchdog;
    std::atomic<bool> processing{false}, finished{false};
    std::atomic<int64_t> checkpoint{0};
    std::atomic<int> blocks{0};
    int instances=0, cycles=0, begins=0, ends=0;
    juce::MemoryBlock saved;
    void check(bool ok,const char* message) { if(!ok)throw std::runtime_error(message); }
    void audioProcessorParameterChanged(int,float) {}
    void parameterValueChanged(int,float) override {}
    void parameterGestureChanged(int,bool start) override {start?++begins:++ends;}
    juce::Component* find(juce::Component& root,const juce::String& name) {
        if(root.getName()==name)return &root;
        for(auto* child:root.getChildren())if(auto* result=find(*child,name))return result;
        return nullptr;
    }
    void stopAudio() {
        processing.store(false);
        if(audio.joinable())audio.join(); // Host completes in-flight callback BEFORE destruction.
    }
    void open() {
        editor=std::make_unique<PlateEditor>(*processor);
        editor->addToDesktop(0);editor->setVisible(true);
        check(editor->getPeer()!=nullptr,"native peer missing");
        if(instances==0&&cycles==0) {
            juce::Image image(juce::Image::ARGB,8,8,true);
            {juce::Graphics g(image);g.fillAll(juce::Colours::red);}
            check(image.getPixelAt(2,2)==juce::Colours::red,"image draw/read");
           #if JUCE_WINDOWS && PLATE_COMPATIBLE_GRAPHICS
            juce::Component plain;plain.setSize(30,30);plain.addToDesktop(0);
            check(plain.getPeer()->getCurrentRenderingEngine()==0,"initial plain peer must be GDI");
            check(image.getPixelData()->createType()->getTypeID()==juce::SoftwareImageType().getTypeID(),"CPU image factory");
            const auto before=processor->state.copyState();
            plateui::EditorGraphics adapter(*editor,processor->softwareGraphics);
            processor->softwareGraphics=false;adapter.apply();
            check(editor->getPeer()->getCurrentRenderingEngine()==1,"GPU peer selection");
            processor->softwareGraphics=true;adapter.apply();
            check(editor->getPeer()->getCurrentRenderingEngine()==0,"GDI peer selection");
            check(before.isEquivalentTo(processor->state.copyState()),"graphics changed audio parameters");
           #endif
        }
    }
    void timerCallback() override {
        try {
            checkpoint.store(juce::Time::currentTimeMillis());
            if(!processor) {
                processor=std::make_unique<PlateProcessor>();
                if(saved.getSize())processor->setStateInformation(saved.getData(),int(saved.getSize()));
                const double rates[]={44100,48000,96000,192000};
                processor->prepareToPlay(rates[instances%4],128);
                processor->state.getParameter("width")->addListener(this);
                processing=true;
                auto* p=processor.get();
                audio=std::thread([this,p]{
                    juce::AudioBuffer<float> buffer(2,128),zero(2,0);juce::MidiBuffer midi;
                    p->setNonRealtime(true);
                    while(processing.load()) {
                        buffer.clear();buffer.setSample(0,0,.1f);p->processBlock(buffer,midi);
                        p->processBlock(zero,midi);++blocks;
                        std::this_thread::yield();
                    }
                });
                cycles=0;open();return;
            }
            // Route a real down/drag through ComponentPeer. This creates JUCE's
            // capture, Slider::ScopedDragNotification, attachment gesture and popup.
            auto* slider=dynamic_cast<juce::Slider*>(find(*editor,"width"));
            check(slider!=nullptr,"width slider");
            auto* peer=editor->getPeer();
            auto position=editor->getLocalPoint(slider,slider->getLocalBounds().getCentre()).toFloat();
            const auto now=juce::Time::currentTimeMillis();
            peer->handleMouseEvent(juce::MouseInputSource::mouse,position,{},0,0,now);
            peer->handleMouseEvent(juce::MouseInputSource::mouse,position,juce::ModifierKeys::leftButtonModifier,0,0,now+1);
            peer->handleMouseEvent(juce::MouseInputSource::mouse,position.translated(0,-18),juce::ModifierKeys::leftButtonModifier,0,0,now+2);
            check(begins>ends,"real attachment gesture did not start");
            check(slider->getCurrentPopupDisplay()!=nullptr,"real drag popup did not open");
            editor.reset();
            check(begins==ends,"gesture left open after editor destruction");
            check(!juce::Desktop::getInstance().getMainMouseSource().isDragging(),"mouse capture survived editor");
            if(++cycles<3){open();return;}
            stopAudio();
            processor->getStateInformation(saved);
            processor->state.getParameter("width")->removeListener(this);
            processor->releaseResources();processor.reset();
            if(++instances==100) {
                std::cout<<"PASS: 100 instances, 300 native editor/drag/popup cycles, "<<blocks.load()
                         <<" audio callbacks; gesture pairs "<<begins<<"/"<<ends<<"\n";
                finished=true;quit();
            }
        }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";setApplicationReturnValue(1);finished=true;quit();}
    }
public:
    const juce::String getApplicationName() override{return "PlateLifetime";}
    const juce::String getApplicationVersion() override{return "0.2";}
    void initialise(const juce::String&) override {
        checkpoint=juce::Time::currentTimeMillis();
        watchdog=std::thread([this]{while(!finished.load()){
            if(juce::Time::currentTimeMillis()-checkpoint.load()>30000){
                std::cerr<<"FAIL: lifecycle watchdog 30 seconds without progress\n";
               #if JUCE_WINDOWS
                auto file=CreateFileW(L"plate-lifecycle-hang.dmp",GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
                if(file!=INVALID_HANDLE_VALUE){MiniDumpWriteDump(GetCurrentProcess(),GetCurrentProcessId(),file,MiniDumpNormal,nullptr,nullptr,nullptr);CloseHandle(file);}
               #endif
                std::_Exit(2);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }});
        startTimer(10);
    }
    void shutdown() override {
        stopTimer();editor.reset();stopAudio();processor.reset();finished=true;
        if(watchdog.joinable())watchdog.join();
    }
};
START_JUCE_APPLICATION(LifetimeApp)
