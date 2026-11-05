#include "PluginEditor.h"
using namespace juce;using namespace vintage;
PlateEditor::PlateEditor(PlateProcessor&proc):AudioProcessorEditor(proc),p(proc),graphics(*this,proc.softwareGraphics){setLookAndFeel(&look);addAndMakeVisible(panel);panel.setInterceptsMouseClicks(false,true);

 for(int i=0;i<10;++i){auto&k=knobs[i];const auto&spec=plateui::dials[size_t(i)];const auto*id=spec.id;k.dialScale=spec.scale;panel.addAndMakeVisible(k);k.setSliderStyle(Slider::RotaryVerticalDrag);k.setTextBoxStyle(Slider::NoTextBox,false,0,0);k.setRotaryParameters(MathConstants<float>::pi*1.25f,MathConstants<float>::pi*2.75f,true);const auto*parameter=p.state.getParameter(id);k.setDoubleClickReturnValue(true,parameter->convertFrom0to1(parameter->getDefaultValue()));k.setPopupDisplayEnabled(true,true,this);k.setName(id);k.setTooltip(String(spec.help)+" | drag / wheel / arrow keys; double-click: parameter default");sliders.push_back(std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(p.state,id,k));}
 for(int i=0;i<3;++i){panel.addAndMakeVisible(times[i]);times[i].setPopupDisplayEnabled(true,true,this);auto id=String("reverb_time_")+String::charToString(char('a'+i));times[i].setName(id);sliders.push_back(std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(p.state,id,times[i]));panel.addAndMakeVisible(letters[i]);letters[i].setName("panel-letter");letters[i].setComponentID("plate-"+String(i));letters[i].setButtonText(String::charToString(char('A'+i)));letters[i].onClick=[this,i]{p.set("plate_select",float(i));};for(int j=0;j<2;++j){auto&b=dampers[i*2+j];panel.addAndMakeVisible(b);b.setName("rack-damper");b.setButtonText(j?"+":"-");b.onClick=[this,i,j,id]{p.set(id,float(jlimit(.5,5.5,times[i].getValue()+(j?.1:-.1))));};}}
 power.setName("metal-switch");eq.setName("metal-switch");solo.setName("rack-solo");power.setComponentID("power");eq.setComponentID("eq_enable");solo.setComponentID("wet_solo");for(auto*b:{&power,&eq,&solo}){panel.addAndMakeVisible(b);b->setClickingTogglesState(true);}buttons.push_back(std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(p.state,"power",power));buttons.push_back(std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(p.state,"eq_enable",eq));buttons.push_back(std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(p.state,"wet_solo",solo));
 panel.addAndMakeVisible(filter);filter.setName("input-filter");filter.setSliderStyle(Slider::LinearHorizontal);filter.setTextBoxStyle(Slider::NoTextBox,false,0,0);filter.setRange(0,9,1);filter.setColour(Slider::trackColourId,Colour(0xff33392b));filter.setColour(Slider::backgroundColourId,Colour(0xff25291e));filter.setColour(Slider::thumbColourId,Colour(0xff737b60));filter.setTooltip(String::fromUTF8("OFF \xc2\xb7 M90 \xc2\xb7 M125 \xc2\xb7 M180 \xc2\xb7 M250 \xc2\xb7 M270 \xc2\xb7 M360 \xc2\xb7 E-4 \xc2\xb7 E-10 \xc2\xb7 E-16"));filter.onValueChange=[this]{int v=int(filter.getValue());p.set("input_filter_model",float(v==0?2:v<7?1:0));if(v>0&&v<7)p.set("input_filter_martech_hz",float(v-1));if(v>=7)p.set("input_filter_emt_db",float(v-7));};
 for(auto*b:{&prev,&next,&ab,&reset,&save,&load})addAndMakeVisible(b);addAndMakeVisible(presets);auto names=PlateProcessor::presetNames();for(int i=0;i<names.size();++i){presets.addItem(names[i],i+1);}presets.setTextWhenNothingSelected("CUSTOM / FACTORY PRESETS");presets.onChange=[this]{presetIndex=presets.getSelectedId()-1;if(presetIndex>=0)p.loadPreset(presetIndex);};prev.onClick=[this]{p.loadPreset(p.getCurrentProgram()>=platebank::count?platebank::count-1:(p.getCurrentProgram()+platebank::count-1)%platebank::count);updatePresetCaption();};next.onClick=[this]{p.loadPreset(p.getCurrentProgram()>=platebank::count?0:(p.getCurrentProgram()+1)%platebank::count);updatePresetCaption();};reset.onClick=[this]{p.reset();presets.setSelectedId(0,dontSendNotification);};alternate=p.state.copyState();ab.onClick=[this]{auto current=p.state.copyState();p.state.replaceState(alternate);alternate=current;bState=!bState;ab.setButtonText(bState?"B / A":"A / B");};save.onClick=[this]{chooseFile(true);};load.onClick=[this]{chooseFile(false);};
 toolbarLook.setColour(TextButton::buttonColourId,Colour(0xff242424));toolbarLook.setColour(TextButton::buttonOnColourId,Colour(0xff4e4933));toolbarLook.setColour(TextButton::textColourOffId,Colour(0xffdddddd));toolbarLook.setColour(TextButton::textColourOnId,Colour(0xffeed280));
 for(auto*b:{&prev,&next,&browse,&inButton,&aButton,&bButton,&copyButton,&pasteButton,&learnButton,&moreButton}){addAndMakeVisible(b);b->setLookAndFeel(&toolbarLook);}
 for(auto*b:{&ab,&reset,&save,&load})b->setVisible(false);presets.setVisible(false);inButton.setClickingTogglesState(true);buttons.push_back(std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(p.state,"power",inButton));
 browse.setName("preset-browser-open");copyButton.setName("preset-copy");pasteButton.setName("preset-paste");aButton.setName("compare-a");bButton.setName("compare-b");stateA=p.state.copyState();stateB=stateA.createCopy();browse.onClick=[this]{openPresets();};
 aButton.onClick=[this]{if(usingB){stateB=p.state.copyState();p.restoreState(stateA);usingB=false;}updatePresetCaption();};
 bButton.onClick=[this]{if(!usingB){stateA=p.state.copyState();p.restoreState(stateB);usingB=true;}updatePresetCaption();};
 copyButton.onClick=[this]{auto v=p.state.copyState();v.setProperty("version",1,nullptr);SystemClipboard::copyTextToClipboard(v.createXml()->toString());};
 pasteButton.onClick=[this]{auto xml=XmlDocument::parse(SystemClipboard::getTextFromClipboard());if(!xml||!xml->hasTagName("PLATE140"))return;auto v=ValueTree::fromXml(*xml);if(!PresetLibrary::validState(v))return;p.restoreState(v);updatePresetCaption();};
 const auto menuOwner = Component::SafePointer<PlateEditor>(this);
 moreButton.onClick=[this,menuOwner]{
  PopupMenu menu;menu.setLookAndFeel(&softwarePopupLook());menu.addItem(1,"Factory reset");menu.addItem(2,"Export current XML...");menu.addItem(3,"Import XML...");
 #if JUCE_WINDOWS
  menu.addSectionHeader("Graphics");menu.addItem(4,"Compatibility (CPU / GDI)",true,p.softwareGraphics.load());menu.addItem(5,"Accelerated (GPU / Direct2D)",true,!p.softwareGraphics.load());
 #endif
  menu.showMenuAsync(PopupMenu::Options().withTargetComponent(&moreButton),[menuOwner](int id){
   if(!menuOwner||menuOwner->closing)return;
   if(id==1)menuOwner->p.reset();
   if(id==2)menuOwner->chooseFile(true);
   if(id==3)menuOwner->chooseFile(false);
   if(id==4||id==5)menuOwner->setSoftwareGraphics(id==4);
  });
 };
 learnButton.onClick=[this]{PopupMenu menu;menu.setLookAndFeel(&softwarePopupLook());for(int i=0;i<20;++i)menu.addItem(i+1,String(platebank::ids[i])+" = "+String(p.get(platebank::ids[i]),2));menu.showMenuAsync(PopupMenu::Options().withTargetComponent(&learnButton),[](int id){if(id>0&&id<=20)SystemClipboard::copyTextToClipboard(platebank::ids[id-1]);});};learnButton.setTooltip("Host automation mapping: select a parameter to copy its stable ID. Not MIDI Learn.");
 setResizable(true,true);setResizeLimits(1043,398,2086,796);getConstrainer()->setFixedAspectRatio(1043.0/398);setSize(1565,597);updatePresetCaption();timerCallback();startTimerHz(30);
}
void PlateEditor::setSoftwareGraphics(bool software){p.softwareGraphics.store(software);graphics.apply();p.updateHostDisplay(AudioProcessor::ChangeDetails().withNonParameterStateChanged(true));}
PlateEditor::~PlateEditor(){
 closing=true;stopTimer();
 // Cancel async UI and parameter callbacks while their controls still exist.
 chooser.reset();plateui::finishInteractions(*this);buttons.clear();sliders.clear();
 if(presetWindow)presetWindow->setVisible(false);
 presetWindow.reset();
 for(auto*b:{&prev,&next,&browse,&inButton,&aButton,&bButton,&copyButton,&pasteButton,&learnButton,&moreButton})b->setLookAndFeel(nullptr);
 setLookAndFeel(nullptr);
}
void PlateEditor::chooseFile(bool saving){chooser=std::make_unique<FileChooser>(saving?"Export PLATE 140 preset":"Import PLATE 140 preset",File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("Plate140.xml"),"*.xml");chooser->launchAsync((saving?FileBrowserComponent::saveMode|FileBrowserComponent::warnAboutOverwriting:FileBrowserComponent::openMode)|FileBrowserComponent::canSelectFiles,[safe=Component::SafePointer<PlateEditor>(this),saving](const FileChooser&fc){if(!safe||safe->closing||safe->chooser.get()!=&fc)return;auto f=fc.getResult();if(f==File())return;if(saving){auto tree=safe->p.state.copyState();tree.setProperty("version",1,nullptr);if(auto xml=tree.createXml())xml->writeTo(f);}else if(auto xml=XmlDocument::parse(f)){if(PresetLibrary::validState(ValueTree::fromXml(*xml))){safe->p.restoreState(ValueTree::fromXml(*xml));safe->presets.setSelectedId(0,dontSendNotification);}}});}
void PlateEditor::resized(){if(closing)return;float s=getWidth()/1043.f;panel.setTransform(AffineTransform::scale(s));panel.setBounds(0,32,1043,366);
 for(size_t i=0;i<knobs.size();++i)knobs[i].setBounds(plateui::dials[i].x,plateui::dials[i].y,64,64);
 for(int i=0;i<3;++i){times[i].setBounds(117+i*84,54,76,151);letters[i].setBounds(29+i*26,84,25,38);dampers[2*i].setBounds(126+i*84,215,25,27);dampers[2*i+1].setBounds(159+i*84,215,25,27);}
 filter.setBounds(126,282,226,30);power.setBounds(505,242,35,27);eq.setBounds(746,57,39,23);solo.setBounds(951,238,47,26);
 float ts=getWidth()/1043.f;auto place=[&](Component&c,int x,int width){c.setBounds(roundToInt(x*ts),roundToInt(5*ts),roundToInt(width*ts),roundToInt(22*ts));};place(prev,6,22);place(next,29,22);place(browse,54,365);place(inButton,424,27);place(aButton,456,25);place(bButton,483,25);place(copyButton,513,45);place(pasteButton,560,47);place(learnButton,935,64);place(moreButton,1004,32);}

void PlateEditor::timerCallback(){if(closing)return;graphics.apply();int active=int(p.get("plate_select"));for(int i=0;i<3;++i){times[i].active=i==active;letters[i].setToggleState(i==active,dontSendNotification);times[i].repaint();}int model=int(p.get("input_filter_model")),index=model==2?0:model==1?1+int(p.get("input_filter_martech_hz")):7+int(p.get("input_filter_emt_db"));if(!filter.isMouseButtonDown())filter.setValue(index,dontSendNotification);eq.setButtonText(p.get("eq_enable")>.5?"IN":"OUT");power.setButtonText(p.get("power")>.5?"ON":"OFF");vu+=.15f*(p.meter.load()-vu);updatePresetCaption();repaint();}
void PlateEditor::paint(Graphics&g){if(closing)return;g.fillAll(Colour(0xffa8744a));g.addTransform(AffineTransform::scale(getWidth()/1043.f));g.setColour(Colour(0xff393939));g.fillRect(0,0,1043,32);bool aux=p.get("wet_solo")>.5f;g.setColour(aux?Colour(0xff9acace):Colour(0xffe4c47b));g.setFont(Font(FontOptions(11,Font::bold)));g.drawText(aux?String("AUX | 100% WET"):String("INSERT | ")+String(p.get("mix"),1)+"% WET",620,5,165,22,Justification::centredLeft);g.addTransform(AffineTransform::translation(0,32));drawPanel(g);}
void PlateEditor::drawPanel(Graphics&g){
 // The approved artwork is embedded into the binary, not loaded from docs at runtime.
 const auto& background=rackBackground();
 g.drawImage(background,Rectangle<float>(0,0,1043,366));
 g.drawImage(rackMeter(),Rectangle<float>(849.4f,23.65f,173.4f,153.7f));
 const float sel=p.get("plate_select"),sx=69,sy=177;
 g.setColour(Colours::black.withAlpha(.3f));g.fillEllipse(sx-18,sy-16,38,38);
 g.setGradientFill(ColourGradient(Colour(0xff67665f),sx-15,sy-16,Colour(0xff181917),sx+15,sy+17,false));g.fillEllipse(sx-17,sy-17,34,34);
 g.setColour(Colour(0xff252622));g.fillEllipse(sx-13,sy-13,26,26);g.setColour(Colour(0xff99968b));g.drawEllipse(sx-16,sy-16,32,32,.7f);
 {Graphics::ScopedSaveState state(g);g.addTransform(AffineTransform::rotation((sel-1)*.38f,sx,sy));
 Path lever;lever.startNewSubPath(sx-7,sy+48);lever.lineTo(sx-3,sy-56);lever.quadraticTo(sx,sy-62,sx+3,sy-56);lever.lineTo(sx+8,sy+48);lever.closeSubPath();
 DropShadow(Colours::black.withAlpha(.5f),4,{2,3}).drawForPath(g,lever);
 g.setGradientFill(ColourGradient(Colour(0xff70706a),sx-7,sy,Colour(0xff131412),sx+8,sy,false));g.fillPath(lever);
 g.setColour(Colour(0xff99998d));g.drawLine(sx-1,sy-55,sx-1,sy+45,.7f);}
 auto value=[&](String s,float x,float y,float w=70){text(g,s,x,y,w,18,14);};
 value(String(p.get("width"),0)+" %",372,112);auto balance=p.get("balance");value(std::abs(balance)<.05f?"C":String(balance<0?"L ":"R ")+String(std::abs(balance),0),372,242);
 value(p.get("mod_rate")<.005?"OFF":String(p.get("mod_rate"),2)+" Hz",603,112);
 value(String(p.get("mod_depth"),1)+" c",603,242);
 auto hz=[](float v){return v>=1000?String(v/1000,1)+" kHz":String(v,0)+" Hz";};
 value(hz(p.get("eq_low_freq")),687,112);value(hz(p.get("eq_high_freq")),772,112);
 value(String(p.get("eq_low_gain"),1)+" dB",687,242);value(String(p.get("eq_high_gain"),1)+" dB",772,242);
 value(String(p.get("predelay"),0)+" ms",858,242);
 const bool lit=p.get("power")>.5;
 Path lamp;lamp.addEllipse(507,183,30,30);
 if(lit)DropShadow(Colour(0xffff2313).withAlpha(.8f),17,{0,0}).drawForPath(g,lamp);
 g.setColour(Colour(0xff777369));g.fillEllipse(502,178,40,40);g.setColour(Colour(0xffe8e2d3));g.drawEllipse(503,179,38,38,1.5f);
 g.setGradientFill(ColourGradient(Colour(lit?0xffff3027:0xff793a32),520,195,Colour(lit?0xffa9000d:0xff351712),533,208,true));g.fillEllipse(507,183,30,30);
 if(lit){g.setGradientFill(ColourGradient(Colour(0xfffff7dc),520,195,Colour(0xffff2b12),526,201,true));g.fillEllipse(516,191,12,12);}
 g.setColour(Colours::white.withAlpha(.45f));g.fillEllipse(513,186,9,5);
 const float cx=936,cy=140,rr=72;
 const float dbv=20*std::log10(jmax(.00001f,vu))+18;
 // The VU's printed dB scale is non-linear; its needle follows linear amplitude.
 const float amplitude=std::pow(10.f,jlimit(-20.f,3.f,dbv)/20.f);
 const float angle=jmap(amplitude,.1f,std::pow(10.f,.15f),-.91f,.91f);
 g.setColour(Colour(0xff24231c));g.drawLine(cx,cy,cx+std::sin(angle)*rr,cy-std::cos(angle)*rr,.9f);screw(g,cx,cy,5);
}

void PlateEditor::mouseDown(const MouseEvent&e){if(!e.mods.isPopupMenu())return;PopupMenu menu;menu.setLookAndFeel(&softwarePopupLook());auto names=PlateProcessor::presetNames();for(int i=0;i<names.size();++i)menu.addItem(i+1,names[i]);menu.addSeparator();menu.addItem(101,"Open preset browser...");menu.addItem(102,"Reset");menu.addItem(103,"Export preset...");menu.addItem(104,"Import preset...");menu.showMenuAsync(PopupMenu::Options().withTargetComponent(this),[safe=Component::SafePointer<PlateEditor>(this)](int id){if(!safe||safe->closing)return;if(id>0&&id<=platebank::count)safe->p.loadPreset(id-1);if(id==101)safe->openPresets();if(id==102)safe->reset.triggerClick();if(id==103)safe->chooseFile(true);if(id==104)safe->chooseFile(false);});}

void PlateEditor::updatePresetCaption(){auto name=p.state.state.getProperty("preset_name","Default").toString();browse.setButtonText(name);aButton.setToggleState(!usingB,dontSendNotification);bButton.setToggleState(usingB,dontSendNotification);}
void PlateEditor::openPresets(){if(!presetWindow){presetWindow=std::make_unique<PresetWindow>(p,[safe=Component::SafePointer<PlateEditor>(this)]{if(safe&&!safe->closing)safe->updatePresetCaption();});auto bounds=getScreenBounds();presetWindow->setTopLeftPosition(bounds.getX()+20,bounds.getY()+35);}presetWindow->refresh();presetWindow->setVisible(true);presetWindow->toFront(true);}
