# NClite: Real-Time Adaptive Noise Suppression

![C++](https://img.shields.io/badge/C++-17-blue.svg) ![JUCE](https://img.shields.io/badge/Framework-JUCE-orange.svg) ![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS-lightgrey)

**NClite** is a high-performance, low-latency audio processing application designed to isolate human speech from complex background noise. Unlike standard gates, NClite uses a **hybrid DSP pipeline** combining spectral subtraction with time-domain hysteresis to clean audio in real-time.

![NClite Interface](screenshot.png)
*(Add a screenshot of your app here named screenshot.png)*

## 🚀 Key Features

* **🧠 AI Environmental Profiling:** Automatically listens to room tone for 3 seconds to calibrate thresholds based on RMS magnitude and noise floor density.
* **🎛️ Hybrid DSP Engine:** Combines **Wiener Filtering** (for stationary hiss/hum) with a **Smart Hysteresis Gate** (for dynamic background music/chatter).
* **🎧 Delta Monitoring:** "Hear Noise Only" mode inverts the phase of the processed signal, allowing the user to audition *exactly* what is being removed to ensure signal integrity.
* **🔊 3 Processing Modes:**
    * **Standard:** Balanced noise reduction.
    * **Broadcast:** Adds a "Radio DJ" EQ curve (Bass/Air boost).
    * **Vocal Isolation:** Aggressive bandpass filtering (300Hz-3.5kHz) for extreme environments.
* **🛡️ Safety & Polish:** Integrated **Soft Clipper (tanh)** to prevent digital distortion and a variable **Mic Boost (+10dB)** for low-voltage inputs.

---

## 🔧 Technical Architecture

The audio engine processes signals in series through a multi-stage pipeline (`AudioEngine.cpp`):

### 1. Input Conditioning
* **Pre-Amplification:** A user-controlled gain stage (up to +10dB) normalizes quiet microphone inputs before processing.
* **Safety:** A digital Soft Clipper (`std::tanh`) ensures the signal never exceeds 0dBFS, preventing hard-clipping distortion.

### 2. Frequency-Domain Processing (The "Cleaner")
* **STFT (Short-Time Fourier Transform):** The signal is segmented into 1024-sample frames with 75% overlap using a Hann window to reduce spectral leakage.
* **Wiener Filter:** We implement an adaptive **Spectral Subtraction** algorithm. The system estimates the noise power spectral density (PSD) and calculates a gain mask based on the *A Priori* Signal-to-Noise Ratio (SNR).
* **Circular Buffering:** A custom FIFO ring buffer manages the misalignment between audio hardware callbacks and FFT window sizes.

### 3. Time-Domain Processing (The "Gate")
* **Smart Gate:** To handle dynamic noise (music, TV), a secondary gate is applied post-spectral cleaning.
* **Hysteresis Loop:** The gate uses dual thresholds—an "Open" threshold to detect voice onset and a lower "Close" threshold to prevent stuttering.
* **Envelope Shaping:** Features an instant Attack, **200ms Hold**, and **100ms Release** to preserve natural speech patterns and stop "gate chatter."

---

## 🛠️ Build Instructions

This project uses **CMake** and the **JUCE Framework**.

### Prerequisites
* C++ Compiler (MSVC for Windows, Clang/Xcode for Mac)
* CMake (3.15 or higher)
* Git

### Building
1.  **Clone the repository** (Recursive is required for JUCE):
    ```bash
    git clone --recursive [https://github.com/YOUR_USERNAME/NClite.git](https://github.com/YOUR_USERNAME/NClite.git)
    ```
2.  **Configure CMake**:
    ```bash
    cd NClite
    cmake -B build
    ```
3.  **Build**:
    ```bash
    cmake --build build --config Release
    ```

---

## 🎮 Usage Guide

1.  **Audio Settings:** Select your Microphone and Headphones (Output). *Note: Use headphones to prevent feedback loops.*
2.  **Mic Boost:** Use the fader on the left to increase gain if your waveform is too thin.
3.  **Analyze & Clean:** Click the blue button and remain silent for 3 seconds. The AI will measure your room noise.
4.  **Strength Knob:**
    * **0%:** Passthrough (0dB reduction).
    * **50%:** Balanced reduction (-12dB target).
    * **100%:** Aggressive reduction (-40dB target).
5.  **Hear Noise Only:** Toggle this to verify that only static/music is being removed, and not your voice.

---

## 📂 Project Structure

* `AudioEngine.cpp/hpp`: The core DSP logic (STFT, Gate, EQ, Filters).
* `MainComponent.cpp/hpp`: The GUI implementation, custom LookAndFeel, and visualizations.
* `STFTProcessor`: Handles the overlap-add math and FFT windowing.
* `WienerPost`: Implements the spectral subtraction algorithm.
* `NLMSFilter`: (Experimental) Adaptive filter for reference-channel cancellation.

---

## 📜 License
This project is built for educational purposes using the JUCE Framework (GPLv3).