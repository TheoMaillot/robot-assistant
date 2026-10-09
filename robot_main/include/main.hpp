#ifndef MAIN_HPP
#define MAIN_HPP

#define SAMPLE_RATE 16000
#define WINDOW_SIZE 32000

#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <csignal>

#include "config.h"
#include "AudioCapture.hpp"
#include "AudioBuffer.hpp"
#include "SpeechDetector.hpp"
#include "client.hpp"
#include "WavFileSource.hpp"

AudioCapture* g_mic = nullptr;

std::queue<std::vector<float>> speech_queue;
std::mutex queue_mutex;
std::condition_variable queue_cv;
std::atomic<bool> is_running(true);

#endif