# **VisionText: Real Time TTS System for Blind Indiviual**
### Developed by Jayesh Vyas and Jainam Bheda
## About Project:

- VisionText is made to help blind individuals read printed text through sound. It speaks out the words seen by the camera, giving users more freedom and confidence. This project is a small step to make their everyday life easier and more connected.ng or descending.


## Features
- 📷 Text Detection: Captures printed text using the ESP32-CAM.

- 🧠 OCR Recognition: Extracts text using Tesseract.js.

- 🔊 Text-to-Speech: Speaks the detected text through a speaker.

- 🔌 Portable Setup: Works with a power bank, no external power needed.

- 🎧 Audio Feedback: Helps users understand their surroundings through sound.

- 🕶️ Wearable Design: Mounted on spectacles or a stand for easy use.

- 🌐 Backend Server: Node.js handles OCR processing and communication.

## 🔩 Hardware

| Component           | Purpose                                      |
|---------------------|----------------------------------------------|
| **ESP32-CAM**        | Captures images of printed text             |
| **Speaker**          | Outputs audio feedback to the user          |
| **Power Bank**       | Powers the system for portability           |
| **Stand** | Holds the ESP32-CAM in position       |

---

## 💻 Software

| Part              | Technology / Language                         |
|-------------------|------------------------------------------------|
| **OCR Engine**     | Tesseract.js                                  |
| **Backend Server** | Node.js + Express.js                          |
| **TTS (Audio)**    | Text-to-Speech API                            |
| **Programming Languages** | **Arduino (for ESP32)**, **JavaScript (for server)** |

## ⚙️ Setup Instructions

### 🔌 Hardware Setup

- **Mount the ESP32-CAM** on spectacles or a stable stand.
- **Connect the speaker** to the ESP32-CAM (or via an audio module).
- **Power the setup** using a portable power bank.
- Ensure the camera is positioned correctly to capture printed text.

## 💻 Software Setup

### 1. ESP32-CAM (Arduino Code)
- Install the **ESP32 board support** in the Arduino IDE.
- Upload the Arduino sketch to ESP32-CAM (code available in the `ESP32_Code` folder).
- Make sure to include required libraries like `WiFi.h` and `ESP32CAM.h`.
- Configure Wi-Fi credentials and camera settings in the sketch.

### 2. Node.js Backend (OCR + TTS)

## Clone the repository
```bash
git clone https://github.com/jayesh-vyas/Vision_Text.git
cd VisionText/server
```

# Install dependencies
```bash
npm install
```

# Start the server
```bash
node index.js
```

## 👥 Authors


### 👤 Jainam Bheda  
https://github.com/JainamBheda

### 👤 Jayesh Vyas  
https://github.com/jayesh-vyas

## 🙏 Thank You

Thank you for taking the time to explore our project.  
We hope VisionText makes a small yet meaningful difference in someone's life.  
Your support, feedback, or even a star ⭐ means a lot to us!

Jainam Bheda & Jayesh Vyas

