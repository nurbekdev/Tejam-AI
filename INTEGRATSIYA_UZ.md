# Tejam AI × Hamroh IO — hackathon demo

## Nima bajarildi?

Siz yuborgan statik `tejam-ai-site` platformasiga Hamroh IoT integratsiyasi qo‘shildi. Asl ishlayotgan moliyaviy sahifalar, mavjud `tejamBridge` va `checks` moduli saqlanadi.

- **Hamroh chat**: foydalanuvchi «Bugun ovqatga 50 ming so‘m sarfladim» yozsa, xarajat biznesning lokal bazasiga kiradi.
- **Natija**: vebdagi mini displeyda summa va ko‘zli virtual yordamchi animatsiyasi ko‘rinadi. Bu ko‘rinish **simulyatsiya**, fizik qurilma emas.
- **BLE**: Chrome/Edge HTTPS/localhost sahifada «Qurilmani ulash» tugmasidan `Tejam-Hamroh` BLE qurilmasini tanlang. Qurilma ulangandan keyin yangi xarajat, daromad va kunlik jami xabarlari BLE ga jo‘natiladi.
- **Qurilmadagi ifodalar**: xarajat – sarg‘ish/yumshoq xafa; daromad – xursand; jami – neytral; xabar kelganda tebranish va ko‘z qisish.
- **Quick entry**: eski tezkor xarajat formasi bilan kiritsangiz ham shu BLE yo‘li ishlaydi; kelgusida original `TejamPet` chati `tejamBridge.addTransaction()` chaqirsa ham xabar ketadi.

## Eng tez ko‘rsatish tartibi

1. ZIP ni oching va `tejam-ai-site` ichida `python3 -m http.server 8000` bajaring.
2. `http://localhost:8000` oching. Yangi **Tejam × Hamroh** blokidagi **Hamroh chat** tugmasini bosing.
3. «Bugun ovqatga 50 ming so‘m sarfladim» yuboring. Saytdagi yangi xarajat va brauzer simulyatsiyasida `50 000 so‘m` chiqishi kerak.
4. «Bugun qancha xarajat qildim?» yuboring. Demo bo‘lmagan bugungi xarajatlar hisoblanadi.
5. Fizik displey uchun avval Waveshare rasmiy `06_LVGL_Test` misoliga `hamroh-esp32c6/TejamHamrohBLE.h` ni ulab flash qiling. Boshqa firmware ni backup qilmasdan ustidan yozmang.
6. Sahifada **Qurilmani ulash** ni bosing, `Tejam-Hamroh` ni tanlang. **Animatsiyani sinash** tugmasi buxgalteriya bazasini o‘zgartirmay qurilmaga 50 000 so‘mlik sinov animatsiyasini yuboradi.
7. Yana xarajat kiriting. Qurilma uni muvaffaqiyatli qabul qilgan bo‘lsa, displeyda ko‘rsatiladi.

## Chat uchun ishlaydigan misollar

- `Bugun ovqatga 50 000 so'm sarfladim` → xarajat 50 000 / Oziq-ovqat
- `Bugun taksiga 25 ming so'm sarfladim` → xarajat 25 000 / Transport
- `Bugun 1.5 mln sarfladim` → xarajat 1 500 000
- `Bugun 200 ming daromad topdim` → kirim 200 000
- `Bugun qancha xarajat qildim?` → kunlik hisob va ekran yangilanishi

Bu qoidaviy (regulyar ifodalar) chat demo: **haqiqiy LLM/Telegram bot integratsiyasi emas**. Original `tejam-pet.js` siz yuborgan arxivda bo‘lmagan. Sahifada aynan chatdan xarajat kiritishning ishlaydigan namoyishi bo‘lishi uchun oddiy chat kiritildi.

## Cheklovlar

- **ESP32-C6 displey kodingiz yo‘q**: ushbu ZIP firmware-ni to‘liq qayta yaratmaydi, rasmiy Waveshare display init/lvgl misoliga qo‘shiluvchi `.h` modul beradi. Joylashuvi `hamroh-esp32c6/README_UZ.md`da tushuntirilgan.
- **Brauzer cheklovi**: Web Bluetooth har bir brauzerda yo‘q; Chrome/Edge va xavfsiz HTTPS (yoki localhost) bilan tekshiring. BLE yaqin masofadagi qurilma uchun, Telegram bot masofadan xabar yubormaydi.
- **Demo maʼlumot**: saytning avvalgi 60 kunlik demo tranzaksiyalari bor. Qurilmaga yuborilgan `today` faqat foydalanuvchi kiritgan real/demo bo‘lmagan bugungi xarajatlarni hisoblaydi, shuning uchun katta demodashboard bilan farq bo‘lishi mumkin.
- **Maʼlumotlar**: sayt bazasi hamon localStorage; bulut/foydalanuvchilar o‘rtasida sinxronlash yo‘q. BLE aloqa hozircha PIN/pairing bilan himoyalanmagan. Haqiqiy maxfiy moliyaviy maʼlumotlar uchun qo‘shimcha security kerak.
- **Testlar**: JavaScript mantiq va BLE xabar formatining avtomatik testlari bajarilgan. Haqiqiy ekran, BLE radio, Arduino/LVGL build ushbu muhitda sinovdan o‘tkazilmagan.

## Asosiy texnik oqim

`Chat` → `parseTejamChat()` → `tejamBridge.addTransaction()` → `localStorage tejam.v3` → `sendHamroh()` → Web Bluetooth GATT characteristic → `TejamHamrohBLE.h` → LVGL ekran / animatsiya.

BLE UUIDs va xabar formati `hamroh-esp32c6/README_UZ.md`da batafsil.

## Dasturchilar uchun tekshirish

```bash
node --test tests/hamroh.test.mjs
node --check app.js
node --check hamroh-device.js
node --check hamroh-chat.js
```

Waveshare rasmiy qo‘llanma: https://docs.waveshare.com/ESP32-C6-Touch-AMOLED-1.64/Arduino
NimBLE-Arduino: https://github.com/h2zero/NimBLE-Arduino
Web Bluetooth: https://developer.mozilla.org/en-US/docs/Web/API/Web_Bluetooth_API
