#pragma once
#include <stdint.h>
constexpr uint32_t LIGHT_AUTO_OFF_MS=15000;
constexpr bool lightOffDue(bool armed,uint32_t now,uint32_t deadline){
  return armed && static_cast<int32_t>(now-deadline)>=0;
}
