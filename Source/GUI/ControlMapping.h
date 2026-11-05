#pragma once
#include <JuceHeader.h>
#include <cmath>
#include <array>
namespace plateui {
enum class DialScale { parameter, frequency, rate };
struct DialSpec {const char* id;int x,y;DialScale scale;const char* help;};
inline constexpr std::array<DialSpec,10> dials{{
 {"width",375,130,DialScale::parameter,"Stereo width of the wet return"},
 {"balance",375,258,DialScale::parameter,"Left/right balance of the wet return"},
 {"mod_rate",606,130,DialScale::rate,"Delay modulation rate; 0 Hz = exactly OFF"},
 {"mod_depth",606,258,DialScale::parameter,"Pitch excursion of the modulated plate delays, in cents"},
 {"eq_low_freq",690,130,DialScale::frequency,"Low shelf frequency (wet return, EQ IN)"},
 {"eq_low_gain",690,258,DialScale::parameter,"Low shelf gain, -12 to +12 dB (wet return, EQ IN)"},
 {"eq_high_freq",775,130,DialScale::frequency,"High shelf frequency (wet return, EQ IN)"},
 {"eq_high_gain",775,258,DialScale::parameter,"High shelf gain, -12 to +12 dB (wet return, EQ IN)"},
 {"predelay",861,258,DialScale::parameter,"Wet pre-delay; noon = 50 ms"},
 {"mix",947,258,DialScale::parameter,"Dry/wet mix; noon = 15%; WET SOLO overrides this control"}
}};
// The physical dial scale is independent of the host's saved normalization.
// Never change APVTS skew/range to match the artwork: that would move automation.
class PanelKnob:public juce::Slider {
public:
 DialScale dialScale=DialScale::parameter;
 double proportionOfLengthToValue(double p)override {
  p=juce::jlimit(0.,1.,p);
  if(dialScale==DialScale::frequency)return getMinimum()*std::pow(getMaximum()/getMinimum(),p);
  if(dialScale==DialScale::rate){if(p<=.02)return .01*p/.02;if(p<=.5)return .01*std::pow(10.,(p-.02)/.48);return .1*std::pow(10.,(p-.5)/.5);}
  return juce::Slider::proportionOfLengthToValue(p);
 }
 double valueToProportionOfLength(double v)override {
  v=juce::jlimit(getMinimum(),getMaximum(),v);
  if(dialScale==DialScale::frequency)return std::log(v/getMinimum())/std::log(getMaximum()/getMinimum());
  if(dialScale==DialScale::rate){if(v<=.01)return .02*v/.01;if(v<=.1)return .02+.48*std::log10(v/.01);return .5+.5*std::log10(v/.1);}
  return juce::Slider::valueToProportionOfLength(v);
 }
};
}
