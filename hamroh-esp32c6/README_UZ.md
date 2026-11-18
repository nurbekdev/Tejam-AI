# Tejam AI × Hamroh IO — ESP32-C6 1.64" AMOLED firmware

## Nima bor?
- BLE nomi: **Tejam-Hamroh**
- BLE UUID protokoli Web sahifadagi `hamroh-device.js` bilan mos.
- 18-byte paketlar, yangi qator (`\\n`) tugagach JSON qabul qilinadi.
- Xarajat (`e`): sarg‘ish ko‘zlar va biroz xafa ifoda.
- Daromad (`i`): quvnoq tabassum.
- Kunlik hisob (`d`): xotirjam ifoda.
- Ko‘z qisish va qaytuvchi yumshoq animatsiya.

## Kerakli apparat
**Waveshare ESP32-C6-Touch-AMOLED-1.64 / -M**, 280×456, CO5300 QSPI displey. Boshqa ESP32-C6 ekranlari pin/driver jihatidan farq qilishi mumkin.

## O‘rnatish: rasmiy displey drayveri bilan

1. https://docs.waveshare.com/ESP32-C6-Touch-AMOLED-1.64/Resources-And-Documents dagi rasmiy GitHub misollarini yuklab oling. `Arduino/examples/06_LVGL_Test` ni oching. **Ekranni ishga tushiruvchi rasmiy kod va kutubxonalarni saqlang.**
2. Arduino IDE: `esp32 by Espressif Systems` **3.2.0+**, target **ESP32C6 Dev Module**. Waveshare yo‘riqnomasiga binoan LVGL **v8.4.0** (`lv_conf.h` bilan) va misoldagi LCD kutubxonalarini sozlang.
3. Library Manager orqali **NimBLE-Arduino **2.5.2** (aniq shu versiya, chunki yangiroq API callbacki o‘zgarishi mumkin)** va **ArduinoJson 7.x** ni o‘rnating.
4. `TejamHamrohBLE.h` faylini `06_LVGL_Test.ino` yoniga qo‘ying; asosiy `.ino` boshiga qo‘shing:

   ```cpp
   #include "TejamHamrohBLE.h"
   ```

5. `.ino` ning `setup()` funksiyasida **rasmiy LCD va LVGL init tugagach**, standart demo vidjetlarini ko‘rsatuvchi `lv_demo_widgets()` kabi chaqiruvni olib tashlang/izohga o‘tkazing va aynan shu joyga qo‘shing:

   ```cpp
`();
   ```

   Agar rasmiy misol ekran/LVGL uchun FreeRTOS lock ishlatsa, bu chaqiruv ham LVGL lock ichida bajarilishi kerak. **`lv_init()`, LCD BSP init, flush callback va `lv_timer_handler()` ni o‘chirmang.** Ular ekranni boshqarish uchun zarur.

6. Flash qiling. Ekranda `TEJAM x HAMROH`, ko‘zli virtual yordamchi va `BLE READY` ko‘rinishi kerak. Chrome/Edge (HTTPS yoki localhost) orqali Tejam saytidagi «Qurilmani ulash» tugmasidan `Tejam-Hamroh` ni tanlang.
7. «Animatsiyani sinash» tugmasi **50000 so‘mlik test ekranini** yuboradi; moliyaviy operatsiyaga yozilmaydi. Chat: `Bugun ovqatga 50 ming so'm sarfladim`. Summa saqlanadi va displeyga yuboriladi.

> Bu fayl Waveshare’ning **rasmiy display-init demosiga qo‘shiladigan overlay**. Chunki taqdim etilgan ZIP ichida ESP32-C6 firmware yoki drayver yo‘q. `TejamHamrohBLE.h` o‘z-o‘zicha mustaqil `.ino` emas. Haqiqiy qurilmada kompilyatsiya va BLE aloqa alohida tekshirilishi kerak. Eskirgan Hamroh firmware ustiga yozishdan oldin backup oling.

## Wire protokoli

UTF-8 JSON + `\\n` (<= 210 byte). Sahifa yozuvlarni 18 byte bo‘laklarda yuboradi.

```json
{"v":1,"k":"e","a":50000,"d":125000,"c":"Oziq-ovqat"}
```

`k`: `e` xarajat, `i` daromad, `d` kunlik xarajat, `h` salom. `a`: joriy summa UZS; `d`: kun davomida **foydalanuvchi kiritgan, demo bo‘lmagan** xarajatlar UZS. `c`: kategoriya, ASCII 24 belgigacha. Qurilmaga transaction ID, shaxsiy ism yoki to‘liq chat yuborilmaydi. BLE hozircha autentifikatsiyasiz; moliyaviy maʼlumotlarning ishlab chiqarish rejimi uchun pairing/bonding va foydalanuvchi ruxsatini kuchaytiring.

## Eslatma

Saytdagi Web Bluetooth faqat ayrim brauzerlarda (masalan, Chrome/Edge desktop/Android) va xavfsiz HTTPS/localhost muhitda ishlaydi. Safari/iOS bilan ishlamasligi mumkin. Server yoki mahalliy Wi-Fi shart emas, biroq Chrome yonida qurilma BLE masofasida bo‘lishi kerak. Hozirgi integratsiya sayt va qurilma bir brauzer ichida faol bo‘lgan payt uchungina; Telegram bot orqali masofaviy yuborish uchun keyinchalik backend/MQTT kerak bo‘ladi.
