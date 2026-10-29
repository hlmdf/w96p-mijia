#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <WebServer.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <NimBLEDevice.h>
#include "protocol.h"
#include "panel.h"
#include "bemfa_ca.h"
#include "vendor_crc.h"
#include "pending_speed.h"
#include "control_policy.h"
#include "light_auto_off.h"

static const char *FAN_MAC="d0:0c:5e:65:f9:c4";
static const char *SPEED_TOPIC="w96speed002", *LIGHT_TOPIC="w96light002";
static WebServer web(80);
static Preferences prefs;
static WiFiClientSecure tls;
static PubSubClient mqtt(tls);
static String uid,csrf;
static QueueHandle_t queue;
static SemaphoreHandle_t lock;
static NimBLEClient *fan;
static volatile bool bleConnected=false,allowReconnect=true;
static volatile uint32_t linkGeneration=0;
static volatile bool keepAwakeVerified=false;
static volatile uint32_t backgroundConnectAttempts=0;
static uint32_t nextId=1;
static String serialLine;
static uint32_t serialPending[16]={};
static PendingSpeed pendingCloudSpeed;
static volatile uint32_t cloudSpeedRevision=0;
static uint32_t pendingSpeedLink=0;
static int lastExplicitCloudSpeed=-1;
static uint32_t lastExplicitCloudAt=0;
static void cloudMessage(char *topic,uint8_t *bytes,unsigned int n);
static bool ensureKeepAwake();
static void observeLight(int value);
enum class Op:uint8_t {Read,Write,Info,Connect,Disconnect,Speed,Light,Status,LinkDrop};
struct Job {uint32_t id=0,created=0,generation=0,speedRevision=0;Op op=Op::Read;uint16_t uuid=0,len=0;uint8_t data[160]={};};
struct Result {uint32_t id=0;bool done=false,ok=false;String hex,error,sn,version,diagnostic;};
static Result results[32];
static int actualSpeed=-1,actualLight=-1,actualPower=-1;
static uint8_t rememberedSpeed=10,rememberedLight=1;
static uint32_t stateAt=0;
static volatile bool lightAutoOffArmed=false;
static volatile uint32_t lightAutoOffDeadline=0;
static int observedLight=-1;
static volatile bool infoWaiting=false,infoReady=false;
static uint8_t infoPayload[300];
static size_t infoLen=0;
static uint8_t infoFrame[306];
static size_t infoUsed=0,infoExpected=0;
static String infoDiagnostic;
static uint8_t infoTrace[320];
static volatile size_t infoTraceLength=0;

static void clearState(){keepAwakeVerified=false;observedLight=-1;xSemaphoreTake(lock,portMAX_DELAY);actualSpeed=actualLight=actualPower=-1;stateAt=0;xSemaphoreGive(lock);}
static NimBLERemoteCharacteristic *characteristic(uint16_t u){
  if(!fan||!fan->isConnected())return nullptr;
  auto s=fan->getService(NimBLEUUID(serviceOf(u)));return s?s->getCharacteristic(NimBLEUUID(u)):nullptr;
}
static bool connectFan(){
  if(fan->isConnected()){bleConnected=true;ensureKeepAwake();return true;}
  clearState();bleConnected=false;
  if(!fan->connect(NimBLEAddress(FAN_MAC)))return false;
  if(!characteristic(0xfff3)){fan->disconnect();return false;}
  ++linkGeneration;bleConnected=true;ensureKeepAwake();return true;
}
static bool readRaw(uint16_t u,String &out){
  auto c=characteristic(u);if(!c||!c->canRead())return false;
  auto v=c->readValue();if(v.size()==0)return false;
  size_t minimum=1;
  if(u==0xfff2||u==0xfff5||u==0xfff8||u==0xfffb)minimum=2;
  if(u==0xfff7)minimum=4;
  if(u==0xffd1||u==0xffd2)minimum=8;
  if(u==0xffd3)minimum=5;
  if(v.size()<minimum||(u==0xffe3&&v.size()!=128))return false;
  out=encodeHex(reinterpret_cast<const uint8_t*>(v.data()),v.size());return true;
}
static bool writeRaw(uint16_t u,const uint8_t *bytes,size_t n,String &error){
  if(!validWrite(u,bytes,n)){error="指令参数超出范围";return false;}
  if(u==0xfffa&&n==1&&bytes[0]==0)lightAutoOffArmed=false;
  if((u==0xfff1||u==0xfff3)&&n==1)Serial.printf("SPEED_WRITE ms=%lu register=%04x value=%u\n",(unsigned long)millis(),u,bytes[0]);
  if(u==0xfffa&&n==1)Serial.printf("LIGHT_WRITE ms=%lu value=%u\n",(unsigned long)millis(),bytes[0]);
  auto c=characteristic(u);if(!c||(!c->canWrite()&&!c->canWriteNoResponse())){error="设备不支持该写入";return false;}
  if(n>20&&fan->getMTU()<n+3){error="蓝牙 MTU 不足，无法完整写入曲线";return false;}
  if(!c->writeValue(bytes,n,c->canWrite())){error="蓝牙写入失败";return false;}
  if(u==0xfffc){auto remaining=characteristic(0xfffb);if(!remaining||!remaining->canRead()){error="已发送 Turbo 指令，但无法确认状态";return false;}vTaskDelay(pdMS_TO_TICKS(100));auto r=remaining->readValue();if(r.size()<2||((be16((const uint8_t*)r.data())>0)!=(bytes[0]!=0))){error="Turbo 状态尚未确认，请刷新后查看";return false;}return true;}
  if(!c->canRead()){error="写入已发送，但设备不支持读取确认";return false;}
  auto v=c->readValue();bool matches=v.size()==n&&memcmp(v.data(),bytes,n)==0;
  if((u==0xfff2||u==0xfff8)&&v.size()>=2&&n==2){uint16_t want=be16(bytes),got=be16((const uint8_t*)v.data());matches=(u==0xfff8&&want==0)?(got==0||got==199):(got<=want&&got+2>=want);}
  if(u==0xffc1){String wanted;for(size_t i=0;i<n;i++)wanted+=char(bytes[i]);matches=String(v.c_str()).indexOf(wanted.substring(0,wanted.length()-1))>=0;}
  if(u==0xffd1&&v.size()>=8){String s;for(size_t i=0;i<n;i++)s+=char(bytes[i]);uint32_t want=s.substring(8).toInt();const uint8_t*p=(const uint8_t*)v.data();uint32_t got=(uint32_t(p[4])<<24)|(uint32_t(p[5])<<16)|(uint32_t(p[6])<<8)|p[7];matches=want==got;}
  if(!matches){error="指令已发送，但回读未确认；请刷新后查看设备状态";return false;}
  if(u==0xfffa){observedLight=bytes[0];lightAutoOffArmed=bytes[0]>0;if(lightAutoOffArmed)lightAutoOffDeadline=millis()+LIGHT_AUTO_OFF_MS;}
  return true;
}
static bool byteWrite(uint16_t u,uint8_t b,String &e){return writeRaw(u,&b,1,e);}
static void observeLight(int value){
  if(value<0||value>4)return;
  if(value==0)lightAutoOffArmed=false;
  else if(value!=observedLight){lightAutoOffArmed=true;lightAutoOffDeadline=millis()+LIGHT_AUTO_OFF_MS;}
  observedLight=value;
}
static void autoOffLight(){
  if(!fan->isConnected()||!lightOffDue(lightAutoOffArmed,millis(),lightAutoOffDeadline))return;
  String error;
  if(byteWrite(0xfffa,0,error)){xSemaphoreTake(lock,portMAX_DELAY);actualLight=0;xSemaphoreGive(lock);Serial.println("LIGHT_AUTO_OFF complete");}
  else{lightAutoOffArmed=true;lightAutoOffDeadline=millis()+2000;Serial.println("LIGHT_AUTO_OFF retry");}
}
static bool ensureKeepAwake(){
  String current,error;keepAwakeVerified=false;
  if(!readRaw(0xfff5,current))return false;
  if(current!="0000"){const uint8_t zero[]={0,0};if(!writeRaw(0xfff5,zero,2,error))return false;}
  keepAwakeVerified=true;return true;
}
static bool speedBurstByte(uint16_t u,uint8_t value,String &error){
  auto c=characteristic(u);
  if(!c||(!c->canWrite()&&!c->canWriteNoResponse())){error="风速控制特征不可写";return false;}
  Serial.printf("SPEED_BURST ms=%lu register=%04x value=%u\n",(unsigned long)millis(),u,value);
  if(!c->writeValue(&value,1,!c->canWriteNoResponse())){error="风速指令发送失败";return false;}
  return true;
}
static void refreshBasic(bool detectShutdown=true){
  int values[3]={-1,-1,-1};uint16_t uuids[]={0xfff3,0xfffa,0xfff1};
  for(size_t i=0;i<3;i++){auto c=characteristic(uuids[i]);if(c&&c->canRead()){auto v=c->readValue();if(v.size()==1)values[i]=uint8_t(v[0]);}}
  xSemaphoreTake(lock,portMAX_DELAY);int oldPower=actualPower,oldSpeed=actualSpeed;xSemaphoreGive(lock);
  if(detectShutdown&&motorBecameOff(oldPower,oldSpeed,values[2],values[0])){
    lightAutoOffArmed=false;
    if(values[1]>0){String error;if(byteWrite(0xfffa,0,error))values[1]=0;else Serial.println("Coupled light-off not confirmed");}
  }
  xSemaphoreTake(lock,portMAX_DELAY);actualSpeed=values[0];actualLight=values[1];actualPower=values[2];
  if(actualSpeed>0&&actualSpeed<=100)rememberedSpeed=actualSpeed;
  if(actualLight>0&&actualLight<=4)rememberedLight=actualLight;
  stateAt=millis();xSemaphoreGive(lock);
  observeLight(values[1]);
}
static void infoNotify(NimBLERemoteCharacteristic*,uint8_t *data,size_t len,bool){
  uint8_t *frame=infoFrame;size_t &used=infoUsed,&expected=infoExpected;
  if(!infoWaiting){used=0;return;}
  for(size_t i=0;i<len&&infoTraceLength<sizeof(infoTrace);i++)infoTrace[infoTraceLength++]=data[i];
  for(size_t i=0;i<len;i++){
    uint8_t b=data[i];if(used==0&&b!=0x55)continue;if(used>=sizeof(infoFrame)){used=0;continue;}frame[used++]=b;
    if(used==4){uint8_t mask=VENDOR_CRC[frame[1]];expected=(frame[2]^mask)|((frame[3]^mask)<<8);if(expected==0||expected>300){used=0;continue;}}
    if(used>=4&&used==expected+5){uint8_t crc=137,mask=VENDOR_CRC[frame[1]];
      for(size_t k=0;k<used-1;k++){uint8_t d=k>=2?(frame[k]^mask):frame[k];crc=VENDOR_CRC[crc^d];}
      if(crc==(frame[used-1]^mask)){infoLen=expected;for(size_t k=0;k<expected;k++)infoPayload[k]=frame[k+4]^mask;infoReady=true;infoWaiting=false;}used=0;
    }
  }
}
static bool requestInfo(uint8_t command,uint8_t *out,size_t &n){
  infoDiagnostic="";
  auto service=fan->getService(NimBLEUUID(uint16_t(0xfee0)));if(!service){infoDiagnostic="FEE0 service missing";return false;}
  auto tx=service->getCharacteristic(NimBLEUUID(uint16_t(0xfee1)));auto rx=service->getCharacteristic(NimBLEUUID(uint16_t(0xfee2)));
  if(!tx||!rx){infoDiagnostic="FEE1/FEE2 missing";return false;}
  if(!rx->canNotify()||!rx->subscribe(true,infoNotify,true)){infoDiagnostic="notification subscription failed";return false;}
  uint8_t frame[]={0x55,0,1,0,command,0};uint8_t crc=137;for(size_t i=0;i<5;i++)crc=VENDOR_CRC[crc^frame[i]];frame[5]=crc;
  infoReady=false;infoUsed=infoExpected=0;infoTraceLength=0;infoWaiting=true;bool sent=tx->writeValue(frame,sizeof(frame),false);uint32_t start=millis();
  while(sent&&!infoReady&&fan->isConnected()&&millis()-start<5000)vTaskDelay(pdMS_TO_TICKS(10));
  infoWaiting=false;bool ok=sent&&infoReady;if(ok){n=infoLen;memcpy(out,infoPayload,n);}else{infoDiagnostic=String("sent=")+(sent?"true":"false")+", rx="+encodeHex(infoTrace,infoTraceLength);}rx->unsubscribe(true);return ok;
}
static Result perform(const Job &j){
  Result r;r.id=j.id;r.done=true;if(millis()-j.created>15000){r.error="指令过期，已丢弃";return r;}
  if(j.op==Op::Status){
    StaticJsonDocument<768>d;d["apIP"]=WiFi.softAPIP().toString();d["apClients"]=WiFi.softAPgetStationNum();d["stationIP"]=WiFi.localIP().toString();d["wifiStatus"]=int(WiFi.status());d["freeHeap"]=ESP.getFreeHeap();
    WiFiClient probe;bool connected=probe.connect(WiFi.softAPIP(),80);d["httpConnected"]=connected;
    if(connected){probe.print("GET / HTTP/1.1\r\nHost: 192.168.4.1\r\nConnection: close\r\n\r\n");uint32_t start=millis();size_t received=0;String header;bool headersDone=false;
      while(millis()-start<7000&&(probe.connected()||probe.available())){while(probe.available()){char c=probe.read();if(headersDone){received++;}else{header+=c;if(header.endsWith("\r\n\r\n"))headersDone=true;if(header.length()>2048)break;}}vTaskDelay(pdMS_TO_TICKS(1));}
      d["httpHeader"]=header;d["receivedBodyBytes"]=received;d["expectedBodyBytes"]=strlen(PANEL_HTML)-strlen("__W96P_CSRF__")+csrf.length();probe.stop();
    }
    serializeJson(d,r.diagnostic);r.ok=true;return r;
  }
  if(j.op==Op::Disconnect){allowReconnect=false;fan->disconnect();bleConnected=false;clearState();r.ok=true;return r;}
  if(j.op==Op::LinkDrop){if(!allowReconnect){r.error="已主动断开，保持断开";return r;}fan->disconnect();bleConnected=false;clearState();r.ok=true;return r;}
  if(j.op==Op::Connect){allowReconnect=true;r.ok=connectFan();if(!r.ok)r.error="无法连接，请确认风扇已唤醒且其他蓝牙控制器已断开";return r;}
  if(!fan->isConnected()){r.error="风扇未连接";return r;}
  if(j.generation!=linkGeneration){r.error="蓝牙连接已变化，旧指令已丢弃";return r;}
  if(j.speedRevision&&j.speedRevision!=cloudSpeedRevision){r.error="已由更新的风速指令替代";return r;}
  if(j.op==Op::Read){r.ok=readRaw(j.uuid,r.hex);if(!r.ok)r.error="读取失败或该固件未提供此功能";return r;}
  if(j.op==Op::Write){
    if(j.uuid==0xfff5&&j.len==2&&be16(j.data)!=0){r.error="米家常在线模式下，蓝牙必须设为永不关闭";return r;}
    if((j.uuid==0xfff3&&j.len==1)||motorOffWrite(j.uuid,j.len,j.data[0])){Job speed=j;speed.op=Op::Speed;return perform(speed);}
    r.ok=writeRaw(j.uuid,j.data,j.len,r.error);return r;
  }
  if(j.op==Op::Info){uint8_t p[300];size_t n=0;bool a=requestInfo(0x8a,p,n);if(a&&n>=2&&(p[0]&0x7f)==4)r.version=String(p[1]/10)+"."+String(p[1]%10);
    String diagnosticA=infoDiagnostic;bool b=requestInfo(0x8f,p,n);if(b&&n>=5&&(p[0]&0x7f)==10){uint32_t sn=uint32_t(p[1])|(uint32_t(p[2])<<8)|(uint32_t(p[3])<<16)|(uint32_t(p[4])<<24);r.sn=String(sn);}
    r.ok=r.version.length()||r.sn.length();if(!r.ok)r.error="设备信息请求失败: version["+diagnosticA+"] sn["+infoDiagnostic+"]";return r;
  }
  int v=j.data[0];if(j.op==Op::Light){r.ok=byteWrite(0xfffa,v,r.error);return r;}
  if(j.op==Op::Speed){
    if(v==0){
      // Shutdown must not depend on optional mode reads/writes. Attempt both outputs.
      lightAutoOffArmed=false;
      String motorError,lightError,optionalError;
      speedBurstByte(0xfffc,0,optionalError);speedBurstByte(0xfff4,0,optionalError);
      bool motorWritten=byteWrite(0xfff1,0,motorError);
      bool lightWritten=byteWrite(0xfffa,0,lightError);
      String power,speed,light;
      bool powerRead=readRaw(0xfff1,power),speedRead=readRaw(0xfff3,speed),lightRead=readRaw(0xfffa,light);
      r.ok=motorWritten&&lightWritten&&powerRead&&speedRead&&lightRead&&power=="00"&&speed=="00"&&light=="00";
      if(!r.ok)r.error=(!motorWritten||!powerRead||power!="00")?"风扇未确认关闭，请刷新状态":"灯光未确认关闭，请刷新状态";
      Serial.printf("SPEED_APPLIED ms=%lu target=0 actual=%lu power=%lu ok=%u\n",(unsigned long)millis(),speedRead?strtoul(speed.c_str(),nullptr,16):255UL,powerRead?strtoul(power.c_str(),nullptr,16):255UL,r.ok);
      return r;
    }
    auto c=characteristic(0xfff1);if(!c){r.error="无法读取开关状态";return r;}auto p=c->readValue();if(p.size()==0){r.error="无法确认开关状态";return r;}
    if(!speedBurstByte(0xfffc,0,r.error)||!speedBurstByte(0xfff4,0,r.error))return r;
    if(j.speedRevision&&j.speedRevision!=cloudSpeedRevision){r.error="已由更新的风速指令替代";return r;}
    // Keep power-on and target-speed adjacent. Verify only after both writes.
    if(p[0]==0&&!speedBurstByte(0xfff1,1,r.error))return r;
    if(!speedBurstByte(0xfff3,v,r.error))return r;
    String hSpeed,hPower,hNature,hTurbo;
    bool readOk=readRaw(0xfff3,hSpeed)&&readRaw(0xfff1,hPower)&&readRaw(0xfff4,hNature)&&readRaw(0xfffb,hTurbo);
    uint8_t actual=readOk?strtoul(hSpeed.c_str(),nullptr,16):255;
    unsigned power=readOk?strtoul(hPower.c_str(),nullptr,16):255;
    r.ok=readOk&&actual==v&&power>0&&hNature=="00"&&hTurbo=="0000";
    if(!r.ok)r.error="风速或模式回读未确认，请刷新状态";
    Serial.printf("SPEED_APPLIED ms=%lu target=%d actual=%u power=%u ok=%u\n",(unsigned long)millis(),v,actual,power,r.ok);
    return r;
  }
  r.error="未知操作";return r;
}
static void worker(void*){
  fan=NimBLEDevice::createClient();fan->setConnectTimeout(6);
  uint32_t lastConnect=0,lastRead=0,lastAwakeCheck=0;
  for(;;){bleConnected=fan->isConnected();if(!bleConnected){clearState();if(allowReconnect&&millis()-lastConnect>10000){lastConnect=millis();++backgroundConnectAttempts;if(connectFan())lastAwakeCheck=millis();}}
    Job j;if(xQueueReceive(queue,&j,pdMS_TO_TICKS(30))==pdTRUE){Result r=perform(j);if(j.op==Op::Write||j.op==Op::Speed||j.op==Op::Light)refreshBasic(!(j.op==Op::Light||(j.op==Op::Write&&j.uuid==0xfffa)));
      xSemaphoreTake(lock,portMAX_DELAY);results[j.id%32]=r;xSemaphoreGive(lock);if(!r.ok)Serial.printf("BLE job %lu failed\n",(unsigned long)j.id);}
    if(fan->isConnected()&&millis()-lastRead>5000&&uxQueueMessagesWaiting(queue)==0){lastRead=millis();refreshBasic();}
    if(fan->isConnected()&&millis()-lastAwakeCheck>(keepAwakeVerified?60000:10000)&&uxQueueMessagesWaiting(queue)==0){lastAwakeCheck=millis();ensureKeepAwake();}
    autoOffLight();
  }
}
static bool submit(Job &j){j.id=nextId++;j.created=millis();j.generation=linkGeneration;xSemaphoreTake(lock,portMAX_DELAY);Result&r=results[j.id%32];r=Result();r.id=j.id;xSemaphoreGive(lock);return xQueueSend(queue,&j,0)==pdTRUE;}
static void flushCloudSpeed(){
  if(pendingCloudSpeed.target<0)return;
  if(!bleConnected||pendingSpeedLink!=linkGeneration){pendingCloudSpeed=PendingSpeed();return;}
  if(!pendingCloudSpeed.ready(millis()))return;
  Job j;j.op=Op::Speed;j.data[0]=pendingCloudSpeed.target;j.speedRevision=pendingCloudSpeed.revision;if(submit(j))pendingCloudSpeed=PendingSpeed();
}
static void jsonReply(int code,const JsonDocument &d){String s;serializeJson(d,s);web.sendHeader("Cache-Control","no-store");web.send(code,"application/json; charset=utf-8",s);}
static void fail(int code,const char *message){StaticJsonDocument<256>d;d["error"]=message;jsonReply(code,d);}
static bool authorized(){if(web.header("X-W96P-Key")!=csrf){fail(403,"页面已失效，请刷新后重试");return false;}return true;}
static bool parseConsoleJob(JsonDocument &d,Job &j){
  String op=d["op"]|"";
  if(op=="status"){j.op=Op::Status;return true;}
  if(op=="connect"){j.op=Op::Connect;return true;}
  if(op=="disconnect"){j.op=Op::Disconnect;return true;}
  if(op=="linkdrop"){j.op=Op::LinkDrop;return true;}
  if(op=="info"){j.op=Op::Info;return true;}
  if(op=="speed"||op=="light"){int v=d["value"]|-1;if(v<0||v>(op=="speed"?100:4))return false;j.op=op=="speed"?Op::Speed:Op::Light;j.data[0]=v;return true;}
  if(op!="read"&&op!="write")return false;
  j.op=op=="read"?Op::Read:Op::Write;String u=d["uuid"]|"";
  if(u.length()!=4)return false;for(char c:u){int v=hexDigit(c);if(v<0)return false;j.uuid=(j.uuid<<4)|v;}
  if(!readable(j.uuid))return false;
  if(j.op==Op::Read)return true;
  String h=d["hex"]|"";if(h.length()==0||h.length()>320||h.length()%2)return false;j.len=h.length()/2;
  for(size_t i=0;i<j.len;i++){int a=hexDigit(h[i*2]),b=hexDigit(h[i*2+1]);if(a<0||b<0)return false;j.data[i]=(a<<4)|b;}
  return validWrite(j.uuid,j.data,j.len);
}
static void serialLoop(){
  while(Serial.available()){
    char c=Serial.read();if(c=='\r')continue;
    if(c!='\n'){if(serialLine.length()<900)serialLine+=c;else serialLine="";continue;}
    if(!serialLine.startsWith("@")){serialLine="";continue;}
    StaticJsonDocument<1100>d;Job j;
    bool parsed=!deserializeJson(d,serialLine.substring(1));serialLine="";
    String injectOp=d["op"]|"";
    if(parsed&&(injectOp=="cloudspeed"||injectOp=="cloudlight")){
      String message=d["message"]|"";int value;
      if(!bleConnected||!lampCommand(message,value)){Serial.println("@{\"accepted\":false}");continue;}
      cloudMessage(const_cast<char*>(injectOp=="cloudspeed"?SPEED_TOPIC:LIGHT_TOPIC),reinterpret_cast<uint8_t*>(const_cast<char*>(message.c_str())),message.length());
      uint32_t id=nextId++;Serial.printf("@{\"accepted\":true,\"id\":%lu}\n@{\"done\":true,\"ok\":true,\"id\":%lu}\n",(unsigned long)id,(unsigned long)id);continue;
    }
    int slot=-1;for(int i=0;i<16;i++)if(!serialPending[i]){slot=i;break;}
    if(!parsed||!parseConsoleJob(d,j)||slot<0||!submit(j)){Serial.println("@{\"accepted\":false}");continue;}
    serialPending[slot]=j.id;Serial.printf("@{\"accepted\":true,\"id\":%lu}\n",(unsigned long)j.id);
  }
  for(int i=0;i<16;i++)if(serialPending[i]){
    uint32_t id=serialPending[i];xSemaphoreTake(lock,portMAX_DELAY);Result r=results[id%32];xSemaphoreGive(lock);
    if(r.id!=id){serialPending[i]=0;continue;}if(!r.done)continue;
    StaticJsonDocument<2300>d;d["id"]=id;d["done"]=true;d["ok"]=r.ok;d["hex"]=r.hex;d["error"]=r.error;d["sn"]=r.sn;d["version"]=r.version;d["diagnostic"]=r.diagnostic;
    Serial.print('@');serializeJson(d,Serial);Serial.println();serialPending[i]=0;
  }
}
static void postJob(){
  if(!authorized())return;if(web.arg("plain").length()>900){fail(413,"请求过大");return;}
  StaticJsonDocument<1024>d;if(deserializeJson(d,web.arg("plain"))){fail(400,"请求格式错误");return;}String op=d["op"]|"";Job j;
  if(op=="connect")j.op=Op::Connect;else if(op=="disconnect")j.op=Op::Disconnect;else if(op=="info")j.op=Op::Info;
  else if(op=="read"||op=="write"){
    j.op=op=="read"?Op::Read:Op::Write;String u=d["uuid"]|"";
    if(u.length()!=36||!u.startsWith("0000")||u.substring(8)!="-0000-1000-8000-00805f9b34fb"){fail(400,"UUID 格式错误");return;}
    for(int i=4;i<8;i++){int v=hexDigit(u[i]);if(v<0){fail(400,"UUID 格式错误");return;}j.uuid=(j.uuid<<4)|v;}
    if(!readable(j.uuid)){fail(400,"该特征不开放");return;}
    if(j.op==Op::Write){String h=d["hex"]|"";if(h.length()==0||h.length()>320||h.length()%2){fail(400,"数据长度错误");return;}j.len=h.length()/2;
      for(size_t i=0;i<j.len;i++){int a=hexDigit(h[i*2]),b=hexDigit(h[i*2+1]);if(a<0||b<0){fail(400,"数据格式错误");return;}j.data[i]=(a<<4)|b;}
      if(!validWrite(j.uuid,j.data,j.len)){fail(400,"设置超出允许范围");return;}
    }
  }else{fail(400,"未知操作");return;}
  if(j.op!=Op::Connect&&j.op!=Op::Disconnect&&!bleConnected){fail(409,"风扇未连接，请先连接风扇");return;}
  if(!submit(j)){fail(429,"设备忙，请稍后重试");return;}StaticJsonDocument<64>reply;reply["id"]=j.id;jsonReply(202,reply);
}
static void getJob(){uint32_t id=web.arg("id").toInt();xSemaphoreTake(lock,portMAX_DELAY);Result r=results[id%32];xSemaphoreGive(lock);if(!id||r.id!=id){fail(404,"任务不存在或已失效");return;}
  StaticJsonDocument<1400>d;d["done"]=r.done;d["ok"]=r.ok;d["hex"]=r.hex;d["error"]=r.error;d["sn"]=r.sn;d["version"]=r.version;jsonReply(200,d);
}
static bool validUid(const String&s){if(s.length()!=32)return false;for(char c:s)if(hexDigit(c)<0)return false;return true;}
static void cloudMessage(char *topic,uint8_t *bytes,unsigned int n){
  if(n>16||!bleConnected)return;String s;for(unsigned i=0;i<n;i++)s+=char(bytes[i]);int v;if(!lampCommand(s,v))return;Job j;
  if(String(topic)==SPEED_TOPIC)Serial.printf("CLOUD_SPEED_RX ms=%lu command=%s\n",(unsigned long)millis(),s.c_str());
  if(String(topic)==LIGHT_TOPIC)Serial.printf("CLOUD_LIGHT_RX ms=%lu command=%s\n",(unsigned long)millis(),s.c_str());
  if(String(topic)==SPEED_TOPIC){
    if(companionOn(v,lastExplicitCloudSpeed,millis(),lastExplicitCloudAt)){Serial.println("CLOUD_SPEED_SUPPRESS companion on");return;}
    uint32_t revision=++cloudSpeedRevision;
    if(v==0){pendingCloudSpeed=PendingSpeed();lastExplicitCloudSpeed=-1;j.op=Op::Speed;j.data[0]=0;j.speedRevision=revision;submit(j);return;}
    if(v>0){lastExplicitCloudSpeed=v;lastExplicitCloudAt=millis();}
    xSemaphoreTake(lock,portMAX_DELAY);int restored=rememberedSpeed;xSemaphoreGive(lock);
    pendingCloudSpeed=mergeSpeed(pendingCloudSpeed,v,restored,millis(),revision);pendingSpeedLink=linkGeneration;return;
  }
  xSemaphoreTake(lock,portMAX_DELAY);
  if(String(topic)==LIGHT_TOPIC){j.op=Op::Light;j.data[0]=v<0?rememberedLight:v==0?0:(v+24)/25;}else{xSemaphoreGive(lock);return;}xSemaphoreGive(lock);submit(j);
}
static void cloudLoop(){
  if(!validUid(uid)||WiFi.status()!=WL_CONNECTED)return;static uint32_t lastTry=0,lastReport=0;
  if(!mqtt.connected()){if(millis()-lastTry<15000)return;lastTry=millis();if(time(nullptr)<1700000000)return;if(mqtt.connect(uid.c_str())){mqtt.subscribe(SPEED_TOPIC);mqtt.subscribe(LIGHT_TOPIC);}return;}
  mqtt.loop();if(millis()-lastReport<5000)return;lastReport=millis();xSemaphoreTake(lock,portMAX_DELAY);int s=actualSpeed,p=actualPower,l=actualLight;bool fresh=bleConnected&&stateAt&&millis()-stateAt<15000;xSemaphoreGive(lock);if(!fresh)return;
  if(s>=0&&s<=100&&p>=0){String msg=(p==0||s==0)?"off":"on#"+String(s);mqtt.publish((String(SPEED_TOPIC)+"/up").c_str(),msg.c_str());}
  if(l>=0&&l<=4){String msg=l==0?"off":"on#"+String(l*25);mqtt.publish((String(LIGHT_TOPIC)+"/up").c_str(),msg.c_str());}
}
void setup(){
  Serial.setRxBufferSize(2048);Serial.begin(115200);prefs.begin("w96bridge",false);uid=prefs.getString("uid","");char token[33];for(size_t i=0;i<4;i++)snprintf(token+i*8,9,"%08lx",(unsigned long)esp_random());csrf=token;
  if(prefs.isKey("lightIntent"))prefs.remove("lightIntent"); // remove the mistaken keep-on setting
  String apPass=prefs.getString("apPass","");if(apPass.length()<12){apPass=csrf.substring(0,12);prefs.putString("apPass",apPass);}
  Serial.println("W96P C3 bridge starting. No fan control is sent at boot.");Serial.printf("Setup AP: W96P-C3-Setup; password: %s\n",apPass.c_str());
  // Start BLE and the local panel immediately, including before Wi-Fi provisioning.
  WiFi.mode(WIFI_AP_STA);WiFi.setAutoReconnect(true);WiFi.begin();
  WiFi.softAP("W96P-C3-Setup",apPass.c_str());
  configTime(0,0,"ntp.aliyun.com","pool.ntp.org");tls.setCACert(BEMFA_ROOT_CA);tls.setHandshakeTimeout(5);mqtt.setServer("bemfa.com",9503);mqtt.setSocketTimeout(3);mqtt.setCallback(cloudMessage);
  const char *headers[]={"X-W96P-Key"};web.collectHeaders(headers,1);
  web.on("/",HTTP_GET,[]{const char *marker=strstr(PANEL_HTML,"__W96P_CSRF__");web.sendHeader("Cache-Control","no-store");web.sendHeader("X-Content-Type-Options","nosniff");web.sendHeader("X-Frame-Options","DENY");web.setContentLength(strlen(PANEL_HTML)-strlen("__W96P_CSRF__")+csrf.length());web.send(200,"text/html; charset=utf-8","");web.sendContent(PANEL_HTML,marker-PANEL_HTML);web.sendContent(csrf);web.sendContent(marker+strlen("__W96P_CSRF__"));});
  web.on("/api/job",HTTP_POST,postJob);web.on("/api/job",HTTP_GET,getJob);
  web.on("/api/status",HTTP_GET,[]{StaticJsonDocument<768>d;d["ip"]=WiFi.status()==WL_CONNECTED?WiFi.localIP().toString():WiFi.softAPIP().toString();d["ble"]=bleConnected;d["cloud"]=mqtt.connected();d["configured"]=validUid(uid);d["bleKeepAwake"]=keepAwakeVerified;d["bleAutoReconnect"]=allowReconnect;d["bleReconnectAttempts"]=backgroundConnectAttempts;d["uptimeMs"]=millis();d["freeHeap"]=ESP.getFreeHeap();d["lightAutoOffSeconds"]=LIGHT_AUTO_OFF_MS/1000;d["lightAutoOffArmed"]=lightAutoOffArmed;d["lightKeepOn"]=false;jsonReply(200,d);});
  web.on("/api/cloud",HTTP_POST,[]{if(!authorized())return;StaticJsonDocument<128>d;if(web.arg("plain").length()>128||deserializeJson(d,web.arg("plain"))){fail(400,"请求格式错误");return;}String value=d["uid"]|"";if(!validUid(value)){fail(400,"UID 应为 32 位十六进制私钥");return;}mqtt.disconnect();uid=value;prefs.putString("uid",uid);StaticJsonDocument<32>r;r["ok"]=true;jsonReply(200,r);});
  web.on("/api/wifi",HTTP_POST,[]{if(!authorized())return;StaticJsonDocument<256>d;if(web.arg("plain").length()>250||deserializeJson(d,web.arg("plain"))){fail(400,"请求格式错误");return;}String ssid=d["ssid"]|"",password=d["password"]|"";if(ssid.length()==0||ssid.length()>32||(password.length()!=0&&password.length()<8)||password.length()>63){fail(400,"请填写有效的名称和 Wi-Fi 密码");return;}StaticJsonDocument<32>r;r["ok"]=true;jsonReply(200,r);mqtt.disconnect();WiFi.persistent(true);WiFi.setAutoReconnect(true);WiFi.begin(ssid.c_str(),password.c_str());});
  web.onNotFound([]{fail(404,"地址不存在");});lock=xSemaphoreCreateMutex();queue=xQueueCreate(12,sizeof(Job));web.begin();
  NimBLEDevice::init("W96P-C3-Bridge");NimBLEDevice::setMTU(247);xTaskCreate(worker,"w96ble",10000,nullptr,1,nullptr);
  Serial.printf("Panel: http://%s/\n",(WiFi.status()==WL_CONNECTED?WiFi.localIP():WiFi.softAPIP()).toString().c_str());
}
void loop(){serialLoop();web.handleClient();cloudLoop();flushCloudSpeed();delay(2);}
