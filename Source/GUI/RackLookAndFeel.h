#pragma once
#include "VintageLookAndFeel.h"
#include "WindowRendering.h"
#include <BinaryData.h>
namespace vintage {
inline const juce::Image& rackBackground(){
 // One decoded CPU image; only pointers/needles move. No per-frame texture work.
 static const auto image=juce::SoftwareImageType{}.convert(juce::ImageFileFormat::loadFrom(BinaryData::PanelBackground_png,BinaryData::PanelBackground_pngSize)).rescaled(2086,732);
 return image;
}
inline const juce::Image& rackMeter(){
 static const auto image=[] {
  auto source=juce::SoftwareImageType{}.convert(juce::ImageFileFormat::loadFrom(BinaryData::MeterFace_png,BinaryData::MeterFace_pngSize));
  const auto sx=source.getWidth()/2117.f,sy=source.getHeight()/743.f;
  return source.getClippedImage(juce::Rectangle<float>(1724*sx,48*sy,352*sx,312*sy).toNearestInt()).createCopy();
 }();return image;
}
class RackLook:public Look {
public:
 void preparePopupMenuWindow(juce::Component& window)override{Look::preparePopupMenuWindow(window);useSoftwareWindowRendering(window);}
 int getSliderThumbRadius(juce::Slider& slider)override{return slider.getName()=="input-filter"?10:Look::getSliderThumbRadius(slider);}
 void drawRotarySlider(juce::Graphics&g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider&)override{
  const float cx=x+w*.5f,cy=y+h*.5f,a=start+pos*(end-start);
  // Fixed concave body/specular light are in the backplate. Rotate only the index.
  const float sx=std::sin(a),sy=-std::cos(a);
  g.setColour(juce::Colours::black.withAlpha(.45f));g.drawLine(cx+sx*12+.6f,cy+sy*12+.8f,cx+sx*21+.6f,cy+sy*21+.8f,2.2f);
  g.setColour(juce::Colour(0xfff4efdf));g.drawLine(cx+sx*12,cy+sy*12,cx+sx*21,cy+sy*21,1.7f);
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
  const double values[]={.5,1,2,3,4,5,5.5};const float positions[]={132.5f,114,99,88.5f,78,51.4f,38.6f};
  float y=positions[6];for(int i=0;i<6;++i)if(getValue()<=values[i+1]){y=juce::jmap(float(getValue()),float(values[i]),float(values[i+1]),positions[i],positions[i+1]);break;}
  g.setColour(juce::Colours::black.withAlpha(.4f));g.fillRect(35.f,y+1.3f,21.f,1.7f);
  g.setColour(juce::Colour(0xffb72f23));g.fillRect(35.f,y,21.f,1.5f);
 }
};
}
