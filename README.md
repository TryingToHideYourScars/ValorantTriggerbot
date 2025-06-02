# 🎯 Valorant Triggerbot

<p align="center">
  <img src="https://media.tenor.com/vPvvrLpzjAUAAAAC/please-i-want-to-to-play-league-downstairs-so-bad.gif" width="400" alt="league gif" />
</p>

A high-performance triggerbot built in C for maximum speed and precision. Designed for users who value direct control, clean code, and reliable execution.

---

---

## 📌 Project Overview

This triggerbot scans screen pixels in real-time and simulates mouse input the moment a target is detected. It is built entirely in **pure C**, using **WinAPI** to minimize latency and maximize responsiveness.

---

## 🖥️ Requirements

To compile and run this project, you will need:

- [GCC (MinGW-w64)](https://sourceforge.net/projects/mingw-w64/) — 64-bit version for Windows
- A basic understanding of using terminal commands

---

## ⚙️ How to Build

1. Install **MinGW-w64**
2. Open a terminal in the project directory (where `main.c` and `helper.c` are located)
3. Run the following command:

```bash
gcc main.c helper.c -o triggerbot.exe -lgdi32 -luser32
