// Host-only validation: g++ -std=c++11 replay.cpp -o replay.exe
// Run replay.exe <file.csv>. Firmware uses this same RepDetector.h.
#include "../../RepTracker/RepDetector.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <cassert>
struct Row { uint32_t ms; float x,z; };
void checks() {
  RepDetector d; d.reset(350,350); d.fullThreshold=15;
  uint32_t t=0;
  auto hold=[&](float x,int n) { for(int i=0;i<n;++i) { d.update(x,350,t); t+=50; } };
  hold(350,40); assert(d.full==0 && d.partial==0);
  hold(375,15); assert(d.full==0); // no count before return
  hold(350,20); assert(d.full==1 && d.partial==0);
  hold(359,15); hold(350,20); assert(d.full==1 && d.partial==1);
  hold(351,40); assert(d.full==1 && d.partial==1);
  hold(375,220); assert(d.rejected==1); // timeout without duplicate count
  hold(350,20); assert(d.full==1 && d.partial==1);
  d.reset(350,350); hold(350,20); hold(325,15); hold(350,20);
  assert(d.full==1); // either movement direction
}
int main(int argc,char**argv) {
  checks();
  if(argc<2) { std::cout<<"Detector checks passed\n"; return 0; }
  std::ifstream file(argv[1]); if(!file) return 1;
  std::string line; std::getline(file,line); std::vector<Row> rows;
  while(std::getline(file,line)) {
    std::stringstream ss(line); std::string cell; std::vector<std::string> cells;
    while(std::getline(ss,cell,',')) cells.push_back(cell);
    if(cells.size()==8) rows.push_back({(uint32_t)std::stoul(cells[3]), std::stof(cells[4]),std::stof(cells[6])});
  }
  if(rows.empty()) return 1;
  float sx=0,sz=0; unsigned n=0;
  for(auto r:rows) if(r.ms-rows[0].ms<2000) { sx+=r.x; sz+=r.z; ++n; }
  RepDetector d; d.reset(sx/n,sz/n); d.fullThreshold=15;
  uint32_t last=0;
  for(auto r:rows) {
    if(r.ms-rows[0].ms<2000 || r.ms-last<50) continue;
    last=r.ms;
    int event=d.update(r.x,r.z,r.ms);
    if(event) std::cout<<"event "<<r.ms<<" type "<<event<<" peak "<<d.peak<<"\n";
  }
  std::cout<<"full="<<d.full<<" partial="<<d.partial<<" rejected="<<d.rejected<<"\n";
}
