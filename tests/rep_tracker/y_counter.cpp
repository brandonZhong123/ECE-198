#include "../../RepTracker/YRepCounter.h"
#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <map>
#include <vector>
int main(int argc,char **argv){
 YRepCounter c;uint32_t time=0;
 auto hold=[&](float y,int n){for(int i=0;i<n;++i){c.update(y,time);time+=50;}};
 hold(340,40);assert(c.reps==0);
 hold(410,20);hold(320,20);assert(c.reps==0);
 hold(410,20);assert(c.reps==1);
 hold(410,50);assert(c.reps==1);
 hold(370,20);hold(410,20);assert(c.reps==1);
 hold(320,220);hold(410,20);assert(c.reps==1);
 c.reset();hold(410,20);hold(0,1);hold(410,20);assert(c.reps==0);
 std::cout<<"Y counter checks passed\n";
 if(argc<2)return 0;
 std::ifstream f(argv[1]);if(!f)return 1;
 std::string line;std::getline(f,line);
 std::map<std::string,YRepCounter> counters;std::map<std::string,uint32_t> last;
 while(std::getline(f,line)){
  std::stringstream ss(line);std::string cell;std::vector<std::string> v;
  while(std::getline(ss,cell,','))v.push_back(cell);
  if(v.size()!=8)continue;
  std::string key=v[0]+","+v[1];uint32_t ms=std::stoul(v[3]);
  if(last.count(key) && ms-last[key]<50)continue;
  last[key]=ms;
  if(counters[key].update(std::stof(v[5])*0.25f,ms))
   std::cout<<key<<" rep "<<counters[key].reps<<" at "<<ms<<" ms\n";
 }
 for(auto &v:counters)std::cout<<"Trial "<<v.first<<": "<<v.second.reps<<" reps\n";
}
