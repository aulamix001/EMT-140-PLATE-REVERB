#include "DSP/PlateEngine.h"
#include <iostream>
#include <stdexcept>
using namespace plate;
int checks=0;
void require(bool ok,const char*msg){++checks;if(!ok)throw std::runtime_error(msg);}
int main(){try{
 {Delay d;d.prepare(12052);d.push(.25);d.data.back()=-.75;
  require(std::abs(d.read(std::nextafter(1.,2.))-.25)<1e-12,"fractional wrap at negative epsilon must stay in bounds");}

 for(double sr:{44100.,48000.,96000.,192000.}){Engine e;e.prepare(sr);Parameters p;for(int i=0;i<500;++i){auto y=e.tick(0,0,p);require(y[0]==0&&y[1]==0,"silence");}double energy=0;for(int block:{32,64,128,256,512,1024,2048})for(int i=0;i<block*5;++i){auto y=e.tick(i==0?.5:0,0,p);require(std::isfinite(y[0])&&std::isfinite(y[1]),"finite");require(std::abs(y[0])<10&&std::abs(y[1])<10,"bounded");energy+=y[0]*y[0]+y[1]*y[1];}require(energy>1e-8,"impulse tail");}
 Engine a,b;a.prepare(48000);b.prepare(48000);Parameters p,q;q.mix=94;for(int i=0;i<20000;++i){double x=i==0?1:0;auto l=a.tick(x,x,p),r=b.tick(x,x,q);require(l==r,"wet solo must ignore stored mix");}
 a.prepare(48000);p.width=0;for(int i=0;i<60000;++i){auto y=a.tick(i==0?1:0,0,p);if(i>50000)require(std::abs(y[0]-y[1])<1e-6,"mono width");}
 a.prepare(48000);p=Parameters{};p.power=false;for(int i=0;i<60000;++i){auto y=a.tick(.1,-.2,p);if(i>59000)require(std::abs(y[0]-.1)<1e-8&&std::abs(y[1]+.2)<1e-8,"bypass");}
 // Opposite-polarity stereo used to disappear at the mono tank input.
 for(int model=0;model<3;++model){
  Engine mono,side;mono.prepare(48000);side.prepare(48000);
  Parameters stereo;stereo.plate=model;stereo.predelay=0;stereo.filterModel=2;
  for(int i=0;i<48000;++i){mono.tick(0,0,stereo);side.tick(0,0,stereo);}
  double monoEnergy=0,sideEnergy=0;
  for(int i=0;i<144000;++i){double x=i==0?.1:0;auto m=mono.tick(x,x,stereo),s=side.tick(x,-x,stereo);
   monoEnergy+=m[0]*m[0]+m[1]*m[1];sideEnergy+=s[0]*s[0]+s[1]*s[1];
   require(std::isfinite(s[0])&&std::isfinite(s[1]),"stereo-only tail finite");
  }
  require(sideEnergy>monoEnergy*.1,"stereo-only input must excite the reverb");
  require(sideEnergy<monoEnergy*10,"stereo-only energy bounded relative to mono");
 }
 Biquad f;f.shelf(48000,120,0,false);for(int i=0;i<1000;++i){double x=std::sin(i*.13);require(std::abs(f.tick(x)-x)<1e-12,"flat shelf");}
 Delay delay;delay.prepare(1000);for(int i=0;i<400;++i){delay.push(i==0?1:0);require(std::abs(delay.read(101)-(i==100?1:0))<1e-12,"predelay sample offset");}
 std::cout<<"PASS: "<<checks<<" assertions; silence, impulse stability, sample rates, block sizes, wet solo, mono, bypass, flat EQ, delay\n";
 }catch(const std::exception&e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;} }
