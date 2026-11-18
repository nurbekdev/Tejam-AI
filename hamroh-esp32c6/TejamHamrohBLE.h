#pragma once
/*
 * Tejam AI × Hamroh IO | BLE + LVGL overlay for Waveshare
 * ESP32-C6-Touch-AMOLED-1.64 (CO5300, 280x456).
 *
 * ADD THIS HEADER TO WAVESHARE'S OFFICIAL Arduino/examples/06_LVGL_Test PROJECT.
 * The official example must initialize the CO5300 panel and LVGL first.
 *
 * Required: ESP32 board package >= 3.2.0, LVGL 8.4.0 (Waveshare bundle),
 * NimBLE-Arduino 2.5.2 (pinned), ArduinoJson 7.x.
 *
 * Setup after LCD+LVGL initialization (while LVGL is locked, if a task uses a lock):
 *     tejamHamrohBegin();
 * No loop modifications required: LVGL timer processes BLE events and animation.
 */
#include <Arduino.h>
#include <lvgl.h>
#include <NimBLEDevice.h>
#include <ArduinoJson.h>
#include <math.h>
#include <string>
#include <stdio.h>
#include <inttypes.h>

static constexpr char TEJAM_SERVICE_UUID[] = "856f4f00-22da-47dd-b7bb-f2e3d6e3a0aa";
static constexpr char TEJAM_RX_UUID[] = "856f4f01-22da-47dd-b7bb-f2e3d6e3a0aa";

namespace TejamHamroh {
struct Event {
  char kind{'h'};
  uint64_t amount{0};
  uint64_t daily{0};
  char category[25]{};
};
static Event pending{};
static bool hasPending = false;
static portMUX_TYPE pendingLock = portMUX_INITIALIZER_UNLOCKED;
static std::string lineBuffer;

static lv_obj_t* page = nullptr;
static lv_obj_t* face = nullptr;
static lv_obj_t* leftEye = nullptr;
static lv_obj_t* rightEye = nullptr;
static lv_obj_t* mouth = nullptr;
static lv_obj_t* eventLabel = nullptr;
static lv_obj_t* amountLabel = nullptr;
static lv_obj_t* dailyLabel = nullptr;
static lv_obj_t* categoryLabel = nullptr;
static lv_obj_t* btLabel = nullptr;
static uint32_t eventStarted = 0;
static uint32_t blinkAt = 0;
static uint32_t blinkMs = 0;
static char currentKind = 'h';
static NimBLEServer* bleServer = nullptr;

static void groupedAmount(uint64_t amount, char* out, size_t cap) {
  char raw[32];
  snprintf(raw, sizeof(raw), "%llu", (unsigned long long)amount);
  const size_t n = strlen(raw);
  size_t pos = 0;
  for (size_t i=0; i<n && pos+1<cap; i++) {
    out[pos++] = raw[i];
    const size_t remaining = n-i-1;
    if (remaining > 0 && remaining%3 == 0 && pos+1<cap) out[pos++] = ' ';
  }
  if (pos<cap) out[pos]='\0';
}

static void parseLine(const std::string& line) {
  JsonDocument doc;
  if (deserializeJson(doc, line.c_str(), line.size())) return;
  if ((doc["v"] | 0) != 1) return;
  const char* kind = doc["k"] | "";
  if (strlen(kind) != 1 || !strchr("eidh", kind[0])) return;
  if (!doc["a"].is<uint64_t>() || !doc["d"].is<uint64_t>()) return;
  const uint64_t a = doc["a"].as<uint64_t>();
  const uint64_t d = doc["d"].as<uint64_t>();
  if (a > 1000000000000ULL || d > 1000000000000ULL) return;
  Event e;
  e.kind=kind[0]; e.amount=a; e.daily=d;
  const char* cat = doc["c"] | "";
  for (size_t i=0;i<sizeof(e.category)-1 && cat[i];i++) {
    e.category[i] = (cat[i]>=32 && cat[i]<=126) ? cat[i] : '?';
  }
  portENTER_CRITICAL(&pendingLock);
  pending=e;
  hasPending=true;
  portEXIT_CRITICAL(&pendingLock);
}

class RXCallbacks final : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* characteristic, NimBLEConnInfo& connection) override {
    // Web Bluetooth sends <=18-byte writes, new line ends one JSON command.
    const std::string bytes = characteristic->getValue();
    for (char c: bytes) {
      if (c == '\n') {if (!lineBuffer.empty()) parseLine(lineBuffer);lineBuffer.clear();}
      else if (lineBuffer.length() < 210) lineBuffer.push_back(c);
      else lineBuffer.clear();
    }
  }
};

class ServerCallbacks final : public NimBLEServerCallbacks {
  void onDisconnect(NimBLEServer* srv, NimBLEConnInfo& connection, int reason) override {
    NimBLEDevice::startAdvertising();
  }
};
static RXCallbacks rxCallbacks;
static ServerCallbacks serverCallbacks;

static lv_obj_t* createText(lv_obj_t* parent, const char* str, lv_color_t color, int y) {
  auto* label = lv_label_create(parent);
  lv_label_set_text(label,str);
  lv_obj_set_width(label,260);
  lv_label_set_long_mode(label,LV_LABEL_LONG_DOT);
  lv_obj_set_style_text_color(label,color,0);
  lv_obj_set_style_text_align(label,LV_TEXT_ALIGN_CENTER,0);
  lv_obj_align(label,LV_ALIGN_TOP_MID,0,y);
  return label;
}

static lv_obj_t* createEye(lv_obj_t* parent, int x) {
  lv_obj_t* obj=lv_obj_create(parent);
  lv_obj_set_size(obj,22,30);
  lv_obj_set_pos(obj,x,45);
  lv_obj_set_style_radius(obj,LV_RADIUS_CIRCLE,0);
  lv_obj_set_style_border_width(obj,0,0);
  lv_obj_set_style_bg_color(obj,lv_color_hex(0x80EFC9),0);
  lv_obj_set_style_shadow_color(obj,lv_color_hex(0x30B8A1),0);
  lv_obj_set_style_shadow_opa(obj,LV_OPA_50,0);
  lv_obj_set_style_shadow_width(obj,15,0);
  lv_obj_clear_flag(obj,LV_OBJ_FLAG_SCROLLABLE);
  return obj;
}
static const lv_point_t smile[]={{0,0},{8,7},{16,10},{24,7},{32,0}};
static const lv_point_t frown[]={{0,10},{8,3},{16,0},{24,3},{32,10}};
static const lv_point_t straight[]={{0,4},{32,4}};

static void applyMood(char kind) {
  lv_color_t color=lv_color_hex(0x80EFC9);
  if(kind=='e') color=lv_color_hex(0xFFC481);
  if(kind=='d') color=lv_color_hex(0x92C4FF);
  lv_obj_set_style_bg_color(leftEye,color,0);
  lv_obj_set_style_bg_color(rightEye,color,0);
  lv_obj_set_style_line_color(mouth,color,0);
  if(kind=='e') lv_line_set_points(mouth,frown,5);
  else if(kind=='d') lv_line_set_points(mouth,straight,2);
  else lv_line_set_points(mouth,smile,5);
}

static void displayEvent(const Event& e) {
  currentKind=e.kind;
  eventStarted=millis();
  char a[40],d[40],buffer[90];
  groupedAmount(e.amount,a,sizeof(a));
  groupedAmount(e.daily,d,sizeof(d));
  if(e.kind=='e') {
    lv_label_set_text(eventLabel,"YANGI XARAJAT");
    snprintf(buffer,sizeof(buffer),"- %s som",a);
    lv_label_set_text(amountLabel,buffer);
  } else if (e.kind=='i') {
    lv_label_set_text(eventLabel,"YANGI DAROMAD");
    snprintf(buffer,sizeof(buffer),"+ %s som",a);
    lv_label_set_text(amountLabel,buffer);
  } else if (e.kind=='d') {
    lv_label_set_text(eventLabel,"BUGUNGI XARAJAT");
    snprintf(buffer,sizeof(buffer),"%s som",a);
    lv_label_set_text(amountLabel,buffer);
  } else {
    lv_label_set_text(eventLabel,"SALOM!");
    lv_label_set_text(amountLabel,"TEJAM AI");
  }
  snprintf(buffer,sizeof(buffer),"Bugun: %s som",d);
  lv_label_set_text(dailyLabel,buffer);
  lv_label_set_text(categoryLabel,e.category);
  applyMood(e.kind);
}

static void timerCallback(lv_timer_t*) {
  Event e;
  bool newEvent=false;
  portENTER_CRITICAL(&pendingLock);
  if(hasPending){e=pending;hasPending=false;newEvent=true;}
  portEXIT_CRITICAL(&pendingLock);
  if(newEvent) displayEvent(e);
  const uint32_t now=millis();
  // Subtle buoyant animation for 1.2 seconds after every expense or chat command.
  const uint32_t dt=now-eventStarted;
  const int y=(dt<1200) ? int(-8.0f*sinf(dt*0.013f)*expf(-dt/800.0f)) : 0;
  lv_obj_align(face,LV_ALIGN_TOP_MID,0,100+y);
  // Periodic blinking that leaves the BLE handler free of LVGL calls.
  if(now-blinkAt>3900) {blinkAt=now; blinkMs=now;}
  const bool blinking=(now-blinkMs)<125;
  lv_obj_set_height(leftEye,blinking?4:30);
  lv_obj_set_height(rightEye,blinking?4:30);
  if(btLabel)lv_label_set_text(btLabel,bleServer && bleServer->getConnectedCount()>0 ? "BLE CONNECTED" : "BLE READY");
}

static void buildUI() {
  page=lv_scr_act();
  lv_obj_clean(page);  // Replaces Waveshare's test widgets; retains its LCD driver/timing.
  lv_obj_set_style_bg_color(page,lv_color_hex(0x071422),0);
  lv_obj_set_style_bg_opa(page,LV_OPA_COVER,0);
  lv_obj_clear_flag(page,LV_OBJ_FLAG_SCROLLABLE);
  createText(page,"TEJAM  x  HAMROH",lv_color_hex(0x85E3C5),26);
  createText(page,"Sizning moliyaviy yordamchingiz",lv_color_hex(0x879AAB),55);
  face=lv_obj_create(page);
  lv_obj_set_size(face,200,145);
  lv_obj_align(face,LV_ALIGN_TOP_MID,0,100);
  lv_obj_set_style_bg_opa(face,LV_OPA_TRANSP,0);
  lv_obj_set_style_border_width(face,0,0);
  lv_obj_set_style_pad_all(face,0,0);
  lv_obj_clear_flag(face,LV_OBJ_FLAG_SCROLLABLE);
  leftEye=createEye(face,49);
  rightEye=createEye(face,128);
  mouth=lv_line_create(face);
  lv_obj_set_pos(mouth,84,99);
  lv_obj_set_style_line_width(mouth,5,0);
  lv_obj_set_style_line_rounded(mouth,true,0);
  lv_line_set_points(mouth,smile,5);
  eventLabel=createText(page,"YORDAMCHINGIZ TAYYOR",lv_color_hex(0x90B8CE),266);
  amountLabel=createText(page,"Assalomu alaykum!",lv_color_hex(0xF4FFFE),297);
#if LV_FONT_MONTSERRAT_20
  lv_obj_set_style_text_font(amountLabel,&lv_font_montserrat_20,0);
#endif
  dailyLabel=createText(page,"Bugun: 0 som",lv_color_hex(0x88E0BB),348);
  categoryLabel=createText(page,"",lv_color_hex(0x96AFC4),380);
  btLabel=createText(page,"BLE READY",lv_color_hex(0x72899C),426);
}

static void begin(){
  buildUI(); // MUST be called after LVGL and LCD display initialization.
  NimBLEDevice::init("Tejam-Hamroh");
  NimBLEDevice::setMTU(185);
  bleServer=NimBLEDevice::createServer();
  bleServer->setCallbacks(&serverCallbacks);
  NimBLEService* service=bleServer->createService(TEJAM_SERVICE_UUID);
  NimBLECharacteristic* rx=service->createCharacteristic(TEJAM_RX_UUID,NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  rx->setCallbacks(&rxCallbacks);
  service->start();
  NimBLEAdvertising* advertising=NimBLEDevice::getAdvertising();
  advertising->addServiceUUID(TEJAM_SERVICE_UUID);
  advertising->setName("Tejam-Hamroh");
  advertising->enableScanResponse(true);
  NimBLEDevice::startAdvertising();
  lv_timer_create(timerCallback,35,nullptr);
}
} // namespace TejamHamroh

inline void tejamHamrohBegin(){TejamHamroh::begin();}
