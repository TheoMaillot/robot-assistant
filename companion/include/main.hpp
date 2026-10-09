#ifndef MAIN_HPP
#define MAIN_HPP

#include <vector>
#include <atomic>
#include <chrono>

#include "server.hpp"
#include "config.h"
#include "transcription.hpp"
#include "llm.hpp"
#include "utils.hpp"
#include "Kokoro.h"

std::atomic<bool> is_running(true);
std::string voice = "ff_siwis";

#endif