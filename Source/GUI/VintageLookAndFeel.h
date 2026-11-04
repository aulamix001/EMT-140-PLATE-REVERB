#pragma once
#include <JuceHeader.h>
namespace vintage {
inline juce::Colour ink(){return juce::Colour(0xff1a1a1a);}
inline void text(juce::Graphics&g,juce::String s,float x,float y,float w,float h,float size=15,juce::Justification j=juce::Justification::centred){g.setColour(ink());g.setFont(juce::Font(juce::FontOptions("DejaVu Sans",size,juce::Font::bold)).withHorizontalScale(.62f));g.drawText(s,juce::Rectangle<float>(x,y,w,h),j,false);}
inline void screw(juce::Graphics&g,float x,float y,float r=5){g.setColour(juce::Colours::black.withAlpha(.35f));g.fillEllipse(x-r,y-r+1,2*r,2*r);g.setGradientFill(juce::ColourGradient(juce::Colour(0xffe5e0cd),x-r,y-r,juce::Colour(0xff55574f),x+r,y+r,false));g.fillEllipse(x-r,y-r,2*r,2*r);g.setColour(juce::Colour(0xff47483f));g.drawLine(x-r*.65f,y+r*.3f,x+r*.65f,y-r*.3f,1.3f);}
class Look:public juce::LookAndFeel_V4 {
public:
 Look(){setColour(juce::Slider::textBoxTextColourId,ink());setColour(juce::Slider::textBoxBackgroundColourId,juce::Colours::transparentBlack);setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff252923));setColour(juce::ComboBox::textColourId,juce::Colour(0xffdbd9c9));setColour(juce::ComboBox::outlineColourId,juce::Colour(0xff606353));}
 void drawRotarySlider(juce::Graphics&g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider&)override{
  float r=juce::jmin(float(w),float(h))*.38f,cx=x+w*.5f,cy=y+h*.5f,a=start+pos*(end-start);
  g.setColour(juce::Colours::black.withAlpha(.45f));g.drawLine(cx+std::sin(a)*(r*.35f)+.7f,cy-std::cos(a)*(r*.35f)+.7f,cx+std::sin(a)*(r*.9f)+.7f,cy-std::cos(a)*(r*.9f)+.7f,2.2f);
  g.setColour(juce::Colour(0xfff8f1dc));g.drawLine(cx+std::sin(a)*(r*.35f),cy-std::cos(a)*(r*.35f),cx+std::sin(a)*(r*.9f),cy-std::cos(a)*(r*.9f),1.8f);

 }
 void drawLinearSlider(juce::Graphics&g,int x,int y,int w,int h,float position,float,float,juce::Slider::SliderStyle,juce::Slider&)override{
  juce::ignoreUnused(x,w);float cy=y+h*.5f;juce::Rectangle<float> thumb(position-7,cy-11,14,22);
  g.setGradientFill(juce::ColourGradient(juce::Colour(0xfff0eadb),position-7,cy,juce::Colour(0xff53564f),position+7,cy,false));g.fillRect(thumb);g.setColour(juce::Colour(0xfff9f5e9));g.drawRect(thumb,.7f);g.setColour(juce::Colour(0xff252720));g.fillRect(position-2,cy-10,4.f,20.f);

 }
 void drawButtonBackground(juce::Graphics&g,juce::Button&b,const juce::Colour&,bool over,bool down)override{
  auto r=b.getLocalBounds().toFloat().reduced(1);bool on=b.getToggleState();if(b.getName()=="panel-letter")return;
  if(b.getName()=="photo-button"){if(on||over||down){g.setColour((on?juce::Colour(0xffe8cc67):juce::Colours::white).withAlpha(down?.28f:.13f));g.fillRoundedRectangle(r,2);g.setColour(juce::Colour(0xffc9af69).withAlpha(.7f));g.drawRoundedRectangle(r,2,.7f);}return;}
  if(b.getName()=="metal-switch"){float cx=r.getCentreX(),cy=r.getCentreY();float tx=on?cx+10:cx-10;g.setColour(juce::Colours::black.withAlpha(.4f));g.drawLine(cx+2,cy+3,tx+2,cy+3,7);g.setGradientFill(juce::ColourGradient(juce::Colour(0xfffaf8ec),cx,cy-3,juce::Colour(0xff52564e),cx,cy+4,false));g.drawLine(cx,cy,tx,cy,6);g.fillEllipse(tx-3,cy-3,6,6);g.setColour(juce::Colours::white.withAlpha(.8f));g.drawLine(cx,cy-2,tx,cy-2,1);return;}

 g.setColour(juce::Colour(0xff34392e));g.fillRoundedRectangle(r,2);r.reduce(2,2);g.setGradientFill(juce::ColourGradient(juce::Colour(on?0xffb5bd8d:0xffe5dfc8).brighter(over?.08f:0),0,r.getY(),juce::Colour(down?0xff747b60:on?0xff7c8955:0xffa8a793),0,r.getBottom(),false));g.fillRoundedRectangle(r,1);g.setColour(juce::Colour(0xff686b5c));g.drawRoundedRectangle(r,1,.7f);
 }
 void drawButtonText(juce::Graphics&g,juce::TextButton&b,bool,bool)override{if(b.getName()=="metal-switch"||b.getName()=="panel-letter"||b.getName()=="photo-button")return;text(g,b.getButtonText(),0,0,float(b.getWidth()),float(b.getHeight()),b.getButtonText().length()>3?10.f:19.f);}
};
class TimeMeter: public juce::Slider {
public:
 bool active=false;TimeMeter(){setSliderStyle(LinearVertical);setTextBoxStyle(NoTextBox,false,0,0);setRange(.5,5.5,.1);setDoubleClickReturnValue(true,2);setTooltip("Reverb time | drag, scroll or use arrow keys | double-click: 2 s");}
 void paint(juce::Graphics&g)override{
  const float values[]={.5f,1,2,3,4,5,5.5f};const float positions[]={119,109,91,71,53,27,11};float py=positions[6];for(int i=0;i<6;++i)if(getValue()<=values[i+1]){py=juce::jmap(float(getValue()),values[i],values[i+1],positions[i],positions[i+1]);break;}g.setColour(juce::Colours::black.withAlpha(.35f));g.drawLine(2,py+1.2f,float(getWidth()-2),py+1.2f,1.4f);g.setColour(juce::Colour(0xffb52d1c));g.drawLine(2,py,float(getWidth()-2),py,1.25f);

 }
};
}
