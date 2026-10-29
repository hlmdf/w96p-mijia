#pragma once
#include <Arduino.h>
inline uint16_t be16(const uint8_t *b){return (uint16_t(b[0])<<8)|b[1];}
inline bool readable(uint16_t u){return (u>=0xfff1&&u<=0xfff8)||u==0xfffa||u==0xfffb||u==0xfffc||(u>=0xffd1&&u<=0xffd3)||u==0xffe3||u==0xffc1;}
inline uint16_t serviceOf(uint16_t u){if(u==0xffe3)return 0xffe0;if(u==0xffc1)return 0xffc0;if(u>=0xffd1&&u<=0xffd3)return 0xffd0;return 0xfff0;}
inline bool validWrite(uint16_t u,const uint8_t *b,size_t n){
  if(n==1){if(u==0xfff1||u==0xfffa)return b[0]<=4;if(u==0xfff3)return b[0]<=100;if(u==0xfff4||u==0xfff6||u==0xfffc)return b[0]<=1;}
  if(n==2){uint16_t v=be16(b);if(u==0xfff2)return v<=28800;if(u==0xfff5)return v==0||v>=10;if(u==0xfff8)return v<=600;}
  if((u==0xfff7&&n==4)||(u==0xffe3&&n==128)){for(size_t i=0;i<n;i++)if(b[i]>100)return false;return true;}
  String s;for(size_t i=0;i<n;i++)s+=char(b[i]);
  if(u==0xffc1)return s=="BLE_SN=0,"||s=="BLE_SN=1,";
  if(u==0xffd1&&s.startsWith("BAT_CAP=")&&s.endsWith(",")){String t=s.substring(8,s.length()-1);if(t.length()==0||t.length()>7)return false;for(char c:t)if(c<'0'||c>'9')return false;long v=t.toInt();return v>=300&&v<=1500000;}
  return false;
}
inline int hexDigit(char c){if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;if(c>='A'&&c<='F')return c-'A'+10;return -1;}
inline String encodeHex(const uint8_t*p,size_t n){static const char h[]="0123456789abcdef";String s;s.reserve(n*2);for(size_t i=0;i<n;i++){s+=h[p[i]>>4];s+=h[p[i]&15];}return s;}
inline bool lampCommand(const String&s,int&v){if(s=="off"){v=0;return true;}if(s=="on"){v=-1;return true;}if(!s.startsWith("on#"))return false;String t=s.substring(3);if(t.length()==0||t.length()>3)return false;for(char c:t)if(c<'0'||c>'9')return false;v=t.toInt();return v>=1&&v<=100;}
