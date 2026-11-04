#include "DSP/PlateEngine.h"
#include "Presets/FactoryPresets.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>
plate::Parameters parameters(const platebank::Preset&s){auto&v=s.values;plate::Parameters p;p.plate=int(v[0]);for(int i=0;i<3;++i)p.rt[i]=v[size_t(i+1)];p.filterModel=int(v[4]);p.emt=int(v[5]);p.martech=int(v[6]);p.width=v[7];p.balance=v[8];p.eq=v[9]>.5;p.lowFreq=v[10];p.lowGain=v[11];p.highFreq=v[12];p.highGain=v[13];p.rate=v[14];p.depth=v[15];p.predelay=v[16];p.mix=v[17];p.wetSolo=v[18]>.5;p.power=v[19]>.5;return p;}
int main(int argc,char**argv){try{if(argc<2)throw std::runtime_error("output directory required");std::filesystem::create_directories(argv[1]);bool models=argc>2&&std::string(argv[2])=="models";std::ofstream manifest;if(!models){manifest.open(std::filesystem::path(argv[1])/"rendered-parameters.csv");manifest<<"number";for(auto id:platebank::ids)manifest<<","<<id;manifest<<"\n";manifest<<std::setprecision(9);}for(int n=0;n<(models?3:platebank::count);++n){auto p=parameters(platebank::presets[n]);if(!models){manifest<<n+1;for(float value:platebank::presets[n].values)manifest<<","<<value;manifest<<"\n";}if(models){p=plate::Parameters{};p.plate=n;p.predelay=0;p.filterModel=2;p.eq=false;}p.wetSolo=true; // isolate the wet engine; keep stored width/EQ/filter/predelay unchanged
 plate::Engine e;e.prepare(48000);for(int i=0;i<96000;++i)e.tick(0,0,p);std::ofstream f(std::filesystem::path(argv[1])/(std::to_string(n+1)+".f32"),std::ios::binary);for(int i=0;i<48000*10;++i){auto y=e.tick(i==0?.5:0,i==0?.5:0,p);float frame[2]={float(y[0]),float(y[1])};if(!std::isfinite(frame[0])||!std::isfinite(frame[1]))throw std::runtime_error("nonfinite output");f.write(reinterpret_cast<const char*>(frame),sizeof(frame));}if(!f)throw std::runtime_error("output write failed");}std::cout<<"Rendered 100 wet-only stereo impulse responses at 48 kHz\n";}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
