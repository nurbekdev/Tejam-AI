# Tejam AI — финансы бизнеса
Статический сайт без сборки. Основные файлы: index.html, style.css, app.js, finance.js, config.js. Дополнительные CSS сохраняют исходный дизайн. Папка checks содержит сохранённый модуль 14 проверок счетов, договоров и возврата. PDF.js и QRCode уже лежат локально в assets; CDN и внешние шрифты не нужны.

## Важно: питомец не приложен
В доступных вложениях был только скриншот. tejam-pet.js в архиве — пустой файл с пояснением, НЕ реализация питомца. Замените его своим оригинальным tejam-pet.js без изменений. Страница уже вызывает TejamPet.mount({telegramUrl, bridge}), если объект существует. Bridge доступен также как window.tejamBridge. Совместимость с demoBridge исходного питомца и его команды не проверены: исходник не предоставлен. pet-demo.html, pet-reference.png и pet.svg не были доступны. Если оригинальный питомец использует отдельные изображения, добавьте их по путям из его файла.

## Запуск
Из распакованной папки:
python3 -m http.server 8000
Открыть http://localhost:8000 . Не открывайте index.html через file://: используются ES-модули.

## Публикация Vercel
cd tejam-ai-site
npm i -g vercel
vercel login
vercel --prod
Set up and deploy? Y; Link to existing project? N; Project name: tejam-ai; Directory: ./; Modify settings? N.

## Что работает
Два бизнеса, 60 дней детерминированных операций на каждый (20.09–18.11.2026), доходы/расходы/прибыль/маржа, сравнение периодов и бизнесов, SVG-график и категории. Операции редактируются, добавляются и удаляются. Иерархия год → месяцы → дни → чеки. Поиск по комментарию и категории. CSV импорт/экспорт и TSV. Заметки: поиск, закрепление, правка, скачивание TXT. Решения: статусы, комментарии, шаги, источник. Legacy-журнал переносится без удаления исходника. 14 проверок сохранены. Точки, договоры, претензии доступны в модуле проверок.

## Данные
Ключ localStorage: tejam.v3. Старый tejam-v2 читается для миграции и не удаляется. invoiceChecks — сохранённая структура проверок внутри v3. Она является отдельным учебным набором и не прибавляется к финансовым операциям бизнеса. Заметки и решения общие для рабочего пространства; операции относятся к businessId. Резервная копия JSON содержит все данные v3. Импорт заменяет текущие данные только после подтверждения. Очистка демо удаляет операции с demo:true, оставляет пользовательские операции и бизнесы. Данные не синхронизируются между браузерами и доменами. Перед переходом на Vercel экспортируйте JSON на старом сайте и импортируйте на новом.

CSV: UTF-8, запятые, заголовок date,type,amount,category,comment. type: income или expense. amount — положительное число; date — YYYY-MM-DD. Кавычки, запятые и переносы в комментариях поддерживаются.

## Метрики
Доходы = сумма income; расходы = сумма expense; прибыль = доходы − расходы; маржа = прибыль / доходы × 100 (при нулевых доходах показано 0%). Изменение: текущий − предыдущий; процент = разница / abs(предыдущий) × 100; нулевая/отсутствующая база обозначена «нет базы». Маржа дополнительно сравнивается в процентных пунктах. Месяц сравнивается с полным предыдущим календарным месяцем; демоданные неполные и не являются результатами реального бизнеса. «Всё время» не имеет предыдущего периода.

## Ограничения и честная демонстрация
Нет серверной модели, серверного хранилища, автоматических платежей или реальной интеграции Telegram с данными. Ссылка на бот находится в config.js TELEGRAM_BOT_URL. Физический питомец и WebSocket не реализованы и не проверены без оригинального файла и устройства. Извлечение условий договоров и ответы старого помощника были шаблонной симуляцией; старый плавающий помощник отключён. Сигнал не означает мошенничество. Демо-проверки и претензии требуют решения человека.

Проверены: 14 правил, допсоглашения, возвраты, финансовые расчёты, изоляция бизнесов, CSV; в браузере ввод расхода 500000 с пересчётом KPI, раскрытие дня, добавление заметки и решения, загрузка прежних проверок. Адаптивная CSS-вёрстка добавлена; отдельная проверка на физическом телефоне не проводилась. Команды питомца не проверены из-за отсутствия файла.

## TEJAM × HAMROH IO — hackathon BLE integration (2026-11-18)

**Qo'shilgan fayllar:** `hamroh-device.js` (Web Bluetooth), `hamroh-chat.js` (Uzbek rule-based parser), `hamroh.css`, `hamroh-esp32c6/TejamHamrohBLE.h` va `hamroh-esp32c6/README_UZ.md`, `tests/hamroh.test.mjs`. Batafsil o‘zbekcha yo‘riqnoma: `INTEGRATSIYA_UZ.md`.

Saytning yuqorisida yangi Tejam × Hamroh moduli mavjud. Chrome/Edge brauzeridagi «Qurilmani ulash» real BLE qurilmaga ulaydi; mini displey faqat *brauzer simulyatsiyasi* ekanligi belgilangan. Chat misolida «Bugun ovqatga 50 ming so‘m sarfladim» xarajatni `tejam.v3` ga qo‘shadi va **ulangandan keyin** BLE yuborishni boshlaydi. Summani faqat tanlangan biznesning demo bo‘lmagan, bugun kiritilgan xarajatlari bo‘yicha hisoblaydi. Rasmiy virtual pet manba kodi berilmagani uchun yangi qoidaviy demo chat qo‘shildi; original `tejam-pet.js` berilsa uning mavjud `bridge.addTransaction(...)` integratsiya nuqtasi ham BLE ni ishga tushiradi.

Boshlash: `python3 -m http.server 8000` va `http://localhost:8000/`, yoki Vercel HTTPS. Test: `node --test tests/hamroh.test.mjs`.

**Muhim:** jismoniy ESP32-C6 firmware/fizik BLE testini ushbu kodni tayyorlash muhitida bajarib bo‘lmaydi. Firmware header Waveshare'ning rasmiy `06_LVGL_Test` misoliga qo‘shiladi; bu zipda original display BSP yo‘q. Brauzer testi ham saytni haqiqiy qurilmada ishlatish bilan teng emas.
