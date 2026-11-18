import test from 'node:test';
import assert from 'node:assert/strict';
import {parseTejamChat,findMoney} from '../hamroh-chat.js';
import {makeWireMessage,splitWireMessage,HamrohDevice} from '../hamroh-device.js';

test('Uzbek expense, no AI assumptions',()=>{
 assert.deepEqual(parseTejamChat('Bugun ovqatga 50 000 so‘m sarfladim').kind,'expense');
 assert.equal(parseTejamChat('Bugun ovqatga 50 000 so‘m sarfladim').amount,50_000);
 assert.equal(parseTejamChat('Bugun taksiga 25 ming so‘m sarfladim').category,'Transport');
 assert.equal(parseTejamChat('Bugun 1.5 mln sarfladim').amount,1_500_000);
 assert.equal(parseTejamChat('Bugun 50.000 xarajat qildim').amount,50_000);
 assert.equal(parseTejamChat('Bugun 50 ming to‘ladim').amount,50_000);
});
test('daily report, income, and invalid claims',()=>{
 assert.equal(parseTejamChat('Bugun qancha xarajat qildim?').kind,'daily');
 assert.equal(parseTejamChat('Bugun 250000 daromad topdim').kind,'income');
 assert.equal(parseTejamChat('Bugun 50 ming sarflamadim').kind,'invalid');
 assert.equal(parseTejamChat('Bugun 5000 va 6000 sarfladim').kind,'invalid');
 assert.equal(parseTejamChat('Bugun ovqatga sarfladim').kind,'invalid');
 assert.equal(parseTejamChat('').kind,'empty');
});
test('BLE payload roundtrips 18-byte minimum MTU chunks',()=>{
 const msg={kind:'expense',amount:50_000,today:175_000,category:'Oziq-ovqat'};
 const packets=splitWireMessage(makeWireMessage(msg));
 assert.ok(packets.length>=2 && packets.every(p=>p.byteLength<=18));
 const parsed=JSON.parse(new TextDecoder().decode(Buffer.concat(packets.map(p=>Buffer.from(p)))));
 assert.deepEqual(parsed,{v:1,k:'e',a:50_000,d:175_000,c:'Oziq-ovqat'});
});
test('BLE wire rejects unknown type and negative number',()=>{
 assert.throws(()=>makeWireMessage({kind:'transfer',amount:10}));
 assert.throws(()=>makeWireMessage({kind:'expense',amount:-2}));
});
test('BLE queue prevents interleaved commands',async()=>{
 const dev=new HamrohDevice();
 let data=[];let inflight=0,maxInflight=0;
 const fake={gatt:{connected:true}};
 dev.bluetoothDevice=fake;
 dev.characteristic={writeValueWithResponse:async bytes=>{inflight++;maxInflight=Math.max(inflight,maxInflight);await new Promise(r=>setTimeout(r,2));data.push(Buffer.from(bytes));inflight--;}};
 await Promise.all([dev.send({kind:'expense',amount:100,today:100,category:'Test'}),dev.send({kind:'daily',amount:100,today:100})]);
 assert.equal(maxInflight,1);
 const combined=Buffer.concat(data).toString();
 const lines=combined.trim().split('\n');
 assert.equal(lines.length,2);
 assert.deepEqual(lines.map(s=>JSON.parse(s).k),['e','d']);
});
