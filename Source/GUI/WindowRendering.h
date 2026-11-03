#pragma once
#include <JuceHeader.h>
namespace vintage {
inline void useSoftwareWindowRendering(juce::Component& component){
 #if JUCE_WINDOWS
 // JUCE 8.0.6's Direct2D window backend joins a presentation thread at teardown.
 // Use the supported GDI renderer for our owned plugin windows instead.
 if(auto*peer=component.getPeer())if(peer->getCurrentRenderingEngine()!=0)peer->setCurrentRenderingEngine(0);
 #else
 juce::ignoreUnused(component);
 #endif
}
class SoftwareLookAndFeel:public juce::LookAndFeel_V4 {
public:
 void preparePopupMenuWindow(juce::Component& window)override{
  juce::LookAndFeel_V4::preparePopupMenuWindow(window);useSoftwareWindowRendering(window);
 }
};
inline juce::LookAndFeel& softwarePopupLook(){static SoftwareLookAndFeel look;return look;}
}
