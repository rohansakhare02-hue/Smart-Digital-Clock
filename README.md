# Smart Task Managing Digital Clock ⏰

An IoT-based smart clock that not only displays time but also helps users manage tasks through real-time reminders.

## Features
- Real-time clock using DS3231 RTC
- Task reminders on LCD display
- Time display on TM1637 7-segment display
- Buzzer notifications
- Web dashboard for task management
- Firebase real-time synchronization
- ESP32 integration

## Tech Stack
- ESP32
- Firebase
- HTML
- CSS
- JavaScript
- DS3231 RTC Module
- LCD Display
- TM1637 Display

## Project Structure
📁 BACKEND → Firebase configuration  
📁 Frontend → Web dashboard  
📁 ESP32 → Microcontroller code  
📁 DOCS → Circuit diagrams/screenshots  
📁 REPORT → Project documentation  

## Working Flow
1. User adds tasks through web app
2. Task gets stored in Firebase
3. ESP32 fetches task
4. Clock displays reminder at scheduled time

## Future Improvements
- Voice assistant integration
- Mobile app integration
- AI scheduling suggestions
