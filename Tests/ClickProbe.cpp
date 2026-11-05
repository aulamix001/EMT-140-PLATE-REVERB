#if PLATE_CLICK_BASELINE
// Exact engine from the released e6b647b commit; benchmark-only, never linked into the plugin.
#include "Reference/PlateEngine-0.2.0.h"
#else
#include "DSP/PlateEngine.h"
#endif
#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
int main(int argc,char**argv){
 plate::Engine engine;engine.prepare(48000);plate::Parameters p;p.filterModel=2;p.predelay=0;p.wetSolo=false;p.mix=0;
 std::cout<<"MIX0_FIRST "<<engine.tick(.2,.2,p)[0]<<'\n';engine.prepare(48000);p=plate::Parameters{};p.power=false;std::cout<<"BYPASS_FIRST "<<engine.tick(.2,.2,p)[0]<<'\n';
 for(double sr:{44100.,48000.,96000.,192000.})for(bool modulation:{false,true}){
  engine.prepare(sr);p=plate::Parameters{};p.rate=modulation?.8:0;p.depth=modulation?10:0;
  for(int i=0;i<int(sr*2);++i)engine.tick(0,0,p);
  double energy=0;std::vector<double> blocks;blocks.reserve(size_t(sr*4/64)+1);
  const auto start=std::chrono::steady_clock::now();
  for(int block=0;block<int(sr*4/64);++block){const auto begin=std::chrono::steady_clock::now();for(int j=0;j<64;++j){const int sample=block*64+j;auto y=engine.tick(.02*std::sin(2*plate::pi*997*sample/sr),.015*std::sin(2*plate::pi*733*sample/sr),p);energy+=y[0]*y[0]+y[1]*y[1];}blocks.push_back(std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-begin).count());}
  const auto elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();std::sort(blocks.begin(),blocks.end());
  std::cout<<"BENCH "<<sr<<' '<<modulation<<' '<<elapsed/4*100<<' '<<blocks[blocks.size()*99/100]<<' '<<blocks.back()<<' '<<energy<<'\n';
 }
 if(argc>1){engine.prepare(48000);p=plate::Parameters{};p.filterModel=2;p.eq=false;p.predelay=137;p.rt={1,1,1};for(int i=0;i<48000*3;++i)engine.tick(.02*std::sin(2*plate::pi*997*i/48000),.02*std::sin(2*plate::pi*997*i/48000),p);std::ofstream f(argv[1],std::ios::binary);for(int i=0;i<48000*3;++i){if(i==48000)p.predelay=0;auto y=engine.tick(.02*std::sin(2*plate::pi*997*i/48000),.02*std::sin(2*plate::pi*997*i/48000),p);float frame[2]={float(y[0]),float(y[1])};f.write(reinterpret_cast<char*>(frame),sizeof(frame));}}
}
