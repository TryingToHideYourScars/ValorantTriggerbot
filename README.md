# 🎯 Valorant Triggerbot

A high-performance triggerbot built in C for maximum speed and precision. Designed for users who value direct control, clean code, and reliable execution.

---

<div align="center">
  <img src="https://media.tenor.com/vPvvrLpzjAUAAAAC/please-i-want-to-to-play-league-downstairs-so-bad.gif" width="400" />
</div>

---

## 📌 Project Overview

This triggerbot operates by scanning specific screen pixels in real time and simulating mouse input the moment a target is detected. It’s written in **pure C** to achieve **minimal latency** and **maximum efficiency**, with direct calls to the Windows API for low-level access.

---

## 🖥️ Requirements

To build and run this project, you’ll need:

- **GCC (MinGW-w64)** – 64-bit version for Windows  
  👉 [Download from SourceForge](https://sourceforge.net/projects/mingw-w64/)

- Basic familiarity with compiling C code in the terminal

---

## ⚙️ How to Build

1. Download and install [MinGW-w64](https://sourceforge.net/projects/mingw-w64/)
2. Open a terminal in the project folder (where `main.c` and `helper.c` are located)
3. Run the following command:

```bash
gcc main.c helper.c -o triggerbot.exe -lgdi32 -luser32
