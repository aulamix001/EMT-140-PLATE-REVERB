#include "DSP/PlateEngine.h"
#include <fstream>
int main(int argc,char**argv){int model=argc>1?std::atoi(argv[1]):0;double rt=argc>2?std::atof(argv[2]):2;plate::Engine e;e.prepare(48000);plate::Parameters p;p.plate=model;p.rt={rt,rt,rt};p.predelay=0;p.filterModel=2;for(int i=0;i<48000;++i)e.tick(0,0,p);std::ofstream f(argc>3?argv[3]:"/tmp/plate-ir.raw",std::ios::binary);for(int i=0;i<48000*9;++i){auto y=e.tick(i==0?1:0,0,p);f.write(reinterpret_cast<const char*>(y.data()),sizeof(double)*2);}}
