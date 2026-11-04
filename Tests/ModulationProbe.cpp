#include "DSP/PlateEngine.h"
#include <filesystem>
#include <fstream>
#include <iostream>
int main(int argc,char**argv){if(argc<2)return 1;std::filesystem::create_directories(argv[1]);for(int mode=0;mode<2;++mode){plate::Engine e;e.prepare(48000);plate::Parameters p;p.plate=2;p.predelay=0;p.filterModel=2;p.rate=mode?.8:0;p.depth=10;p.rt={2,2,2};std::ofstream f(std::filesystem::path(argv[1])/(mode?"modulated.f32":"stationary.f32"),std::ios::binary);for(int i=0;i<48000*32;++i){double x=.15*std::sin(2*plate::pi*1000*i/48000);auto y=e.tick(x,x,p);if(i>=48000*16){float pair[]={float(y[0]),float(y[1])};f.write(reinterpret_cast<const char*>(pair),sizeof(pair));}}}std::cout<<"Rendered stationary and modulated 1 kHz steady-state probes\n";}
