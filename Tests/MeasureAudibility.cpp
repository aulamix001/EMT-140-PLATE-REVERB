// Deterministic offline measurements; no device or host is exercised.
#include "DSP/PlateEngine.h"
#include "Presets/FactoryPresets.h"
#include <iostream>
#include <iomanip>
#include <random>

plate::Parameters parameters(const platebank::Preset& preset) {
 const auto& v=preset.values; plate::Parameters p;
 p.plate=int(v[0]);p.rt={v[1],v[2],v[3]};p.filterModel=int(v[4]);p.emt=int(v[5]);p.martech=int(v[6]);
 p.width=v[7];p.balance=v[8];p.eq=v[9]>.5f;p.lowFreq=v[10];p.lowGain=v[11];p.highFreq=v[12];p.highGain=v[13];
 p.rate=v[14];p.depth=v[15];p.predelay=v[16];p.mix=v[17];p.wetSolo=v[18]>.5f;p.power=v[19]>.5f;return p;
}
int main() {
 constexpr int sr=48000, count=sr*2;
 std::cout<<"preset,mode,wet_gain_db,insert_wet_gain_db,peak,correlation\n"<<std::setprecision(9);
 for(const auto& preset:platebank::presets) for(int mode=0;mode<2;++mode) {
  auto p=parameters(preset);const double mix=p.wetSolo?1:p.mix*.01;p.wetSolo=true;
  plate::Engine e;e.prepare(sr);for(int i=0;i<sr;++i)e.tick(0,0,p);
  std::mt19937 rng(140);double inputEnergy=0,left=0,right=0,cross=0,peak=0;
  for(int i=0;i<count;++i) {
   const double x=(double(rng())/4294967295.0*2-1)*.1;
   const auto y=e.tick(x,mode?-x:x,p);
   if(i<sr/2)continue;
   inputEnergy+=x*x;left+=y[0]*y[0];right+=y[1]*y[1];cross+=y[0]*y[1];
   peak=std::max({peak,std::abs(y[0]),std::abs(y[1])});
  }
  const double gain=std::sqrt((left+right)/(2*inputEnergy));
  const auto db=[](double x){return 20*std::log10(std::max(1e-15,x));};
  std::cout<<'"'<<preset.name<<"\","<<(mode?"opposite":"mono")<<','<<db(gain)<<','<<db(gain*mix)<<','<<peak<<','<<(left*right>0?cross/std::sqrt(left*right):0)<<'\n';
 }
}
