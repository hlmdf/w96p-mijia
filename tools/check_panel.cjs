// Offline transport checks: no serial port, Bluetooth or real fan access.
const {test}=require('node:test');
const assert=require('node:assert/strict');
const vm=require('node:vm');
const fs=require('node:fs');
const html=fs.readFileSync('include/panel.h','utf8');
const script=html.match(/<script>([\s\S]*?)<\/script>/)[1];
new vm.Script(script);
const transport=script.slice(script.indexOf('const bridgeKey ='),script.indexOf('const proxyService='));
function context(fetch){const c={fetch,setTimeout:f=>f(),hexStringToBytes:h=>Uint8Array.from(h.match(/../g)||[],v=>parseInt(v,16)),Uint8Array,DataView,Error};vm.createContext(c);vm.runInContext(transport,c);return c;}
test('confirmed read returns original bytes and sends CSRF header',async()=>{
  let n=0;const c=context(async(url,options)=>{if(++n===1){assert.equal(options.headers['X-W96P-Key'],'__W96P_CSRF__');return {ok:true,json:async()=>({id:7})};}assert.equal(url,'/api/job?id=7');return {ok:true,json:async()=>({done:true,ok:true,hex:'0080ff'})};});
  const v=await vm.runInContext("proxyCharacteristic('u').readValue()",c);assert.equal(v.byteLength,3);assert.equal(v.getUint8(2),255);
});
test('BLE readback mismatch rejects the write instead of reporting success',async()=>{
  let n=0;const c=context(async()=>({ok:true,json:async()=>++n===1?{id:2}:{done:true,ok:false,error:'回读未确认'}}));
  await assert.rejects(vm.runInContext("proxyCharacteristic('u').writeValue(new Uint8Array([11]))",c),/回读未确认/);
});
test('full queue fails immediately and is not polled',async()=>{
  let n=0;const c=context(async()=>{n++;return {ok:false,json:async()=>({error:'设备忙'})};});
  await assert.rejects(vm.runInContext("bridgeJob('write','u','0b')",c),/设备忙/);assert.equal(n,1);
});
test('expired job fails without resubmitting a write',async()=>{
  let n=0;const c=context(async()=>++n===1?{ok:true,json:async()=>({id:1})}:{ok:false,json:async()=>({error:'任务已失效'})});
  await assert.rejects(vm.runInContext("bridgeJob('write','u','0b')",c),/任务已失效/);assert.equal(n,2);
});
test('all original controls remain and browser Bluetooth was removed',()=>{
  for(const id of ['curveCanvas','setTimerBtn','applySleepDelayBtn','applyTurboTimeBtn','setBatteryCapacityBtn','setSpeedsBtn','bleSnToggleBtn'])assert.ok(html.includes('id="'+id+'"'),id);
  assert.ok(!html.includes('navigator.bluetooth'));
  assert.ok(html.includes("if (!await safeWrite('LIGHT_CONTROL', hexValue)) return;"));
});
test('rapid speed input eventually sends the last target',async()=>{
  const body=script.slice(script.indexOf('async function sendSpeedImmediately('),script.indexOf('function onSpeedSliderChange('));
  let release;const sent=[];
  const c={state:{device:{gatt:{connected:true}},currentPower:'01',naturalWindOn:false,turboOn:false,mainService:{getCharacteristic:async()=>({writeValue:async b=>{sent.push(b[0]);if(sent.length===1)await new Promise(r=>release=r);}})}},CONFIG:{CHARACTERISTICS:{FAN_SPEED:'fff3'}},ensureTurboOff:async()=>{},utils:{alert:()=>{}},Uint8Array};
  vm.createContext(c);vm.runInContext(body,c);
  const first=vm.runInContext('sendSpeedImmediately(10)',c);
  await new Promise(r=>setImmediate(r));
  const second=vm.runInContext('sendSpeedImmediately(25)',c);
  const final=vm.runInContext('sendSpeedImmediately(37)',c);
  release();await Promise.all([first,second,final]);assert.deepEqual(sent,[10,37]);
});
