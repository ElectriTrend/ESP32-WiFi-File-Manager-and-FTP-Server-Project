/*******************************************************
 *  Project   : ESP32 দিয়ে বানিয়ে ফেলুন FTP Server | OLED ডিসপ্লে ও SD কার্ড ফাইল ম্যানেজার সার্ভার |
 *  Author    : Electri Trend
 *  Date      : 19 October 2025
 *  
 *  Description:
 *  ---------------------------------
 *  - ESP32 দিয়ে SD Card Module কানেক্ট করা
 *  - OLED Display (SSD1306 I2C) তে রিয়েল-টাইম আপলোড প্রগ্রেস দেখানো
 *  - HTML + CSS + JavaScript দিয়ে লোকাল Web UI তৈরি করা
 *  - File Upload, Delete, এবং Auto Refresh সিস্টেম বানানো
 *  - SD Card থেকে যেকোনো ফাইল (txt, jpg, mp4, zip ইত্যাদি) ট্রান্সফার করা
 *  - ESP32-কে Local Wi-Fi Network এর মাধ্যমে সার্ভার হিসেবে চালানো
 *  - IP Address Access করে ওয়েব সার্ভারে লগইন করা
 *  ---------------------------------
 *  Copyright © 2025 Electri Trend
 *  All rights reserved.
 *
 *  ⚡ For tutorials, visit:
 *     👉 YouTube:    https://www.youtube.com/@Electritrendbd
 *     👉 Facebook:   https://www.facebook.com/electritrendbd
 *     👉 Instagram:  https://www.instagram.com/electri.trend
 *     👉 Github:     https://github.com/ElectriTrend
 *     👉 Website:    https://sites.google.com/view/electritrend/home
 *******************************************************/
// ========== [ Required Libraries ] ==========
#include <WiFi.h>               // For WiFi connectivity
#include <WebServer.h>          // For hosting web server
#include <SD.h>                 // For SD card file handling
#include <SPI.h>                // For SPI communication with SD
#include <Adafruit_GFX.h>       // Graphics library for OLED
#include <Adafruit_SSD1306.h>   // OLED display driver
#include <Wire.h>               // I2C communication (used by OLED)

/* 
 এই অংশে ESP32-এর WiFi, SD card, এবং OLED ডিসপ্লে কাজের জন্য 
 প্রয়োজনীয় সব লাইব্রেরি ইনক্লুড করা হয়েছে।
*/

// ========== [ SD Card Configuration ] ==========
#define SD_CS 5                 // SD card Chip Select (CS) pin
/*
 SD কার্ড মডিউল ESP32-এর GPIO5 পিনে কানেক্ট করা আছে।
*/


// ========== [ OLED Configuration ] ==========
#define SCREEN_WIDTH 128        // OLED display width (in pixels)
#define SCREEN_HEIGHT 64        // OLED display height (in pixels)
#define OLED_RESET -1           // No reset pin (using default)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
/* 
 OLED ডিসপ্লে 128x64 রেজোলিউশনের, এখানে Adafruit লাইব্রেরি দিয়ে 
 ডিসপ্লে অবজেক্ট তৈরি করা হয়েছে।
*/

// ========== [ WiFi Configuration ] ========== 
const char* ssid = "not_allow";                 // Your WiFi name (STA mode)
const char* password = "#araf#arafin#alif";     // Your WiFi password

// Access Point fallback credentials
const char* apSSID = "ESP32_FileHub";           // AP name if no WiFi available
const char* apPass = "12345678";                // AP password
/*
 যদি ESP32 তোমার WiFi-তে কানেক্ট হতে না পারে, তাহলে এটি 
 স্বয়ংক্রিয়ভাবে "ESP32_FileHub" নামে একটি WiFi Access Point খুলে দেবে।
*/


// ====== Hardware pins ======
#define LED_PIN 2        // onboard LED (change if needed)
#define BUZZER_PIN 15    // buzzer pin (use transistor if required)
/*
 ESP32-এর বিল্ট-ইন LED পিন হলো 2, এবং buzzer 15 নম্বর GPIO তে যুক্ত আছে।
*/

// ========== [ Web Server Initialization ] ==========
WebServer server(80);   // Create web server object on port 80
File uploadFile;        // For handling uploaded files.


// ========== [ Timing Control Variables ] ==========
unsigned long lastUpdate = 0;           // For periodic tasks like SD check

// ========== [ Upload Progress Variables ] ==========
volatile unsigned long uploadProgress = 0; // bytes written so far
volatile unsigned long uploadTotal = 0;    // total bytes (if known)
volatile bool uploading = false;           // Is uploading in progress?
String currentUploadFile = "";             // Name of current upload file

// ========== [ OLED Display Progress Control ] ==========
float oledDisplayPercent = 0.0;     // current shown percent (0..100)
float oledTargetPercent = 0.0;      // target percent updated during upload
unsigned long lastSmoothMillis = 0; // For smooth animation timing

// SD detect blink
bool sdPresent = false;
unsigned long sdBlinkMillis = 0;
bool sdBlinkState = false; // for blinking when SD absent

// ----------------- Helper functions -----------------
void showMessage(String line1, String line2 = "", String line3 = "") {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 6);
  display.println(line1);
  if (line2 != "") { display.setCursor(0, 22); display.println(line2); }
  if (line3 != "") { display.setCursor(0, 38); display.println(line3); }
  display.display();
}

uint64_t computeSDUsedBytes() {
  uint64_t total = 0;
  File root = SD.open("/");
  if (!root) return 0;
  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) total += file.size();
    file = root.openNextFile();
  }
  root.close();
  return total;
}

// Safe SD present check: try to open root dir (non-destructive)
bool checkSDPresent() {
  File root = SD.open("/");
  if (!root) return false;
  root.close();
  return true;
}

// Draw upload progress bar (Bar + Percentage)
void updateOLEDProgressBar() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // top line: uploading filename (truncate if long)
  display.setCursor(0, 0);
  display.print("Upload: ");
  if (currentUploadFile.length() > 18) {
    String s = currentUploadFile.substring(0, 15) + "...";
    display.println(s);
  } else {
    display.println(currentUploadFile);
  }

  // progress percent text
  display.setCursor(0, 14);
  display.print("Progress: ");
  display.print((int)oledDisplayPercent);
  display.println("%");

  // progress bar
  int barX = 10, barY = 34, barW = 108, barH = 12;
  display.drawRect(barX, barY, barW, barH, SSD1306_WHITE);
  int filled = (int)((barW * oledDisplayPercent) / 100.0);
  if (filled > 0) display.fillRect(barX, barY, filled, barH, SSD1306_WHITE);

  // bottom: will be updated by updateDisplayStatus when not uploading
  display.display();
}

// Update general OLED status (when not uploading) including animated SD icon
void updateDisplayStatus() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // WiFi Info
  display.setCursor(0, 0);
  display.print("WiFi: ");
  display.println(WiFi.SSID());

  display.setCursor(0, 10);
  display.print("IP: ");
  display.println(WiFi.localIP());

  // Signal Strength
  display.setCursor(0, 20);
  display.print("RSSI: ");
  display.print(WiFi.RSSI());
  display.println(" dBm");

  // SD status and usage
  uint64_t usedBytes = computeSDUsedBytes();
  uint64_t usedMB = usedBytes / (1024ULL * 1024ULL);
  uint64_t cardSizeMB = 0; // many SD libs don't provide this portably

  display.setCursor(0, 30);
  if (sdPresent) {
    if (cardSizeMB > 0) {
      display.print("SD: ");
      display.print(usedMB);
      display.print("/");
      display.print(cardSizeMB);
      display.println(" MB");
    } else {
      display.print("SD used: ");
      display.print(usedMB);
      display.println(" MB");
    }
  } else {
    display.print("SD: ");
    if (sdBlinkState) display.println("No Card!");
    else display.println("Insert SD");
  }

  // System Uptime
  unsigned long uptime = millis() / 1000;
  display.setCursor(0, 46);
  display.print("Uptime: ");
  display.print(uptime / 60);
  display.println(" min");

  display.display();
}

String buildHTML() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP32 File Manager</title>
<style>
:root {
  --primary: #0a84ff;
  --bg: #0b0b0f;
  --card: rgba(255,255,255,0.05);
  --text: #eaeaea;
  --accent: #0f0;
}
body {
  font-family: 'Segoe UI', Arial, sans-serif;
  background: var(--bg);
  color: var(--text);
  margin: 0;
  display: flex;
  flex-direction: column;
  align-items: center;
  min-height: 100vh;
}
h2 {
  background: linear-gradient(90deg, #111, #1e1e1e);
  width: 100%;
  text-align: center;
  padding: 12px;
  font-size: 20px;
  color: var(--primary);
  box-shadow: 0 2px 6px rgba(0,0,0,0.5);
}
.card {
  background: var(--card);
  backdrop-filter: blur(6px);
  padding: 20px;
  border-radius: 14px;
  box-shadow: 0 0 10px rgba(0,0,0,0.4);
  width: 90%;
  max-width: 700px;
  margin: 20px auto;
}
button {
  background: var(--primary);
  border: none;
  padding: 8px 14px;
  color: white;
  border-radius: 8px;
  cursor: pointer;
  font-weight: 500;
  transition: 0.3s;
}
button:hover { background: #006ae0; transform: scale(1.03); }
input[type=file] {
  color: white;
  margin: 10px;
}
table {
  width: 100%;
  border-collapse: collapse;
  text-align: left;
  margin-top: 10px;
}
th, td {
  padding: 10px;
  border-bottom: 1px solid #333;
}
a { color: var(--primary); text-decoration: none; }
.progress {
  width: 100%;
  height: 16px;
  background: #222;
  border-radius: 10px;
  overflow: hidden;
  margin-top: 8px;
}
.bar {
  height: 100%;
  width: 0%;
  background: linear-gradient(90deg, var(--primary), #00ff9d);
  transition: width 0.2s linear;
}
#statusBox {
  margin-top: 10px;
  font-size: 13px;
  color: #ccc;
}
footer {
  font-size: 12px;
  color: #666;
  margin: 20px 0;
}
</style>
</head>
<body>
<h2>📁 ESP32 SD File Manager</h2>

<div class="card">
  <h3 style="margin-top:0;">⬆️ Upload New File</h3>
  <form id="uploadForm" method="POST" action="/upload" enctype="multipart/form-data">
    <input type="file" name="data" id="fileInput" required>
    <button type="submit">Upload</button>
  </form>
  <div class="progress"><div class="bar" id="bar"></div></div>
</div>

<div class="card" id="statusCard">
  <h3>📡 System Status</h3>
  <div id="statusBox">Connecting...</div>
</div>

<div class="card">
  <h3>📂 File List</h3>
  <div id="filelist">Loading...</div>
</div>

<footer>ESP32 File Manager © Electritrend</footer>

<script>
function loadFiles(){
  fetch('/list').then(r=>r.json()).then(data=>{
    let html='<table><tr><th>File Name</th><th>Size (KB)</th><th>Action</th></tr>';
    data.forEach(f=>{
      html+=`<tr>
        <td><a href="/download?name=${encodeURIComponent(f.name)}">${f.name}</a></td>
        <td>${(f.size/1024).toFixed(2)}</td>
        <td><button onclick="del('${encodeURIComponent(f.name)}')">🗑 Delete</button></td>
      </tr>`;
    });
    html+='</table>';
    document.getElementById('filelist').innerHTML=html;
  });
}
function del(name){
  fetch('/delete?name='+decodeURIComponent(name)).then(()=>loadFiles());
}
loadFiles();
setInterval(loadFiles,5000);

function updateStatus(){
  fetch('/status').then(r=>r.json()).then(s=>{
    let str = `🌐 IP: ${s.ip} | 📶 RSSI: ${s.rssi} dBm | ⏱️ ${s.uptime_min} min`;
    if (s.sd_total_mb && s.sd_total_mb > 0) str += ` | 💾 SD: ${s.sd_used_mb}/${s.sd_total_mb} MB`;
    else if (s.sd_present) str += ` | 💾 SD: present (${s.sd_used_mb} MB)`;
    else str += ` | ⚠️ SD: not present`;
    document.getElementById('statusBox').innerText = str;
  }).catch(e=>{
    document.getElementById('statusBox').innerText = '⚠️ Status unavailable';
  });
}
updateStatus();
setInterval(updateStatus,3000);

document.getElementById('uploadForm').onsubmit=function(e){
  e.preventDefault();
  var file=document.getElementById('fileInput').files[0];
  if(!file) return;
  var xhr=new XMLHttpRequest();
  xhr.open("POST","/upload",true);
  xhr.upload.onprogress=function(e){
    if(e.lengthComputable){
      var percent=(e.loaded/e.total)*100;
      document.getElementById('bar').style.width=percent+'%';
    }
  };
  xhr.onload=function(){
    document.getElementById('bar').style.width='0%';
    loadFiles();
  };
  var formData=new FormData();
  formData.append("data",file);
  xhr.send(formData);
};
</script>
</body>
</html>
)rawliteral";
  return html;
}

void handleList() {
  File root = SD.open("/");
  File file = root.openNextFile();
  String json = "[";
  bool first = true;
  while (file) {
    if (!first) json += ",";
    json += "{\"name\":\"" + String(file.name()) + "\",\"size\":" + String(file.size()) + "}";
    first = false;
    file = root.openNextFile();
  }
  json += "]";
  server.send(200, "application/json", json);
}

void handleDelete() {
  if (!server.hasArg("name")) return server.send(400, "text/plain", "Missing file name");
  String filename = "/" + server.arg("name");
  if (SD.exists(filename)) {
    SD.remove(filename);
    showMessage("File Deleted", server.arg("name"));
    server.send(200, "text/plain", "Deleted");
  } else {
    server.send(404, "text/plain", "Not found");
  }
}

void handleDownload() {
  if (!server.hasArg("name")) return server.send(400, "text/plain", "Missing file name");
  String filename = "/" + server.arg("name");
  File file = SD.open(filename);
  if (!file) return server.send(404, "text/plain", "File not found");
  server.streamFile(file, "application/octet-stream");
  file.close();
}

void handleFree() {
  uint64_t usedBytes = computeSDUsedBytes();
  uint64_t usedMB = usedBytes / (1024ULL * 1024ULL);
  bool present = sdPresent;
  uint64_t cardSizeMB = 0;
  String json = "{";
  json += "\"sd_present\":" + String(present ? "true" : "false") + ",";
  json += "\"sd_used_mb\":" + String(usedMB) + ",";
  json += "\"sd_total_mb\":" + String(cardSizeMB);
  json += "}";
  server.send(200, "application/json", json);
}

void handleStatus() {
  uint64_t usedBytes = computeSDUsedBytes();
  uint64_t usedMB = usedBytes / (1024ULL * 1024ULL);
  uint64_t cardSizeMB = 0;
  unsigned long uptimeMin = (millis() / 1000UL) / 60UL;
  String json = "{";
  json += "\"ssid\":\"" + String(WiFi.SSID()) + "\",";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
  json += "\"uptime_min\":" + String(uptimeMin) + ",";
  json += "\"sd_present\":" + String(sdPresent ? "true" : "false") + ",";
  json += "\"sd_used_mb\":" + String(usedMB) + ",";
  json += "\"sd_total_mb\":" + String(cardSizeMB);
  json += "}";
  server.send(200, "application/json", json);
}

void handleFileUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = "/" + upload.filename;
    uploadFile = SD.open(filename, FILE_WRITE);
    currentUploadFile = upload.filename;
    uploading = true;
    uploadProgress = 0;
    uploadTotal = upload.totalSize; // may be 0 if unknown
    oledTargetPercent = 0.0;
    oledDisplayPercent = 0.0;
    lastSmoothMillis = millis();
    showMessage("Upload started", upload.filename);
    // initial draw
    updateOLEDProgressBar();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      uploadFile.write(upload.buf, upload.currentSize);
    }
    uploadProgress += upload.currentSize;
    if (upload.totalSize > 0) {
      float percent = ((float)uploadProgress / (float)upload.totalSize) * 100.0;
      if (percent > 100.0) percent = 100.0;
      oledTargetPercent = percent;
    } else {
      // unknown total: show activity
      oledTargetPercent += 0.8;
      if (oledTargetPercent > 99.0) oledTargetPercent = 99.0;
    }
    // immediate update to feel responsive
    updateOLEDProgressBar();
  } else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile) uploadFile.close();
    showMessage("Upload Done", upload.filename);
    uploading = false;
    uploadProgress = 0;
    uploadTotal = 0;
    oledTargetPercent = 100.0;
    oledDisplayPercent = 100.0;
    updateOLEDProgressBar();

    // LED blink + buzzer beep 3 times
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, HIGH);
      tone(BUZZER_PIN, 2000, 120); // 2kHz 120ms beep
      delay(180);
      digitalWrite(LED_PIN, LOW);
      delay(120);
    }
    delay(400);
    oledDisplayPercent = 0.0;
    oledTargetPercent = 0.0;
    updateDisplayStatus();
  }
}

void handleRoot() {
  server.send(200, "text/html", buildHTML());
}

// ----------------- Setup -----------------
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED init failed"));
    while (1);
  }

  showMessage("Connecting...", "WiFi: " + String(ssid));
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 30) {
    delay(500);
    Serial.print(".");
    attempt++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    // AP fallback
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSSID, apPass);
    IPAddress apIP = WiFi.softAPIP();
    showMessage("AP Mode", "SSID: " + String(apSSID), "IP: " + apIP.toString());
    Serial.println();
    Serial.print("Started AP: ");
    Serial.println(apSSID);
    Serial.print("AP IP: ");
    Serial.println(apIP.toString());
  } else {
    IPAddress IP = WiFi.localIP();
    Serial.println();
    Serial.println("WiFi Connected! IP: " + IP.toString());
    showMessage("WiFi Connected", "IP: " + IP.toString());
  }

  // Init SD
  sdPresent = false;
  if (!SD.begin(SD_CS)) {
    showMessage("SD Mount Failed!", "Check card & CS pin");
    Serial.println("SD Mount Failed!");
    // sdPresent remains false; UI will show "No SD"
  } else {
    sdPresent = checkSDPresent();
    showMessage("SD Card OK", "Starting Server...");
  }

  // Routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/list", HTTP_GET, handleList);
  server.on("/delete", HTTP_GET, handleDelete);
  server.on("/download", HTTP_GET, handleDownload);
  server.on("/upload", HTTP_POST, []() { server.send(200, "text/plain", "Upload OK"); }, handleFileUpload);
  server.on("/free", HTTP_GET, handleFree);
  server.on("/status", HTTP_GET, handleStatus);

  server.begin();
  IPAddress ipToShow = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP() : WiFi.softAPIP();
  showMessage("Server Ready", "IP: " + ipToShow.toString());
  Serial.println("Server started at: " + ipToShow.toString());

  lastUpdate = millis();
  lastSmoothMillis = millis();
  sdBlinkMillis = millis();
}

// ----------------- Main Loop -----------------
void loop() {
  server.handleClient();

  unsigned long now = millis();

  // SD presence check occasionally (lightweight)
  if (now - lastUpdate >= 2000) { // every 2 seconds
    bool presentNow = checkSDPresent();
    if (presentNow != sdPresent) {
      sdPresent = presentNow;
      // flash message on SD inserted/removed
      if (sdPresent) showMessage("SD Inserted", "Card detected");
      else showMessage("SD Removed", "Insert SD card");
      delay(600);
    }
    lastUpdate = now;
  }

  // SD blink state for "No SD" animation (toggle every 600ms)
  if (!sdPresent) {
    if (now - sdBlinkMillis >= 600) {
      sdBlinkMillis = now;
      sdBlinkState = !sdBlinkState;
      // If not uploading, redraw status to reflect blink
      if (!uploading) updateDisplayStatus();
    }
  } else {
    // ensure blink off when sd present
    sdBlinkState = false;
  }

  // Smooth interpolation for OLED percent (runs frequently)
  if (uploading || oledTargetPercent > 0.01) {
    unsigned long dt = now - lastSmoothMillis;
    if (dt >= 50) { // update ~20Hz
      lastSmoothMillis = now;
      float diff = oledTargetPercent - oledDisplayPercent;
      float step = 0.0;
      if (diff > 10.0) step = 4.0;
      else if (diff > 5.0) step = 2.0;
      else if (diff > 1.0) step = 0.6;
      else step = 0.2;
      if (diff > 0) {
        oledDisplayPercent += min(step, diff);
      } else {
        oledDisplayPercent += max(-step, diff);
      }
      if (oledDisplayPercent < 0) oledDisplayPercent = 0;
      if (oledDisplayPercent > 100) oledDisplayPercent = 100;
      // draw progress bar
      updateOLEDProgressBar();
    }
  } else {
    // Not uploading — update general status every 5s
    static unsigned long lastGeneral = 0;
    if (now - lastGeneral > 5000) {
      updateDisplayStatus();
      lastGeneral = now;
    }
  }
}
