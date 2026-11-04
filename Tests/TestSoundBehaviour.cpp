#include "DSP/PlateEngine.h"
#include <cstdlib>
#include <new>
#include <iostream>
#include <stdexcept>
namespace {thread_local bool rendering=false;size_t allocations=0;}
void* operator new(std::size_t n){if(rendering)++allocations;if(void*p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t n){return ::operator new(n);}void operator delete(void*p)noexcept{std::free(p);}void operator delete[](void*p)noexcept{std::free(p);}void operator delete(void*p,std::size_t)noexcept{std::free(p);}void operator delete[](void*p,std::size_t)noexcept{std::free(p);}
void require(bool pass,const char*message){if(!pass)throw std::runtime_error(message);}
int main(){try{
 for(double sr:{44100.,48000.,96000.,192000.}){plate::Engine a,b,c;a.prepare(sr);b.prepare(sr);c.prepare(sr);plate::Parameters off,depth,rate;off.predelay=depth.predelay=rate.predelay=0;depth.depth=10;rate.rate=1;rate.depth=0;
  for(int i=0;i<int(sr);++i){a.tick(0,0,off);b.tick(0,0,depth);c.tick(0,0,rate);}rendering=true;bool identical=true,finite=true;int first=-1;
  for(int i=0;i<int(sr*.15);++i){double x=i==0?.5:0;auto u=a.tick(x,x,off),v=b.tick(x,x,depth),w=c.tick(x,x,rate);identical=identical&&(u==v)&&(u==w);finite=finite&&std::isfinite(u[0])&&std::isfinite(u[1]);if(first<0&&(std::abs(u[0])+std::abs(u[1])>1e-6))first=i;}
  rendering=false;require(identical,"rate zero or depth zero must be bit-identical to stationary tank");require(finite,"finite output");require(first>=0&&first/sr<.002,"wet onset must occur within 2 ms of zero predelay");
  plate::Parameters p;p.rate=1;p.depth=10;p.rt={5.5,5.5,5.5};p.plate=2;rendering=true;for(int i=0;i<int(sr*.25);++i){auto y=a.tick(.2*std::sin(i*.1),.2*std::sin(i*.13),p);finite=finite&&std::isfinite(y[0])&&std::isfinite(y[1])&&std::abs(y[0])<20&&std::abs(y[1])<20;}rendering=false;require(finite,"bounded maximum-depth modulation");
 }
 require(allocations==0,"no allocations in tick, including live modulation");std::cout<<"PASS: exact modulation OFF, early density onset, maximum-depth stability and zero callback allocations at 4 sample rates\n";
 }catch(const std::exception&e){rendering=false;std::cerr<<e.what()<<'\n';return 1;}}
