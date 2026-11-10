#pragma once
#include <stdint.h>
// Normalized 10-bit units; R4 thresholds = 1400 and 1550.
// Signal cycles only, not full/partial form classification.
struct YRepCounter {
 float filtered=0, history[5]={};
 bool initialized=false,armed=false,moving=false,highPending=false,lowPending=false;
 uint32_t highSince=0,lowSince=0,started=0,reps=0;
 uint8_t index=0;
 static constexpr float LOW=350.0f, HIGH=387.5f;
 void reset(){*this=YRepCounter();}
 bool update(float y,uint32_t now){
  if(!initialized){for(uint8_t i=0;i<5;++i)history[i]=y;filtered=y;initialized=true;}
  history[index]=y;index=(index+1)%5;
  float sorted[5];for(uint8_t i=0;i<5;++i)sorted[i]=history[i];
  for(uint8_t i=1;i<5;++i){float v=sorted[i];int8_t j=i-1;
   while(j>=0 && sorted[j]>v){sorted[j+1]=sorted[j];--j;}sorted[j+1]=v;}
  filtered+=0.5f*(sorted[2]-filtered);
  if(moving && now-started>10000){moving=armed=highPending=lowPending=false;}
  if(filtered>=HIGH){
   lowPending=false;
   if(!highPending){highSince=now;highPending=true;}
   if(now-highSince>=100){bool counted=moving && now-started>=500;
    moving=false;armed=true;if(counted){++reps;return true;}}
  }else{
   highPending=false;
   if(filtered<=LOW && armed && !moving){
    if(!lowPending){lowSince=now;lowPending=true;}
    if(now-lowSince>=100){moving=true;armed=false;started=now;lowPending=false;}
   }else lowPending=false;
  }
  return false;
 }
};
