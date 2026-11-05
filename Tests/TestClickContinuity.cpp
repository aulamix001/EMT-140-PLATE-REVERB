#include "DSP/PlateEngine.h"
#include "Common/ParameterTransactions.h"
#include <iostream>
#include <stdexcept>
#include <limits>
void require(bool ok,const char*what){if(!ok)throw std::runtime_error(what);}
int main(){try{
 for(double sr:{44100.,48000.,96000.,192000.}){
  for(bool bypass:{false,true}){plate::Engine e;e.prepare(sr);plate::Parameters p;p.mix=0;p.wetSolo=false;p.power=!bypass;
   for(int i=0;i<2048;++i){const double x=.2*std::sin(i*.15)+.1;auto y=e.tick(x,-x,p);require(y[0]==x&&y[1]==-x,"startup altered dry MIX 0 or bypass");}
   e.reset();auto y=e.tick(.2,-.3,p);require(y[0]==.2&&y[1]==-.3,"DSP reset altered dry/bypass");
  }
  for(int model=0;model<3;++model){
   plate::Engine e;e.prepare(sr);plate::Parameters p;p.plate=model;p.filterModel=2;p.eq=false;p.predelay=137;p.rt={1,1,1};
   int sample=0;auto render=[&](){const auto x=.02*std::sin(2*plate::pi*997*sample++/sr);return e.tick(x,x,p);};
   for(int i=0;i<int(sr*3);++i)render();auto previous=render();double steady=0,changing=0;
   for(int i=0;i<int(sr*.5);++i){auto y=render();for(int c=0;c<2;++c)steady=std::max(steady,std::abs(y[c]-previous[c]));previous=y;}
   for(double delay:{0.,250.,70.,0.}){p.predelay=delay;for(int i=0;i<int(sr*.12);++i){auto y=render();for(int c=0;c<2;++c)changing=std::max(changing,std::abs(y[c]-previous[c]));previous=y;}}
   std::cout<<"predelay sr="<<sr<<" model="<<model<<" step ratio="<<changing/steady<<'\n';
   require(changing<steady*1.3,"pre-delay change generated a discontinuity/Doppler burst");
   // Strong automation, including model changes during tail, must stay finite/bounded.
   p.eq=true;for(int i=0;i<int(sr);++i){if(i%127==0){p.plate=(p.plate+1)%3;p.lowFreq=p.lowFreq==20?2000:20;p.highFreq=p.highFreq==200?20000:200;p.lowGain=-p.highGain;p.highGain=p.highGain==12?-12:12;p.rate=p.rate==0?.8:0;p.depth=10;p.predelay=double((i/127)%251);p.wetSolo=!p.wetSolo;p.mix=double((i/127)%101);p.width=double((i/127)%101);p.balance=double((i/127)%201)-100;}auto y=render();require(std::isfinite(y[0])&&std::isfinite(y[1])&&std::abs(y[0])<3&&std::abs(y[1])<3,"unstable automation");}
   e.reset();for(int i=0;i<1024;++i){auto y=e.tick(0,0,p);require(y[0]==0&&y[1]==0,"host DSP reset did not flush history");}
  }
 }
 std::array<std::atomic<float>,20> values{};std::array<std::atomic<float>*,20> raw{};std::array<float,20> snapshot{};
 for(size_t i=0;i<20;++i){values[i]=float(i);raw[i]=&values[i];}plate::ParameterTransactions transactions;
 require(transactions.capture(raw,snapshot),"initial snapshot");const auto old=snapshot;
 {plate::ParameterTransactions::Write write(transactions);for(size_t i=0;i<10;++i)values[i]=100+float(i);require(!transactions.capture(raw,snapshot)&&snapshot==old,"reader consumed half a preset");{plate::ParameterTransactions::Write nested(transactions);values[10]=110;}require(!transactions.capture(raw,snapshot),"nested writer released another writer");for(size_t i=10;i<20;++i)values[i]=100+float(i);}
 require(transactions.capture(raw,snapshot),"committed snapshot rejected");for(size_t i=0;i<20;++i)require(snapshot[i]==100+float(i),"incoherent committed snapshot");
 std::cout<<"PASS: dry/bypass startup, click-free pre-delay heads, rapid automation, reset and bounded coherent snapshots at four sample rates\n";
 }catch(const std::exception&e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
