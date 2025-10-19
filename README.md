# ⚡ ESP32 WiFi File Manager / FTP Server

![GitHub Repo Stars](https://img.shields.io/github/stars/ElectriTrend/ESP32-File-Manager?style=for-the-badge)
![ESP32 Supported](https://img.shields.io/badge/ESP32-Supported-success?style=for-the-badge)
![SD Card Ready](https://img.shields.io/badge/SD%20Card-FAT32-orange?style=for-the-badge)
![License MIT](https://img.shields.io/badge/License-MIT-blue?style=for-the-badge)

> 🚀 A lightweight local **WiFi File Manager + FTP-style Server** built using **ESP32 + SD Card + OLED Display**, allowing you to **upload, download, and delete files locally — no internet required!**

---

## 📷 UI Preview


<img width="1440" height="814" alt="Screenshot 2025-10-19 153324" src="https://github.com/user-attachments/assets/8994366c-dea4-4c5e-a6a1-73308a49ae24" /><img width="1440" height="812" alt="Screenshot 2025-10-19 153350" src="https://github.com/user-attachments/assets/49075df1-5851-4281-9ec6-346218182421" />




---

## 📌 Project Overview
এই প্রোজেক্টে আমরা **ESP32 + SD Card Module + OLED Display** ব্যবহার করে তৈরি করেছি একটি **Local WiFi File Manager / FTP-style Server**।

🔹 এটি SD Card-এর ভিতরে ফাইল **Upload, Download, Delete** করার সুযোগ দেয়।  
🔹 Web Browser থেকেই আপনি আপনার ESP32-এর SD Card ম্যানেজ করতে পারবেন।  
🔹 OLED Display তে **Upload Progress Real-Time** দেখা যাবে।

> ⚙️ Perfect for **IoT File Transfer**, **Data Logging**, **Local Media Storage**, এবং **Embedded File System Management**।

## 📷 Diagram

<img width="1640" height="924" alt="ESP32 Embedded HTTP File Server SD Card Module Circuit Diagram" src="https://github.com/user-attachments/assets/fc2391dc-1c73-4461-861b-26ddb3071dd8" />
<img width="1640" height="924" alt="ESP32 Embedded HTTP File Server 0 93 Inch oled Display Circuit Diagram" src="https://github.com/user-attachments/assets/c4ab0207-ff4a-403f-a2ca-630563ac02c8" />
<img width="1640" height="924" alt="ESP32 Embedded HTTP File Server Main Circuit Diagram" src="https://github.com/user-attachments/assets/46b2c939-0b33-423a-8a0f-08c81471cc89" />


---

## ✨ Features
✅ File Upload / Download / Delete (Supports `.txt`, `.jpg`, `.mp3`, `.mp4`, `.zip`, etc.)  
✅ Real-Time Upload Progress Display on OLED (SSD1306 I2C)  
✅ Modern Web UI using HTML + CSS + JS (Async Fetch API Upload)  
✅ Auto Refresh File List after Upload/Delete  
✅ File Size Display with Table View  
✅ Works Locally (No Internet Required)  
✅ Fully Integrated with SD Card FAT32 File System  

---

## 🧰 Components Used

| Component | Description | Required |
|------------|-------------|-----------|
| ESP32 DevKit V1 | Main Controller Board | ✅ Yes |
| Micro SD Card Module | SPI-based Storage Interface | ✅ Yes |
| 0.96" OLED Display (SSD1306 I2C) | Real-time upload progress | ⚙ Optional |
| Jumper Wires | Connection setup | - |
| Breadboard | Prototype setup | - |

> 💡 *Future upgrade:* Add **password protection + file encryption** for secure access.

---

## ⚡ Circuit Connection

| ESP32 Pin | SD Card Module | OLED (SSD1306 I2C) |
|------------|----------------|--------------------|
| 3.3V | VCC | VCC |
| GND | GND | GND |
| GPIO 5 | CS | - |
| GPIO 18 | SCK | - |
| GPIO 23 | MOSI | - |
| GPIO 19 | MISO | - |
| GPIO 22 | - | SCL |
| GPIO 21 | - | SDA |

> 🧩 **Note:**  
> - SD Card SPI pins follow ESP32 default SPI bus configuration.  
> - OLED uses I2C address `0x3C` by default.

---

## ⚙️ Installation & Setup (Arduino IDE)

1️⃣ Install ESP32 Board Support via Arduino Preferences → Additional URL.
2️⃣ Install Required Libraries:

SD.h (Default for ESP32)

SPI.h (Default)

Wire.h (Default)

Adafruit SSD1306

Adafruit GFX Library
3️⃣ Open the main .ino file.
4️⃣ Update your WiFi SSID and Password inside the code.
5️⃣ Copy all files from data/ folder to SD Card root directory.
6️⃣ Upload code to ESP32.
7️⃣ Open Serial Monitor → Note the IP Address → Access via Browser.

✅ Example Access:
http://192.168.1.124/

---

## 🌐 How the Web File Manager Works

User connects to ESP32 over WiFi.

Browser loads index.html (UI).

JavaScript (Fetch API) sends uploaded files to ESP32 via HTTP POST.

ESP32 writes files to SD Card.

OLED displays real-time upload % progress.

File list auto-refreshes on completion.

🖥️ Simple, fast, and fully local — no external cloud required.

---

## 🔮 Future Upgrades

 🔐 Password-Protected Access (Admin Login)  
 
 📂 Folder Navigation + Multi-File Upload     
 
 🌍 Cloud Access via Telegram, ngrok, or DNS Tunnel      
 
 📱 Android / iOS App Integration     
 
 🧠 Encrypted File Transfer Support      

---

## 👨‍💻 Author

✨ Developed by ElectriTrend 

👉 YouTube:    https://www.youtube.com/@Electritrendbd      

👉 Facebook:   https://www.facebook.com/electritrendbd       

👉 Instagram:  https://www.instagram.com/electri.trend       

👉 Website:    https://sites.google.com/view/electritrend/home

---

## 📜 License

This project is licensed under the MIT License — you’re free to use, modify, and distribute it with proper credit.

---


## 📁 Project Folder Structure
```bash
ESP32-File-Manager/
├── src/
│   └── ESP32_File_Manager.ino
│
├── data/                # Web UI Files
│   ├── index.html
│   ├── style.css
│   └── script.js
│
├── assets/
│   └── preview.png      # Screenshot / GIF
│
├── wiring.svg           # Circuit Diagram
├── LICENSE              # MIT License
├── README.md            # You are here
└── .gitignore
