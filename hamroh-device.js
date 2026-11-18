// Tejam AI → Hamroh BLE v1, built for Web Bluetooth (Chrome/Edge, HTTPS or localhost).
// Messages are JSON lines, split into 18-byte GATT writes for minimum-MTU safety.
export const HAMROH_SERVICE = '856f4f00-22da-47dd-b7bb-f2e3d6e3a0aa';
export const HAMROH_WRITE   = '856f4f01-22da-47dd-b7bb-f2e3d6e3a0aa';
const MAX_PACKET = 18;
const MAX_MESSAGE = 210;
export const makeWireMessage = ({kind, amount = 0, today = 0, category = ''}) => {
  const code = ({expense:'e',income:'i',daily:'d',hello:'h'})[kind];
  if (!code) throw new Error('Nomaʼlum Hamroh xabari');
  const a = Math.round(Number(amount)), d = Math.round(Number(today));
  if (!Number.isSafeInteger(a) || !Number.isSafeInteger(d) || a<0 || d<0) throw new Error('Notoʻgʻri summa');
  // Only ASCII names are needed on the device; LVGL default fonts omit Uzbek Cyrillic.
  const c = String(category).normalize('NFKD').replace(/[^\x20-\x7e]/g,'').slice(0,24);
  const value = JSON.stringify({v:1,k:code,a,d,c}) + '\n';
  if (new TextEncoder().encode(value).length > MAX_MESSAGE) throw new Error('Xabar juda uzun');
  return value;
};
export function splitWireMessage(value){
  const bytes = new TextEncoder().encode(value);
  return Array.from({length:Math.ceil(bytes.length / MAX_PACKET)},(_,i)=>bytes.slice(i*MAX_PACKET,(i+1)*MAX_PACKET));
}

export class HamrohDevice extends EventTarget {
  constructor(){super();this.bluetoothDevice=null;this.characteristic=null;this.queue=Promise.resolve();this.status='uzilgan';}
  get connected(){return !!(this.characteristic && this.bluetoothDevice?.gatt?.connected);}
  setStatus(s){this.status=s;this.dispatchEvent(new CustomEvent('status',{detail:{status:s,connected:this.connected}}));}
  async connect(){
    if(this.connected) return true;
    if (!window.isSecureContext || !navigator.bluetooth) throw new Error('Bluetooth uchun Chrome yoki Edge va HTTPS/localhost kerak.');
    // requestDevice must originate from a real button click.
    const device = await navigator.bluetooth.requestDevice({filters:[{namePrefix:'Tejam-Hamroh'}],optionalServices:[HAMROH_SERVICE]});
    this.bluetoothDevice=device;
    device.addEventListener('gattserverdisconnected',()=>{this.characteristic=null;this.setStatus('uzildi');});
    this.setStatus('ulanmoqda');
    try{
      const gatt = await device.gatt.connect();
      const service = await gatt.getPrimaryService(HAMROH_SERVICE);
      this.characteristic = await service.getCharacteristic(HAMROH_WRITE);
      this.setStatus('ulangan');
      return true;
    }catch(err){this.characteristic=null;this.setStatus('xatolik');throw err;}
  }
  disconnect(){this.bluetoothDevice?.gatt?.disconnect();this.characteristic=null;this.setStatus('uzilgan');}
  async send(message){
    if (!this.connected) return false;
    const char = this.characteristic;
    const packetList=splitWireMessage(makeWireMessage(message));
    // Serialize concurrent browser writes so frames cannot be interleaved.
    const task = this.queue.catch(()=>{}).then(async()=>{
      for(const packet of packetList){
        if (!this.connected || char !== this.characteristic) throw new Error('Hamroh bilan aloqa uzildi');
        if (char.writeValueWithResponse) await char.writeValueWithResponse(packet);
        else await char.writeValue(packet);
      }
      this.dispatchEvent(new CustomEvent('sent',{detail:message}));
      return true;
    });
    this.queue=task.catch(err=>{this.dispatchEvent(new CustomEvent('error',{detail:err}));return false;});
    return task;
  }
}
export const hamrohDevice = new HamrohDevice();
