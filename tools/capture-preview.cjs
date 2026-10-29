// Offline preview only, against an isolated headless Chrome at port 9228.
const fs=require('node:fs');
(async()=>{
 const tabs=await(await fetch('http://127.0.0.1:9228/json/list')).json();
 const tab=tabs.find(t=>t.type==='page');if(!tab)throw Error('No preview tab');
 const ws=new WebSocket(tab.webSocketDebuggerUrl);await new Promise((r,j)=>{ws.onopen=r;ws.onerror=j;});
 let seq=0;const pending=new Map();ws.onmessage=e=>{const j=JSON.parse(e.data);const p=pending.get(j.id);if(p){pending.delete(j.id);j.error?p.reject(Error(j.error.message)):p.resolve(j.result);}};
 const send=(method,params={})=>new Promise((resolve,reject)=>{const id=++seq;pending.set(id,{resolve,reject});ws.send(JSON.stringify({id,method,params}));});
 await send('Emulation.setDeviceMetricsOverride',{width:390,height:844,deviceScaleFactor:1,mobile:true});
 await send('Page.navigate',{url:'http://127.0.0.1:8769/'});
 await new Promise(r=>setTimeout(r,1500));
 const metrics=await send('Runtime.evaluate',{expression:'JSON.stringify({width:innerWidth,scroll:document.documentElement.scrollWidth,curve:!!document.getElementById("curveCanvas"),height:document.body.scrollHeight})',returnByValue:true});
 const m=JSON.parse(metrics.result.value);console.log(JSON.stringify(m));if(m.scroll>m.width)throw Error('Horizontal overflow');
 const shot=await send('Page.captureScreenshot',{format:'png',captureBeyondViewport:false});fs.writeFileSync('preview-mobile.png',Buffer.from(shot.data,'base64'));ws.close();
})().catch(e=>{console.error(e.message);process.exitCode=1;});
