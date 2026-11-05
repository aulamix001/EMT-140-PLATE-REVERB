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
    juce::MemoryBlock saved;juce::int64 openedAt=0;int nativeWait=0;
    void check(bool ok,const char* message) { if(!ok)throw std::runtime_error(message); }
    void audioProcessorParameterChanged(int,float) {}
    void parameterValueChanged(int,float) override {}
    void parameterGestureChanged(int,bool start) override {start?++begins:++ends;}
    juce::Component* find(juce::Component& root,const juce::String& name) {
        if(root.getName()==name)return &root;
        for(auto* child:root.getChildren())if(auto* result=find(*child,name))return result;
        return nullptr;
    }
    void mark(const char* phase){checkpoint.store(juce::Time::currentTimeMillis());std::cout<<"checkpoint "<<instances<<"/"<<cycles<<" "<<phase<<std::endl;}
    void stopAudio() {
        processing.store(false);
        if(audio.joinable())audio.join(); // Host completes in-flight callback BEFORE destruction.
    }
    void open() {
        mark("editor.open.begin");
        editor=std::make_unique<PlateEditor>(*processor);
        editor->setTopLeftPosition(20,20);editor->addToDesktop(0);editor->setVisible(true);editor->toFront(true);
        check(editor->getPeer()!=nullptr,"native peer missing");
        if(instances==0&&cycles==0) {
            juce::Image image(juce::Image::ARGB,8,8,true);
            {juce::Graphics g(image);g.fillAll(juce::Colours::red);}
            check(image.getPixelAt(2,2)==juce::Colours::red,"image draw/read");
            const auto parameterState=processor->state.copyState();
            const auto pixelsBefore=editor->createComponentSnapshot(editor->getLocalBounds());
            mark("graphics.gpu.begin");editor->setSoftwareGraphics(false);mark("graphics.gpu.end");
           #if JUCE_WINDOWS
            check(editor->getPeer()->getCurrentRenderingEngine()==1,"GPU peer selection");
           #endif
            const auto pixelsAfter=editor->createComponentSnapshot(editor->getLocalBounds());
            check(pixelsBefore.getBounds()==pixelsAfter.getBounds(),"graphics switch changed editor dimensions");
            int changed=0;for(int y=0;y<pixelsBefore.getHeight();++y)for(int x=0;x<pixelsBefore.getWidth();++x)
                if(pixelsBefore.getPixelAt(x,y)!=pixelsAfter.getPixelAt(x,y))++changed;
            std::cout<<"RGBA snapshot comparison: "<<changed<<" changed pixels / "<<pixelsBefore.getWidth()*pixelsBefore.getHeight()
                     <<" (platform "<<juce::SystemStats::getOperatingSystemName()<<")\n";
            mark("graphics.cpu.begin");editor->setSoftwareGraphics(true);mark("graphics.cpu.end");
           #if JUCE_WINDOWS
            check(editor->getPeer()->getCurrentRenderingEngine()==0,"GDI peer selection");
           #endif
            check(parameterState.isEquivalentTo(processor->state.copyState()),"graphics changed audio parameters");
           #if ! JUCE_WINDOWS
            check(changed==0,"Linux appearance changed when storing graphics preference");
           #endif

           #if JUCE_WINDOWS && PLATE_COMPATIBLE_GRAPHICS
            juce::Component plain;plain.setSize(30,30);plain.addToDesktop(0);
            check(plain.getPeer()->getCurrentRenderingEngine()==0,"initial plain peer must be GDI");
            check(image.getPixelData()->createType()->getTypeID()==juce::SoftwareImageType().getTypeID(),"CPU image factory");
           #endif
        }
       #if JUCE_WINDOWS
        // Native hit-testing must see the test window, not a runner console.
        editor->getPeer()->setAlwaysOnTop(true);
        editor->toFront(true);
        editor->repaint();
       #else
        editor->toFront(true);
       #endif
        openedAt=juce::Time::currentTimeMillis();nativeWait=0;mark("editor.open.end");
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
            editor->toFront(true);
            const auto logicalPosition=editor->getLocalPoint(slider,slider->getLocalBounds().getCentre()).toFloat();
            auto position=logicalPosition*editor->getDesktopScaleFactor();
            juce::Desktop::getInstance().getMainMouseSource().setScreenPosition(slider->getScreenBounds().getCentre().toFloat());
            if(!peer->contains(position.roundToInt(),true)) {
                if(nativeWait++%100==0)std::cerr<<"Native hit-test waiting: instance="<<instances<<" cycle="<<cycles
                         <<" peer position="<<position.toString()<<" editor bounds="<<editor->getScreenBounds().toString()
                         <<" scale="<<editor->getDesktopScaleFactor()<<std::endl;
                check(juce::Time::currentTimeMillis()-openedAt<5000,"native window did not become available for real input");
                editor->repaint();return;
            }
            const auto now=juce::Time::currentTimeMillis();
            peer->handleMouseEvent(juce::MouseInputSource::mouse,position,{},0,0,now);
            peer->handleMouseEvent(juce::MouseInputSource::mouse,position,juce::ModifierKeys::leftButtonModifier,0,0,now+1);
            peer->handleMouseEvent(juce::MouseInputSource::mouse,position.translated(0,-18),juce::ModifierKeys::leftButtonModifier,0,0,now+2);
            if(begins<=ends){auto* hit=juce::Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();
                std::cerr<<"Gesture diagnostic: instance="<<instances<<" cycle="<<cycles<<" begins="<<begins<<" ends="<<ends
                         <<" target="<<(hit?hit->getName():juce::String("null"))<<"\n";}
            check(begins>ends,"real attachment gesture did not start");
            check(slider->getCurrentPopupDisplay()!=nullptr,"real drag popup did not open");
            mark("editor.close.begin");editor.reset();mark("editor.close.end");
            check(begins==ends,"gesture left open after editor destruction");
            check(!juce::Desktop::getInstance().getMainMouseSource().isDragging(),"mouse capture survived editor");
            if(++cycles<3){open();return;}
            mark("audio.join.begin");stopAudio();mark("audio.join.end");
            processor->getStateInformation(saved);
            processor->state.getParameter("width")->removeListener(this);
            mark("processor.free.begin");processor->releaseResources();processor.reset();mark("processor.free.end");
            if(++instances==100) {
                std::cout<<"PASS: 100 instances, 300 native editor/drag/popup cycles, "<<blocks.load()
                         <<" audio callbacks; gesture pairs "<<begins<<"/"<<ends<<std::endl;
                finished=true;quit();
            }
        }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";setApplicationReturnValue(1);finished=true;quit();}
    }
public:
    bool moreThanOneInstanceAllowed() override{return true;}
    const juce::String getApplicationName() override{return "PlateLifetime";}
    const juce::String getApplicationVersion() override{return "0.2";}
    void initialise(const juce::String& args) override {
       #if JUCE_WINDOWS
        auto tokens=juce::StringArray::fromTokens(args,true);
        if(tokens.size()==3&&tokens[0]=="--dump-process") {
            const auto pid=DWORD(tokens[1].getIntValue());
            auto process=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|PROCESS_DUP_HANDLE,FALSE,pid);
            const auto path=tokens[2].unquoted();
            auto file=CreateFileW(path.toWideCharPointer(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
            const bool ok=process&&file!=INVALID_HANDLE_VALUE&&MiniDumpWriteDump(process,pid,file,MiniDumpNormal,nullptr,nullptr,nullptr);
            std::cout<<"external minidump "<<(ok?"PASS":"FAIL")<<" error="<<(ok?0:GetLastError())<<std::endl;
            if(file!=INVALID_HANDLE_VALUE)CloseHandle(file);if(process)CloseHandle(process);
            setApplicationReturnValue(ok?0:1);quit();return;
        }
       #endif
        if(args=="--scale125")juce::Desktop::getInstance().setGlobalScaleFactor(1.25f);
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
        mark("shutdown.begin");stopTimer();editor.reset();stopAudio();processor.reset();finished=true;
        if(watchdog.joinable())watchdog.join();mark("shutdown.end");
    }
};
START_JUCE_APPLICATION(LifetimeApp)
