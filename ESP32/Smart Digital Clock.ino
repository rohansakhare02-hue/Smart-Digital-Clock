// ============================================================
//  TASK MANAGING CLOCK — ESP32 Firmware
//  Hardware : ESP32 + LCD 16x2 I2C (0x27) + DS3231 RTC
//           + Buzzer (GPIO23) + TM1637 4-digit 7-seg (GPIO18/19)
// ============================================================

#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <TM1637Display.h>          // ← NEW: install via Library Manager

// ── USER CONFIGURATION ───────────────────────────────────────
#define WIFI_SSID             "Rohan's device"
#define WIFI_PASSWORD         "1234566778"
#define FIREBASE_API_KEY      "AIzaSyBjIwARMVJhUSBtpk6VE0kpR7B2qo7_wxk"
#define FIREBASE_DATABASE_URL "https://smart-clock-c7f60-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_USER_EMAIL   "esp32@clock.com"
#define FIREBASE_USER_PASSWORD "clock1234"

// ── PIN DEFINITIONS ──────────────────────────────────────────
#define BUZZER_PIN   23     // GPIO23
#define TM_CLK_PIN   18     // ← NEW: TM1637 CLK
#define TM_DIO_PIN   19     // ← NEW: TM1637 DIO
// I2C shared bus: SDA → GPIO21,  SCL → GPIO22

// ── OBJECTS ──────────────────────────────────────────────────
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231        rtc;
FirebaseData      fbdo;
FirebaseAuth      auth;
FirebaseConfig    config;
TM1637Display     seg(TM_CLK_PIN, TM_DIO_PIN);  // ← NEW

// ── TASK STRUCT (unchanged) ───────────────────────────────────
struct Task {
  String id, title, description, assignedPerson;
  String date, time, priority, repeatType, status;
  bool   soundEnabled, alerted;
  int    buzzerDuration;
};

const int MAX_TASKS = 30;
Task tasks[MAX_TASKS];
int  taskCount = 0;

// ── TIMING ───────────────────────────────────────────────────
unsigned long lastFetch     = 0;
unsigned long lastClockDraw = 0;
unsigned long lastScroll    = 0;

const unsigned long FETCH_MS  = 10000;
const unsigned long CLOCK_MS  =  1000;
const unsigned long SCROLL_MS =  4000;

int  scrollIdx     = 0;
bool firebaseReady = false;
bool colonOn       = false;   // ← NEW: blink state

// ── FORWARD DECLARATIONS ─────────────────────────────────────
void connectWiFi();
void initFirebase();
void fetchTasks();
void drawClock();
void update7Seg(int hh, int mm, bool colon);  // ← NEW
void checkDeadlines();
void scrollTasks();
void buzzForTask(Task& t);
void lcdMsg(const char* l1, const char* l2, int ms = 0);
void lcdRow(int row, String text);
String pad2(int n);

// ============================================================
//  SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Task Managing Clock — Boot ===");

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // LCD
  lcd.init();
  lcd.backlight();
  lcdMsg("Task Managing", "  Clock  v2.0", 1500);

  // TM1637 — init & brightness (0–7)
  seg.setBrightness(5);
  seg.showNumberDecEx(0, 0b01000000, true);  // show "00:00" on boot

  // DS3231 — I2C on GPIO21/22
  Wire.begin(21, 22);
  if (!rtc.begin()) {
    Serial.println("[RTC] DS3231 not found!");
    lcdMsg("RTC ERROR!", "Check SDA/SCL", 3000);
    seg.showNumberDecEx(8888, 0b01000000, true);  // show "88:88" as error
  } else {
    if (rtc.lostPower()) {
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    Serial.println("[RTC] DS3231 OK");
  }

  connectWiFi();
  initFirebase();
  lcdMsg("System Ready!", "Loading tasks..", 800);
}

// ============================================================
//  MAIN LOOP
// ============================================================
void loop() {
  unsigned long now = millis();

  if (now - lastClockDraw >= CLOCK_MS) {
    lastClockDraw = now;
    drawClock();
  }

  if (firebaseReady && (now - lastFetch >= FETCH_MS)) {
    lastFetch = now;
    fetchTasks();
  }

  checkDeadlines();

  if (now - lastScroll >= SCROLL_MS) {
    lastScroll = now;
    scrollTasks();
  }
}

// ============================================================
//  WIFI & FIREBASE (unchanged from original)
// ============================================================
void connectWiFi() {
  Serial.printf("[WiFi] Connecting to: %s\n", WIFI_SSID);
  lcdMsg("Connecting WiFi", WIFI_SSID, 0);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries++ < 30) delay(500);
  if (WiFi.status() == WL_CONNECTED) {
    lcdMsg("WiFi Connected!", WiFi.localIP().toString().c_str(), 1500);
  } else {
    lcdMsg("WiFi Failed!", "Offline mode", 2000);
  }
}

void initFirebase() {
  config.api_key               = FIREBASE_API_KEY;
  config.database_url          = FIREBASE_DATABASE_URL;
  auth.user.email              = FIREBASE_USER_EMAIL;
  auth.user.password           = FIREBASE_USER_PASSWORD;
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  lcdMsg("Firebase", "Authenticating.", 0);
  unsigned long t = millis();
  while (!Firebase.ready() && millis() - t < 12000) delay(300);
  firebaseReady = Firebase.ready();
  if (firebaseReady) lcdMsg("Firebase OK!", "", 1000);
  else               lcdMsg("Firebase Error!", "Check config", 2500);
}

// ============================================================
//  FETCH TASKS (unchanged from original)
// ============================================================
void fetchTasks() {
  Serial.println("[Firebase] Syncing /tasks ...");
  if (!Firebase.RTDB.getJSON(&fbdo, "/tasks")) {
    Serial.println("[Firebase] ERROR: " + fbdo.errorReason());
    return;
  }
  FirebaseJson& root  = fbdo.jsonObject();
  size_t        total = root.iteratorBegin();

  struct Cache { String id; bool alerted; };
  Cache cache[MAX_TASKS];
  int   cacheSize = taskCount;
  for (int i = 0; i < cacheSize; i++) cache[i] = { tasks[i].id, tasks[i].alerted };
  taskCount = 0;

  for (size_t i = 0; i < total && taskCount < MAX_TASKS; i++) {
    int type = 0; String key, value;
    root.iteratorGet(i, type, key, value);
    FirebaseJson child; FirebaseJsonData f;
    child.setJsonData(value);
    Task t; t.id = key; t.alerted = false;
    for (int j = 0; j < cacheSize; j++)
      if (cache[j].id == key) { t.alerted = cache[j].alerted; break; }
    auto S = [&](const char* k, String def="")->String{ child.get(f,k); return f.success?f.stringValue:def; };
    auto B = [&](const char* k, bool def=false)->bool{ child.get(f,k); return f.success?f.boolValue:def; };
    auto I = [&](const char* k, int def=0)->int{ child.get(f,k); return f.success?f.intValue:def; };
    t.title=S("title","Untitled"); t.description=S("description");
    t.assignedPerson=S("assignedPerson"); t.date=S("date"); t.time=S("time");
    t.priority=S("priority","medium"); t.repeatType=S("repeatType","none");
    t.status=S("status","pending"); t.soundEnabled=B("soundEnabled",true);
    t.buzzerDuration=I("buzzerDuration",3);
    tasks[taskCount++] = t;
  }
  root.iteratorEnd();
  Serial.printf("[Firebase] Loaded %d task(s).\n", taskCount);
}

// ============================================================
//  DRAW CLOCK — LCD row 0 + TM1637
// ============================================================
void drawClock() {
  DateTime now = rtc.now();
  const char* dayNames[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

  // ── LCD row 0 ───────────────────────────────────────────
  String line =
    String(dayNames[now.dayOfTheWeek()]) + " " +
    pad2(now.day())   + "/" +
    pad2(now.month()) + "  " +
    pad2(now.hour())  + ":" +
    pad2(now.minute());
  lcd.setCursor(0, 0);
  lcd.print(line);

  // ── TM1637: HH:MM with blinking colon ──────────────────
  colonOn = !colonOn;
  update7Seg(now.hour(), now.minute(), colonOn);
}

// ============================================================
//  UPDATE 7-SEGMENT DISPLAY
//  Encodes HHMM as a 4-digit number; colon bit = 0b01000000
// ============================================================
void update7Seg(int hh, int mm, bool colon) {
  int timeVal = hh * 100 + mm;          // e.g. 09:35 → 935
  uint8_t colonBits = colon ? 0b01000000 : 0;
  seg.showNumberDecEx(timeVal, colonBits, true);  // true = leading zero
}

// ============================================================
//  CHECK DEADLINES (same logic, added 7-seg flash on alert)
// ============================================================
void checkDeadlines() {
  if (taskCount == 0) return;
  DateTime now     = rtc.now();
  String   nowTime = pad2(now.hour()) + ":" + pad2(now.minute());
  String   nowDate = String(now.year()) + "-" + pad2(now.month()) + "-" + pad2(now.day());

  for (int i = 0; i < taskCount; i++) {
    Task& t = tasks[i];
    if (t.status == "done" || t.status == "alerted") continue;
    if (t.alerted || !t.soundEnabled) continue;
    int currentMinutes = now.hour() * 60 + now.minute();
    int taskMinutes    = t.time.substring(0,2).toInt() * 60 + t.time.substring(3,5).toInt();
    if (abs(currentMinutes - taskMinutes) > 1) continue;
    bool dateMatch = (t.date == nowDate) || (t.repeatType == "daily");
    if (!dateMatch) continue;

    Serial.printf("[ALERT] \"%s\" due now!\n", t.title.c_str());
    String alertStr = ">" + t.title;
    if (alertStr.length() > 16) alertStr = alertStr.substring(0, 16);
    lcdRow(1, alertStr);

    buzzForTask(t);   // also flashes 7-seg inside

    t.alerted = true;
    String path = "/tasks/" + t.id + "/status";
    Firebase.RTDB.setString(&fbdo, path, "alerted");
  }
}

// ============================================================
//  BUZZER + 7-SEG FLASH during alert
// ============================================================
void buzzForTask(Task& t) {
  int durationSec = constrain(t.buzzerDuration, 1, 30);
  int onMs = 300, offMs = 200;
  if (t.priority == "high") { onMs = 150; offMs = 100; }
  if (t.priority == "low")  { onMs = 500; offMs = 400; }

  unsigned long stopAt = millis() + (unsigned long)(durationSec * 1000);
  bool flashState = false;
  while (millis() < stopAt) {
    digitalWrite(BUZZER_PIN, HIGH);

    // Flash 7-seg: alternate between time and "----" while buzzing
    flashState = !flashState;
    if (flashState) {
      seg.showNumberDecEx(
        rtc.now().hour() * 100 + rtc.now().minute(),
        0b01000000, true
      );
    } else {
      seg.clear();  // blank display for flash effect
    }

    delay(onMs);
    digitalWrite(BUZZER_PIN, LOW);
    delay(offMs);
  }

  // Restore display after buzz
  DateTime now = rtc.now();
  update7Seg(now.hour(), now.minute(), true);
}

// ============================================================
//  SCROLL TASKS (unchanged)
// ============================================================
void scrollTasks() {
  DateTime now     = rtc.now();
  String   nowDate = String(now.year()) + "-" + pad2(now.month()) + "-" + pad2(now.day());
  int eligible[MAX_TASKS], eligibleCount = 0;
  for (int i = 0; i < taskCount; i++) {
    if (tasks[i].status == "done") continue;
    if (tasks[i].date == nowDate || tasks[i].repeatType == "daily")
      eligible[eligibleCount++] = i;
  }
  if (eligibleCount == 0) { lcdRow(1, "No tasks today!"); return; }
  if (scrollIdx >= eligibleCount) scrollIdx = 0;
  Task& t = tasks[eligible[scrollIdx++]];
  char icon = (t.priority == "high") ? '!' : (t.priority == "low") ? '.' : '-';
  String title = t.title.substring(0, min((int)t.title.length(), 10));
  lcdRow(1, String(icon) + title + " " + t.time);
}

// ============================================================
//  UTILITIES (unchanged)
// ============================================================
String pad2(int n) { return (n < 10 ? "0" : "") + String(n); }
void lcdRow(int row, String text) {
  while ((int)text.length() < 16) text += " ";
  lcd.setCursor(0, row);
  lcd.print(text.substring(0, 16));
}
void lcdMsg(const char* l1, const char* l2, int ms) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(String(l1).substring(0, 16));
  lcd.setCursor(0, 1); lcd.print(String(l2).substring(0, 16));
  if (ms > 0) delay(ms);
}