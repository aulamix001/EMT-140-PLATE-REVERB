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
struct Smooth { double value=0, k=0.001; void prepare(double sr,double seconds=.05){k=1-std::exp(-1/(sr*seconds));} double next(double target){if(value==target)return value;value+=k*(target-value);if(std::abs(target-value)<1e-10*std::max(1.,std::abs(target)))value=target;return value;} };
struct Biquad {
 double b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0;
 double lastSr=-1,lastHz=-1,lastDb=1e30;bool lastHigh=false;
 std::array<double,5> goal{},increment{};int rampLeft=0;
 void reset()noexcept{z1=z2=0;rampLeft=0;lastSr=-1;}
 void shelf(double sr,double hz,double db,bool high,int ramp=0) {
  if(sr==lastSr&&hz==lastHz&&db==lastDb&&high==lastHigh)return;
  lastSr=sr;lastHz=hz;lastDb=db;lastHigh=high;
  const std::array<double,5> old{{b0,b1,b2,a1,a2}};
  hz=std::clamp(hz,10.,sr*.45); double A=std::pow(10.,db/40),w=2*pi*hz/sr,c=std::cos(w),s=std::sin(w),q=std::sqrt(2*A)*s;
  double a0;
  if(high){b0=A*((A+1)+(A-1)*c+q); b1=-2*A*((A-1)+(A+1)*c); b2=A*((A+1)+(A-1)*c-q); a0=(A+1)-(A-1)*c+q; a1=2*((A-1)-(A+1)*c); a2=(A+1)-(A-1)*c-q;}
  else {b0=A*((A+1)-(A-1)*c+q); b1=2*A*((A-1)-(A+1)*c); b2=A*((A+1)-(A-1)*c-q); a0=(A+1)+(A-1)*c+q; a1=-2*((A-1)+(A+1)*c); a2=(A+1)+(A-1)*c-q;}
  b0/=a0;b1/=a0;b2/=a0;a1/=a0;a2/=a0;
  goal={{b0,b1,b2,a1,a2}};rampLeft=ramp;
  if(ramp>0){for(int i=0;i<5;++i)increment[size_t(i)]=(goal[size_t(i)]-old[size_t(i)])/ramp;b0=old[0];b1=old[1];b2=old[2];a1=old[3];a2=old[4];}
 }
 double tick(double x){if(rampLeft>0){b0+=increment[0];b1+=increment[1];b2+=increment[2];a1+=increment[3];a2+=increment[4];if(--rampLeft==0){b0=goal[0];b1=goal[1];b2=goal[2];a1=goal[3];a2=goal[4];}}double y=b0*x+z1;z1=b1*x-a1*y+z2;z2=b2*x-a2*y; if(std::abs(z1)<1e-30)z1=0;if(std::abs(z2)<1e-30)z2=0;return y;}
};
struct Delay {
 std::vector<double> data; size_t pos=0;
 void prepare(int length){data.assign(std::max(2,length),0);pos=0;}
 void clear()noexcept{std::fill(data.begin(),data.end(),0.);pos=0;}
 double read(double samples)const {
  const double delay=std::clamp(samples,1.,double(data.size()-1));
  const auto whole=size_t(delay);const double fraction=delay-double(whole);
  // Wrap integer indices, not a near-zero negative floating position: adding
  // capacity to -epsilon can round up to capacity and read one past the buffer.
  const size_t newer=pos>=whole?pos-whole:pos+data.size()-whole;
  const size_t older=newer==0?data.size()-1:newer-1;
  return data[newer]*(1-fraction)+data[older]*fraction;
 }
 double readInteger(size_t samples)const {return data[pos>=samples?pos-samples:pos+data.size()-samples];}
 double oldest()const{return data[pos];}
 void push(double x){data[pos]=std::abs(x)<1e-30?0:x;if(++pos==data.size())pos=0;}
 double allpass(double x,double coefficient=.65){auto d=oldest();double y=d-coefficient*x;push(x+coefficient*y);return y;}

};
// Changing pre-delay crossfades stationary read heads instead of sweeping the
// read pointer through the buffer (which creates a large Doppler burst).
struct PredelayHeads {
 double from=0,to=0;int elapsed=0,length=1;bool fading=false;
 void prepare(double sr)noexcept{length=std::max(1,int(sr*.03));}
 void initialize(double samples)noexcept{from=to=samples;elapsed=0;fading=false;}
 struct Position{double from,to,mix;};
 Position next(double target)noexcept{
  if(!fading&&target!=to){from=to;to=target;elapsed=0;fading=true;}
  if(!fading)return {to,to,0};
  const double x=double(elapsed)/length,w=x*x*(3-2*x);
  const Position out{from,to,w};
  if(++elapsed>length){fading=false;from=to;}
  return out;
 }
};
// All tuning is an independent plate-inspired design, not a proprietary circuit model.
struct Tank {
 std::array<Delay,16> lines,dispersion;
 std::array<Delay,6> diffusion,sideDiffusion;
 Delay early,earlySide;
 std::array<size_t,16> nominal{};
 std::array<double,16> low{},gainLow{},gainHigh{},roundTrip{},phaseSin{},phaseCos{};
 std::array<Smooth,16> excursion;
 std::array<size_t,12> earlyLeft{},earlyRight{};
 Smooth rt;
 std::array<Biquad,2> pickupColour;
 double sr=48000,damping=0,allpassCoefficient=.55,inputCoefficient=.67,lowRatio=1.02,highRatio=.45,earlyGain=.14;
 int counter=0,modelIndex=0;double cachedRt=-1;
 void clear()noexcept{
  for(auto&d:lines)d.clear();for(auto&d:dispersion)d.clear();for(auto&d:diffusion)d.clear();for(auto&d:sideDiffusion)d.clear();early.clear();earlySide.clear();low.fill(0);counter=0;cachedRt=-1;for(auto&f:pickupColour)f.reset();for(auto&s:excursion)s.value=0;
 }
 void prepare(double sampleRate,int model){
  sr=sampleRate;modelIndex=model;counter=0;cachedRt=-1;rt.prepare(sr);rt.value=2;low.fill(0);
  static constexpr int lengths[3][16]={
   {149,211,293,353,431,503,593,683,787,907,1031,1163,1301,1451,1613,1783},
   {97,163,229,307,397,487,607,739,883,1033,1193,1361,1543,1733,1933,2141},
   {181,263,349,443,547,661,787,929,1087,1259,1451,1657,1879,2111,2357,2617}};
  static constexpr int inputDelays[3][6]={{31,73,113,179,283,421},{19,43,79,131,211,337},{43,101,197,307,487,701}};
  static constexpr double earlyMs[12]={.35,.71,1.13,1.79,2.41,3.19,4.31,5.83,7.19,9.23,12.31,16.17};
  const double cutoff[]={4500,3500,9000},lowRatios[]={1.02,1.06,1.005},highRatios[]={.45,.25,.85};
  const double diffusionCoefficients[]={.67,.52,.74},dispersionCoefficients[]={.55,.42,.70};
  pickupColour={};const double colourDb[]={-3.,-7.,2.5},colourHz[]={4500,3000,6500};for(auto&filter:pickupColour)filter.shelf(sr,colourHz[model],colourDb[model],true);
  damping=std::exp(-2*pi*cutoff[model]/sr);lowRatio=lowRatios[model];highRatio=highRatios[model];
  inputCoefficient=diffusionCoefficients[model];allpassCoefficient=dispersionCoefficients[model];earlyGain=model==1?.20:model==2?.12:.14;
  for(int i=0;i<16;++i){
   nominal[i]=size_t(std::max(2,int(lengths[model][i]*sr/48000)));
   size_t dispersiveLength=size_t(std::max(2,int((11+i*3+model*7)*sr/48000)));
   lines[i].prepare(int(nominal[i]+size_t(sr*.006)+4));dispersion[i].prepare(int(dispersiveLength));
   // Reference-band allpass group delay contributes to the actual round-trip time.
   double groupDelay=0;for(double hz:{354.,420.,500.,595.,707.}){double c=allpassCoefficient;groupDelay+=double(dispersiveLength)*(1-c*c)/(1+c*c-2*c*std::cos(2*pi*hz*double(dispersiveLength)/sr));}
   roundTrip[i]=(double(nominal[i])+groupDelay/5)/sr;
   phaseSin[i]=std::sin(i*2.399963229728653+model*1.37);phaseCos[i]=std::cos(i*2.399963229728653+model*1.37);
   excursion[i].prepare(sr);excursion[i].value=0;gainLow[i]=gainHigh[i]=.8;
  }
  for(int i=0;i<6;++i){int length=std::max(2,int(inputDelays[model][i]*sr/48000));diffusion[i].prepare(length);sideDiffusion[i].prepare(length);}
  early.prepare(int(sr*.025)+8);earlySide.prepare(int(sr*.025)+8);
  for(int i=0;i<12;++i){double factor=model==1?.75:model==2?1.12:1.;earlyLeft[i]=size_t(std::max(1,int(earlyMs[i]*factor*sr*.001)));earlyRight[i]=size_t(std::max(1,int((earlyMs[i]*factor+.13+(i%3)*.09)*sr*.001)));}
 }
 std::array<double,2> tick(double x,double target,double modulationSamples,double sine,double cosine,double side=0){
  rt.next(target);for(auto&d:diffusion)x=d.allpass(x,inputCoefficient);early.push(x);for(auto&d:sideDiffusion)side=d.allpass(side,inputCoefficient);earlySide.push(side);
  if(counter==0&&rt.value!=cachedRt){cachedRt=rt.value;for(int i=0;i<16;++i){gainLow[i]=std::exp(-6.907755278982137*roundTrip[i]/(std::max(.3,rt.value)*lowRatio));gainHigh[i]=std::exp(-6.907755278982137*roundTrip[i]/(std::max(.3,rt.value)*highRatio));}}
  std::array<double,16> y{};double sum=0;
  for(int i=0;i<16;++i){double amplitude=excursion[i].next(std::min(modulationSamples,std::min(sr*.006,double(nominal[i])*.35)));if(amplitude<1e-9){excursion[i].value=0;y[i]=lines[i].readInteger(nominal[i]);}else y[i]=lines[i].read(double(nominal[i])+amplitude*(sine*phaseCos[i]+cosine*phaseSin[i]));sum+=y[i];}
  std::array<double,2> out{};
  for(int i=0;i<16;++i){
   double mixed=y[i]-sum/8;low[i]=(1-damping)*mixed+damping*low[i];
   double feedback=gainHigh[i]*mixed+(gainLow[i]-gainHigh[i])*low[i];
   feedback=dispersion[i].allpass(feedback,allpassCoefficient);
   int inputSign=((i+modelIndex)%2)?-1:1;
   lines[i].push(inputSign*.14*x+(((i/2+modelIndex)%2)?-.14:.14)*side+feedback);
   out[0]+=y[i]*((i%2)?-1:1)*.14;
   out[1]+=y[i]*(((i/2+modelIndex)%2)?-1:1)*.14;
  }
  for(int i=0;i<12;++i){out[0]+=early.readInteger(earlyLeft[i]+1)*earlyGain*((i%3)==1?-.28:.28);out[1]+=early.readInteger(earlyRight[i]+1)*earlyGain*((i%3)==2?-.28:.28);}
  for(int i=0;i<12;++i){out[0]+=earlySide.readInteger(earlyLeft[i]+1)*earlyGain*((i%3)==1?-.28:.28);out[1]-=earlySide.readInteger(earlyRight[i]+1)*earlyGain*((i%3)==2?-.28:.28);}
  for(int c=0;c<2;++c)out[c]=pickupColour[c].tick(out[c]);
  counter=(counter+1)%32;return out;
 }
};
class Engine {
 double sr=48000,phase=0,modulationSamples=0,lastRate=-1,lastDepth=-1,lfoSine=0,lfoCosine=1,dcPrevious=0,dcState=0,dcPreviousSide=0,dcStateSide=0,dcCoefficient=0;unsigned filterCounter=0;
 std::array<Tank,3> tanks;std::array<Delay,2> predelay;
 std::array<Biquad,2> input,eqLow,eqHigh;
 std::array<Smooth,3> select;Smooth width,balance,mix,power,solo,eqOn,lowF,lowG,highF,highG,filterHz,filterDb;
 PredelayHeads readHeads;bool initialized=false,filtersReady=false;std::array<bool,3> running{};
 void initialize(const Parameters&p)noexcept{
  initialized=true;for(int i=0;i<3;++i){running[i]=i==p.plate;select[i].value=i==p.plate?1:0;tanks[i].rt.value=p.rt[i];}
  width.value=p.width*.01;balance.value=p.balance*.01;mix.value=p.mix*.01;power.value=p.power?1:0;solo.value=p.wetSolo?1:0;eqOn.value=p.eq?1:0;
  lowF.value=p.lowFreq;lowG.value=p.lowGain;highF.value=p.highFreq;highG.value=p.highGain;
  const double mh[]={90,125,180,250,270,360},ed[]={-4,-10,-16};filterHz.value=p.filterModel==2?80:p.filterModel==0?80:mh[std::clamp(p.martech,0,5)];filterDb.value=p.filterModel==2?0:p.filterModel==0?ed[std::clamp(p.emt,0,2)]:-12;
  readHeads.initialize(p.predelay*.001*sr);
 }
public:
 void prepare(double sampleRate){sr=sampleRate;initialized=filtersReady=false;readHeads.prepare(sr);phase=0;lastRate=lastDepth=-1;modulationSamples=0;lfoSine=0;lfoCosine=1;dcPrevious=dcState=dcPreviousSide=dcStateSide=0;dcCoefficient=std::exp(-2*pi*8/sr);filterCounter=0;input={};eqLow={};eqHigh={};for(int i=0;i<3;++i){tanks[i].prepare(sr,i);select[i].prepare(sr);select[i].value=i==0?1:0;}
  for(auto&d:predelay)d.prepare(int(sr*.251)+4);
  for(auto*s:{&width,&balance,&mix,&power,&solo,&eqOn,&lowF,&lowG,&highF,&highG,&filterHz,&filterDb})s->prepare(sr);
  width.value=1;balance.value=0;mix.value=.15;power.value=1;solo.value=1;eqOn.value=1;lowF.value=120;highF.value=8000;lowG.value=highG.value=0;filterHz.value=80;filterDb.value=-10;
 }
 void reset()noexcept{for(auto&t:tanks)t.clear();for(auto&d:predelay)d.clear();for(auto&f:input)f.reset();for(auto&f:eqLow)f.reset();for(auto&f:eqHigh)f.reset();initialized=filtersReady=false;phase=0;lastRate=lastDepth=-1;modulationSamples=0;lfoSine=0;lfoCosine=1;dcPrevious=dcState=dcPreviousSide=dcStateSide=0;filterCounter=0;}
 std::array<double,2> tick(double l,double r,const Parameters&p){
  if(!std::isfinite(l))l=0;if(!std::isfinite(r))r=0;
  if(!initialized)initialize(p);
  double on=power.next(p.power?1:0);if(on<1e-7&&!p.power)return {l,r};
  const double mh[]={90,125,180,250,270,360},ed[]={-4,-10,-16};
  filterHz.next(p.filterModel==2?80:p.filterModel==0?80:mh[std::clamp(p.martech,0,5)]);filterDb.next(p.filterModel==2?0:p.filterModel==0?ed[std::clamp(p.emt,0,2)]:-12);
  lowF.next(p.lowFreq);lowG.next(p.lowGain);highF.next(p.highFreq);highG.next(p.highGain);
  if((filterCounter++%32)==0){const int ramp=filtersReady?32:0;for(int c=0;c<2;++c){input[c].shelf(sr,filterHz.value,filterDb.value,false,ramp);eqLow[c].shelf(sr,lowF.value,lowG.value,false,ramp);eqHigh[c].shelf(sr,highF.value,highG.value,true,ramp);}filtersReady=true;}
  if(p.rate!=lastRate||p.depth!=lastDepth){lastRate=p.rate;lastDepth=p.depth;modulationSamples=p.rate>0&&p.depth>0?std::expm1(std::log(2.)*p.depth/1200)*sr/(2*pi*p.rate):0;}
  double filteredL=input[0].tick(l),filteredR=input[1].tick(r);double conditioned=(filteredL+filteredR)*.5;double conditionedSide=(filteredL-filteredR)*.5;double blockedSide=conditionedSide-dcPreviousSide+dcCoefficient*dcStateSide;dcPreviousSide=conditionedSide;dcStateSide=std::abs(blockedSide)<1e-30?0:blockedSide;double sideInput=std::tanh(blockedSide*1.05)/1.05;double blocked=conditioned-dcPrevious+dcCoefficient*dcState;dcPrevious=conditioned;dcState=std::abs(blocked)<1e-30?0:blocked;double x=std::tanh(blocked*1.05)/1.05;
  if(p.rate>0){phase+=2*pi*p.rate/sr;if(phase>2*pi)phase-=2*pi;lfoSine=std::sin(phase);lfoCosine=std::cos(phase);}
  double sine=lfoSine,cosine=lfoCosine;
  std::array<double,2> wet{};for(int i=0;i<3;++i){
   const double a=select[i].next(i==p.plate?1:0);
   if(i==p.plate&&!running[i]){running[i]=true;tanks[i].rt.value=p.rt[i];}
   if(!running[i])continue;
   if(i!=p.plate&&a<1e-7){tanks[i].clear();select[i].value=0;running[i]=false;continue;}
   const auto y=tanks[i].tick(x,p.rt[i],modulationSamples,sine,cosine,sideInput);for(int c=0;c<2;++c)wet[c]+=a*y[c];
  }
  const auto heads=readHeads.next(p.predelay*.001*sr);double e=eqOn.next(p.eq?1:0);for(int c=0;c<2;++c){predelay[c].push(wet[c]);double first=predelay[c].read(heads.from+1);wet[c]=3.0*(heads.mix==0?first:first+heads.mix*(predelay[c].read(heads.to+1)-first));double filtered=eqHigh[c].tick(eqLow[c].tick(wet[c]));wet[c]+=e*(filtered-wet[c]);}
  double w=width.next(p.width*.01),b=balance.next(p.balance*.01),mid=(wet[0]+wet[1])*.5,side=(wet[0]-wet[1])*.5*w;wet={(mid+side)*(b>0?1-b:1),(mid-side)*(b<0?1+b:1)};
  double m=mix.next(p.mix*.01),s=solo.next(p.wetSolo?1:0);m+=(1-m)*s;return {l+on*m*(wet[0]-l),r+on*m*(wet[1]-r)};
 }
};
}
