# Smart Task Managing Digital Clock ⏰ | IoT Productivity System

A smart IoT-based digital clock that goes beyond simply displaying time—it helps users manage daily tasks through real-time reminders, cloud synchronization, and a custom web dashboard.

This project integrates **hardware, cloud computing, and web development** to create a productivity-focused smart device that reminds users of scheduled tasks directly through a physical clock interface.


Web Dashboard: https://smart-clock-c7f60.web.app/tasks
---

## Problem Statement
Traditional clocks only display time and alarms but lack intelligent task management capabilities.

This project solves that problem by creating a smart clock that:

- Displays real-time clock data
- Shows scheduled tasks
- Alerts users with buzzer notifications
- Allows task management through a web dashboard
- Syncs everything using Firebase cloud infrastructure

---

## Key Features

✅ Real-time time tracking using DS3231 RTC module  
✅ Task reminders displayed on LCD screen  
✅ Time displayed on TM1637 7-segment display  
✅ Buzzer notifications for reminders  
✅ Custom web dashboard for task creation  
✅ Firebase real-time cloud synchronization  
✅ ESP32 WiFi-based communication  
✅ Task prioritization and status tracking  

---

## Tech Stack

### Hardware
- ESP32
- DS3231 RTC Module
- TM1637 7-Segment Display
- 16x2 LCD Display
- Buzzer Module

### Software
- HTML
- CSS
- JavaScript
- Firebase Realtime Database

### Tools
- Arduino IDE
- Firebase Console
- GitHub

---

## System Architecture

User → Web Dashboard → Firebase → ESP32 → Clock Display + Buzzer

### Workflow:
1. User adds task via web dashboard  
2. Task gets stored in Firebase Realtime Database  
3. ESP32 fetches task data via WiFi  
4. Clock displays current time  
5. LCD shows scheduled task reminders  
6. Buzzer alerts user at task time  

---

## Project Structure

```bash
Smart-Digital-Clock/
│
├── Frontend/        # Web dashboard source code
├── FIREBASE/        # Cloud backend configuration
├── ESP32/           # Hardware source code
├── DOCS/            # Circuit diagrams & screenshots
├── REPORT/          # Project documentation
└── README.md
```

---

## Real-World Applications

- Student task management  
- Office productivity reminders  
- Smart home scheduling  
- Personal time management assistant  

---

## Future Enhancements

- Voice assistant integration  
- Mobile application support  
- AI-based task scheduling  
- Smart notification analytics  

---

## Project Demo

Web Dashboard: Add your deployed URL here  

Demo Video: Add your demo link here  

---

## Project Highlights

This project demonstrates:

✔ IoT Development  
✔ Embedded Systems  
✔ Cloud Integration  
✔ Web Development  
✔ Real-Time Synchronization  
✔ Problem Solving through Technology  

---

Built with innovation to make time management smarter 🚀
