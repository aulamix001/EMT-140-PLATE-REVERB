#pragma once
#include "VintageLookAndFeel.h"
#include <BinaryData.h>
namespace vintage {
inline const juce::Image& rackBackground(){
 static const auto image=[] {
  auto full=juce::ImageFileFormat::loadFrom(BinaryData::PanelBackground_png,BinaryData::PanelBackground_pngSize).rescaled(2011,782);
  return full.getClippedImage({0,75,2011,707});
 }();return image;
}
class RackLook:public Look {
 juce::Image knob;
public:
 RackLook(){auto source=juce::ImageFileFormat::loadFrom(BinaryData::hardwareconcept_png,BinaryData::hardwareconcept_pngSize);knob=source.getClippedImage({750,337,89,89});}
 void drawRotarySlider(juce::Graphics&g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider&)override{
  const float cx=x+w*.5f,cy=y+h*.5f,r=23.2f;
  juce::Path circle;circle.addEllipse(cx-r,cy-r,2*r,2*r);
  juce::DropShadow(juce::Colours::black.withAlpha(.4f),4,{1,2}).drawForPath(g,circle);
  juce::Graphics::ScopedSaveState save(g);g.setOpacity(1.0f);g.reduceClipRegion(circle);
  // The photographed knob's index is at the clockwise end of its travel.
  g.addTransform(juce::AffineTransform::rotation(start+pos*(end-start)-end,cx,cy));
  g.drawImage(knob,juce::Rectangle<float>(cx-r,cy-r,2*r,2*r));
 }
 void drawButtonBackground(juce::Graphics&g,juce::Button&b,const juce::Colour&c,bool over,bool down)override{
  if(b.getName()=="rack-damper"||b.getName()=="rack-solo"){
   if(down){g.setColour(juce::Colours::black.withAlpha(.25f));g.fillRoundedRectangle(b.getLocalBounds().toFloat(),2);}
   if(over|| (b.getName()=="rack-solo"&&b.getToggleState())){g.setColour(juce::Colour(0xffc5a761).withAlpha(.6f));g.drawRoundedRectangle(b.getLocalBounds().toFloat().reduced(1),2,1);}
   return;
  }
  if(b.getName()=="metal-switch"){
   // The socket is part of the artwork. Only the moving metal lever is drawn.
   const float cx=b.getWidth()*.5f,cy=b.getHeight()*.5f;const bool on=b.getToggleState();
   juce::Rectangle<float> lever(on?cx-2:cx-20,cy-5,23,10);juce::Path path;path.addRoundedRectangle(lever,2);
   juce::DropShadow(juce::Colours::black.withAlpha(.5f),3,{1,2}).drawForPath(g,path);
   g.setGradientFill(juce::ColourGradient(juce::Colour(0xffe8e5d7),cx,cy-5,juce::Colour(0xff56534a),cx,cy+5,false));g.fillPath(path);
   g.setColour(juce::Colour(0xffc8c3b2));g.drawRoundedRectangle(lever,2,.7f);return;
  }
  Look::drawButtonBackground(g,b,c,over,down);
 }
 void drawButtonText(juce::Graphics&g,juce::TextButton&b,bool over,bool down)override{
  if(b.getName()=="rack-damper"||b.getName()=="rack-solo"||b.getName()=="panel-letter")return;
  Look::drawButtonText(g,b,over,down);
 }
};
class RackTimeMeter:public juce::Slider {
public:
 bool active=false;
 RackTimeMeter(){setSliderStyle(LinearVertical);setTextBoxStyle(NoTextBox,false,0,0);setRange(.5,5.5,.1);setDoubleClickReturnValue(true,2);setPopupDisplayEnabled(true,true,nullptr);setTooltip("Reverb time | drag / wheel / arrows; double-click: 2 s");}
 void paint(juce::Graphics&g)override{
  const float y=122.4f-float(getValue()-.5)*15.94f;
  g.setColour(juce::Colours::black.withAlpha(.4f));g.fillRect(37.f,y+2,18.f,2.f);
  g.setColour(juce::Colour(0xffb72f23));g.fillRect(37.f,y,18.f,2.f);
 }
};
}
