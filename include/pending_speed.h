#pragma once
#include <stdint.h>

struct PendingSpeed {
  int target;
  uint32_t due, revision;
  bool explicitPercent;
  constexpr PendingSpeed(int t=-1,uint32_t d=0,uint32_t r=0,bool e=false)
    :target(t),due(d),revision(r),explicitPercent(e){}
  constexpr bool ready(uint32_t now) const {
    return target>=0 && static_cast<int32_t>(now-due)>=0;
  }
};
// Bare on waits for a possible percentage companion; an explicit percentage wins.
constexpr PendingSpeed mergeSpeed(PendingSpeed old,int request,int remembered,uint32_t now,uint32_t revision){
  return request<0 && old.target>=0 && old.explicitPercent
    ? PendingSpeed(old.target,old.due,revision,true)
    : PendingSpeed(request<0?remembered:request,now+(request<0?650:80),revision,request>=0);
}
constexpr bool companionOn(int request,int lastPercent,uint32_t now,uint32_t lastPercentAt){
  return request<0 && lastPercent>=1 && uint32_t(now-lastPercentAt)<650;
}
