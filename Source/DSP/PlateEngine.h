#pragma once
#include <array>
#include <vector>
#include <algorithm>
#include <cmath>
namespace plate {
constexpr double pi=3.14159265358979323846;
struct Parameters {
 int plate=0, filterModel=0, emt=1, martech=2;
 std::array<double,3> rt{2,2,2};
 double width=100,balance=0,lowFreq=120,lowGain=0,highFreq=8000,highGain=0,rate=0,depth=0,predelay=50,mix=15;
 bool eq=true,wetSolo=true,power=true;
};
struct Smooth { double value=0, k=0.001; void prepare(double sr,double seconds=.05){k=1-std::exp(-1/(sr*seconds));} double next(double target){return value+=k*(target-value);} };
struct Biquad {
 double b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0;
 void shelf(double sr,double hz,double db,bool high) {
  hz=std::clamp(hz,10.,sr*.45); double A=std::pow(10.,db/40),w=2*pi*hz/sr,c=std::cos(w),s=std::sin(w),q=std::sqrt(2*A)*s;
  double a0;
  if(high){b0=A*((A+1)+(A-1)*c+q); b1=-2*A*((A-1)+(A+1)*c); b2=A*((A+1)+(A-1)*c-q); a0=(A+1)-(A-1)*c+q; a1=2*((A-1)-(A+1)*c); a2=(A+1)-(A-1)*c-q;}
  else {b0=A*((A+1)-(A-1)*c+q); b1=2*A*((A-1)-(A+1)*c); b2=A*((A+1)-(A-1)*c-q); a0=(A+1)+(A-1)*c+q; a1=-2*((A-1)+(A+1)*c); a2=(A+1)+(A-1)*c-q;}
  b0/=a0;b1/=a0;b2/=a0;a1/=a0;a2/=a0;
 }
 double tick(double x){double y=b0*x+z1;z1=b1*x-a1*y+z2;z2=b2*x-a2*y; if(std::abs(z1)<1e-30)z1=0;if(std::abs(z2)<1e-30)z2=0;return y;}
};
struct Delay {
 std::vector<double> data; size_t pos=0;
 void prepare(int length){data.assign(std::max(2,length),0);pos=0;}
 double read(double samples)const {double p=double(pos)-std::clamp(samples,1.,double(data.size()-1));if(p<0)p+=data.size();auto a=size_t(p);double f=p-a;return data[a]*(1-f)+data[(a+1)%data.size()]*f;}
 double oldest()const{return data[pos];}
 void push(double x){data[pos]=std::abs(x)<1e-30?0:x;pos=(pos+1)%data.size();}
 double allpass(double x){auto d=oldest();double y=d-.65*x;push(x+.65*y);return y;}
};
struct Tank {
 std::array<Delay,16> lines; std::array<Delay,4> diffusion,sideDiffusion; std::array<double,16> low{},gain{}; Smooth rt; double sr=48000,damping=0;int counter=0,modelIndex=0;
 void prepare(double sampleRate,int model){sr=sampleRate;counter=0;modelIndex=model;rt.prepare(sr);rt.value=2;low.fill(0);
  const std::array<int,16> lengths{1423,1559,1747,1867,1999,2143,2287,2411,2557,2687,2797,2953,3089,3251,3391,3547};
  for(int i=0;i<16;++i){lines[i].prepare(int(lengths[i]*sr/48000*(1+model*.071)+(i%3)*model*17));gain[i]=.8;}
  for(int i=0;i<4;++i){const int length=int((113+i*137+model*31)*sr/48000);diffusion[i].prepare(length);sideDiffusion[i].prepare(length);}
  damping=std::exp(-2*pi*(model==0?6500:model==1?8500:10500)/sr);
 }
 std::array<double,2> tick(double x,double target,double side=0){rt.next(target);for(auto& d:diffusion)x=d.allpass(x);for(auto& d:sideDiffusion)side=d.allpass(side);
  std::array<double,16> y{};double sum=0;
  for(int i=0;i<16;++i){y[i]=lines[i].oldest();sum+=y[i];}
  std::array<double,2> out{};
  for(int i=0;i<16;++i){double v=y[i]-sum/8;low[i]=(1-damping)*v+damping*low[i];if(counter==0)gain[i]=std::pow(10.,-3.0*double(lines[i].data.size())/(sr*std::max(.3,rt.value-(modelIndex==1?.045*std::exp(-3*(rt.value-.5)):0))));
   // Orthogonal mid/side injection retains stereo-only input energy.
   lines[i].push((i%2?-.18:.18)*x+((i/2)%2?-.18:.18)*side+low[i]*gain[i]);out[0]+=y[i]*(i%2?-1:1)*.17;out[1]+=y[i]*((i/2)%2?-1:1)*.17;}
  counter=(counter+1)%32;return out;
 }
};
class Engine {
 double sr=48000,phase=0;int filterCounter=0;
 std::array<Tank,3> tanks;std::array<Delay,2> predelay;
 std::array<Biquad,2> input,eqLow,eqHigh;
 std::array<Smooth,3> select;Smooth width,balance,delay,mix,power,solo,eqOn,lowF,lowG,highF,highG,filterHz,filterDb;
public:
 void prepare(double sampleRate){sr=sampleRate;phase=0;filterCounter=0;input={};eqLow={};eqHigh={};for(int i=0;i<3;++i){tanks[i].prepare(sr,i);select[i].prepare(sr);select[i].value=i==0?1:0;}
  for(auto&d:predelay)d.prepare(int(sr*.251)+4);
  for(auto*s:{&width,&balance,&delay,&mix,&power,&solo,&eqOn,&lowF,&lowG,&highF,&highG,&filterHz,&filterDb})s->prepare(sr);
  width.value=1;balance.value=0;delay.value=.05*sr;mix.value=.15;power.value=1;solo.value=1;eqOn.value=1;lowF.value=120;highF.value=8000;lowG.value=highG.value=0;filterHz.value=80;filterDb.value=-10;
 }
 std::array<double,2> tick(double l,double r,const Parameters&p){
  if(!std::isfinite(l))l=0;if(!std::isfinite(r))r=0;
  double on=power.next(p.power?1:0);
  if(on<1e-7&&!p.power)return {l,r};
  const double mh[]={90,125,180,250,270,360},ed[]={-4,-10,-16};
  filterHz.next(p.filterModel==2?80:p.filterModel==0?80:mh[std::clamp(p.martech,0,5)]);filterDb.next(p.filterModel==2?0:p.filterModel==0?ed[std::clamp(p.emt,0,2)]:-12);
  lowF.next(p.lowFreq);lowG.next(p.lowGain);highF.next(p.highFreq);highG.next(p.highGain);
  if((filterCounter++%32)==0){for(int c=0;c<2;++c){input[c].shelf(sr,filterHz.value,filterDb.value,false);eqLow[c].shelf(sr,lowF.value,lowG.value,false);eqHigh[c].shelf(sr,highF.value,highG.value,true);}}
  const double filteredL=input[0].tick(l),filteredR=input[1].tick(r);
  double x=std::tanh((filteredL+filteredR)*.5*1.05)/1.05;
  double sideInput=std::tanh((filteredL-filteredR)*.5*1.05)/1.05;
  phase+=2*pi*p.rate/sr;if(phase>2*pi)phase-=2*pi;
  std::array<double,2> wet{};
  for(int i=0;i<3;++i){auto y=tanks[i].tick(x,p.rt[i]*std::pow(2.,p.depth*std::sin(phase)/1200),sideInput);double a=select[i].next(i==p.plate?1:0);for(int c=0;c<2;++c)wet[c]+=a*y[c];}
  double d=delay.next(p.predelay*.001*sr), e=eqOn.next(p.eq?1:0);
  for(int c=0;c<2;++c){predelay[c].push(wet[c]);
   // Fixed +9.54 dB return trim; decay feedback and dry/bypass gains are unchanged.
   wet[c]=3.0*predelay[c].read(d+1);double filtered=eqHigh[c].tick(eqLow[c].tick(wet[c]));wet[c]+=e*(filtered-wet[c]);}
  double w=width.next(p.width*.01),b=balance.next(p.balance*.01),mid=(wet[0]+wet[1])*.5,side=(wet[0]-wet[1])*.5*w;
  wet={ (mid+side)*(b>0?1-b:1), (mid-side)*(b<0?1+b:1) };
  double m=mix.next(p.mix*.01),s=solo.next(p.wetSolo?1:0);m+=(1-m)*s;
  return {l+on*m*(wet[0]-l),r+on*m*(wet[1]-r)};
 }
};
}
