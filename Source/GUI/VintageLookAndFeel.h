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
  for(int i=0;i<=10;++i){float angle=start+(end-start)*i/10;g.setColour(ink().withAlpha(.75f));g.drawLine(cx+std::sin(angle)*(r+6),cy-std::cos(angle)*(r+6),cx+std::sin(angle)*(r+7.5f),cy-std::cos(angle)*(r+7.5f),1);}
  juce::Path shadow;shadow.addEllipse(cx-r,cy-r,2*r,2*r);juce::DropShadow(juce::Colours::black.withAlpha(.35f),6,{3,4}).drawForPath(g,shadow);
  g.setGradientFill(juce::ColourGradient(juce::Colour(0xff74776b),cx-r,cy-r,juce::Colour(0xff242823),cx+r,cy+r,false));g.fillEllipse(cx-r,cy-r,2*r,2*r);
  g.setColour(juce::Colour(0xff171b18));g.drawEllipse(cx-r,cy-r,2*r,2*r,1.2f);
  for(int i=0;i<64;++i){float t=2*juce::MathConstants<float>::pi*i/64;g.setColour(juce::Colours::black.withAlpha(.35f));g.drawLine(cx+std::sin(t)*(r-2),cy+std::cos(t)*(r-2),cx+std::sin(t)*(r-5),cy+std::cos(t)*(r-5),1);}
  g.setGradientFill(juce::ColourGradient(juce::Colour(0xff252724),cx-3,cy-4,juce::Colour(0xff51534e),cx+r,cy+r,true));g.fillEllipse(cx-r+5,cy-r+5,2*r-10,2*r-10);g.setColour(juce::Colour(0xff777b6e));g.drawEllipse(cx-r+5,cy-r+5,2*r-10,2*r-10,.5f);
  juce::Path shine;shine.addCentredArc(cx,cy,r-3,r-3,0,-1.2f,-.15f,true);g.setColour(juce::Colours::white.withAlpha(.48f));g.strokePath(shine,juce::PathStrokeType(1.6f));g.setColour(juce::Colours::white);g.drawLine(cx+std::sin(a)*(r-10),cy-std::cos(a)*(r-10),cx+std::sin(a)*(r-3),cy-std::cos(a)*(r-3),2);
 }
 void drawLinearSlider(juce::Graphics&g,int x,int y,int w,int h,float position,float,float,juce::Slider::SliderStyle,juce::Slider&)override{
  float cy=y+h*.5f;g.setColour(juce::Colours::black);g.fillRoundedRectangle(float(x),cy-2,float(w),4,2);
  juce::Rectangle<float> thumb(position-9,cy-17,18,34);juce::Path sh;sh.addRectangle(thumb);juce::DropShadow(juce::Colours::black.withAlpha(.35f),4,{2,2}).drawForPath(g,sh);
  g.setGradientFill(juce::ColourGradient(juce::Colour(0xffeeeadd),position-9,cy,juce::Colour(0xff565950),position+9,cy,false));g.fillRect(thumb);g.setColour(juce::Colour(0xfff9f5e9));g.drawRect(thumb,1);g.setColour(juce::Colour(0xff20231f));g.fillRect(position-3,cy-15,6.f,30.f);
 }
 void drawButtonBackground(juce::Graphics&g,juce::Button&b,const juce::Colour&,bool over,bool down)override{
  auto r=b.getLocalBounds().toFloat().reduced(1);bool on=b.getToggleState();if(b.getName()=="panel-letter")return;
  if(b.getName()=="metal-switch") {float cx=r.getCentreX(),cy=r.getCentreY();g.setColour(juce::Colour(0xff73746c));g.fillEllipse(cx-13,cy-13,26,26);g.setColour(juce::Colour(0xffeeeeea));g.drawEllipse(cx-12,cy-12,24,24,2);g.setColour(juce::Colour(0xff181f17));g.fillEllipse(cx-8,cy-8,16,16);g.setGradientFill(juce::ColourGradient(juce::Colour(0xfff4eedc),cx,cy-5,juce::Colour(0xff535650),cx,cy+5,false));g.fillRoundedRectangle(on?cx-3:cx-20,cy-5,23,10,3);g.setColour(juce::Colours::white.withAlpha(.9f));g.drawLine(on?cx-1:cx-18,cy-3,on?cx+17:cx,cy-3,1.3f);g.setColour(juce::Colour(0xffece8d6));g.drawRoundedRectangle(on?cx-3:cx-20,cy-5,23,10,3,.6f);return;}
 g.setColour(juce::Colour(0xff34392e));g.fillRoundedRectangle(r,2);r.reduce(2,2);g.setGradientFill(juce::ColourGradient(juce::Colour(on?0xffb5bd8d:0xffe5dfc8).brighter(over?.08f:0),0,r.getY(),juce::Colour(down?0xff747b60:on?0xff7c8955:0xffa8a793),0,r.getBottom(),false));g.fillRoundedRectangle(r,1);g.setColour(juce::Colour(0xff686b5c));g.drawRoundedRectangle(r,1,.7f);
 }
 void drawButtonText(juce::Graphics&g,juce::TextButton&b,bool,bool)override{if(b.getName()=="metal-switch")return;text(g,b.getButtonText(),0,0,float(b.getWidth()),float(b.getHeight()),b.getButtonText().length()>3?10.f:19.f);}
};
class TimeMeter: public juce::Slider {
public:
 bool active=false;TimeMeter(){setSliderStyle(LinearVertical);setTextBoxStyle(NoTextBox,false,0,0);setRange(.5,5.5,.1);setDoubleClickReturnValue(true,2);setTooltip("Reverb time | drag, scroll or use arrow keys | double-click: 2 s");}
 void paint(juce::Graphics&g)override{
  auto r=getLocalBounds().toFloat();
  g.setColour(juce::Colour(0xff131413));g.fillRoundedRectangle(r,5);
  g.setGradientFill(juce::ColourGradient(juce::Colour(0xffa6a79e),0,0,juce::Colour(0xff171916),float(getWidth()),float(getHeight()),false));g.fillRoundedRectangle(r.reduced(2),4);
  g.setColour(juce::Colour(0xff242623));g.fillRoundedRectangle(r.reduced(5),3);
  juce::Rectangle<float> face(17,27,float(getWidth()-34),float(getHeight()-34));
  g.setColour(juce::Colours::black);g.fillRect(face.expanded(6,5));
  g.setGradientFill(juce::ColourGradient(juce::Colour(0xffe8dfb5).withMultipliedBrightness(active?1.f:.92f),face.getCentreX()-3,face.getCentreY()-8,juce::Colour(0xff89825a),face.getRight()+10,face.getBottom()+12,true));g.fillRect(face);
  for(int i=0;i<=50;++i){float y=face.getBottom()-9-i*(face.getHeight()-28)/50;g.setColour(juce::Colour(0xff4e4b35));float len=i%10==0?15:i%5==0?10:5;g.drawLine(face.getRight()-len,y,face.getRight()-2,y,.6f);}
  const char* labels[]={"max","5","sek.","4","3","2","1","min."};
  const float positions[]={.02f,.12f,.22f,.34f,.48f,.63f,.78f,.91f};
  for(int i=0;i<8;++i)text(g,labels[i],face.getX()+1,face.getY()+positions[i]*face.getHeight(),23,11,9);
  float py=face.getBottom()-9-float((getValue()-.5)/5)*(face.getHeight()-28);g.setColour(juce::Colours::black.withAlpha(.4f));g.drawLine(face.getX(),py+2,face.getRight(),py+2,1.4f);g.setColour(juce::Colour(0xffc43324));g.drawLine(face.getX()-1,py,face.getRight()+1,py,1.5f);
  for(int i=0;i<7;++i){g.setColour(juce::Colours::black.withAlpha(.17f*(1-i/7.f)));g.drawRect(face.reduced(float(i)),1.f);}
  juce::Path reflection;reflection.startNewSubPath(face.getX(),face.getY());reflection.lineTo(face.getRight(),face.getY());reflection.lineTo(face.getX(),face.getBottom()*.7f);reflection.closeSubPath();g.setColour(juce::Colours::white.withAlpha(.17f));g.fillPath(reflection);
  screw(g,r.getCentreX(),11,5);

 }
};
}
