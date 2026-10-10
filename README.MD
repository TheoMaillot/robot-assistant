# Robot assistant

An end-to-end companion robot system written in **C++**. 

The robot is based on a Milk-V Duo S board, handling real-time audio capture, energy-based voice activity detection (VAD), and network streaming, and an Companion Server running on my computer, performing fully offline Speech-to-Text (STT), Large Language Model (LLM) reasoning, and Text-to-Speech (TTS) synthesis.

---

## Current architecture Overview

---

## Key Features

- **Embedded Audio Pipeline**: Low-latency ALSA audio acquisition (`plughw:0,0`, 16 kHz mono) with hardware ring-buffer management and zero-copy conversion.
- **Voice Activity Detection (VAD)**: Embedded energy/RMS thresholding with pre-roll history to avoid clipping initial phonemes.
- **Offline Edge AI Pipeline**:
  - **STT**: `whisper.cpp` running quantized `large-v3-turbo` for high-accuracy multilingual transcription.
  - **Reasoning**: `llama.cpp` running `Qwen2.5-0.5B-Instruct` (Q4_K_M) with sub-second response latency (<600 ms).
  - **TTS**: `Kokoro-82M` neural voice synthesis accelerated by **ONNX Runtime C++**, supporting phonetic tokenization (Jieba, CMUDict, Pinyin).
- **Cross-Compilation**: Streamlined build system with CMake and cross-toolchain support for `riscv64-unknown-linux-musl`.

---