#pragma once
#include <stdint.h>
constexpr bool motorOffWrite(uint16_t uuid,uint16_t length,uint8_t value){
  return (uuid==0xfff1||uuid==0xfff3)&&length==1&&value==0;
}
constexpr bool motorBecameOff(int oldPower,int,int newPower,int){
  // Natural-wind curves may briefly contain 0%; that is not a shutdown action.
  return oldPower>0&&newPower==0;
}
