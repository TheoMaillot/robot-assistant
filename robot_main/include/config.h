#ifndef CONFIG_H
#define CONFIG_H

// VAD
#define SILENCE_THRESHOLD   1200 // 1500ms
#define PRE_VOICE_CHUNKS    3 // 300ms
#define MIN_SPEECH          5 // 500ms
#define MAX_SPEECH          100 // 10000ms

// TAG
#define CMD_TAG             'C'
#define AUDIO_TAG           'A'

#include <vector>
#include <deque>
#include <mutex>
#include <functional>

#endif