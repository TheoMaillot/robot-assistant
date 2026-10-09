#ifndef SPEECHDETECTOR_HPP
#define SPEECHDETECTOR_HPP

#include "config.h"

enum class VADState {
    LISTENING,
    RECORDING
};

class SpeechDetector {
public:
    using UtteranceCallback = std::function<void(const std::vector<float>&)>;

    SpeechDetector(size_t silence_chunks = SILENCE_THRESHOLD, 
                   size_t pre_roll_chunks = PRE_VOICE_CHUNKS, 
                   size_t min_speech_chunks = 5,
                   size_t max_speech_chunks = 100)
        : state_(VADState::LISTENING),
          silence_threshold_chunks_(silence_chunks),
          pre_roll_limit_(pre_roll_chunks),
          min_speech_chunks_(min_speech_chunks),
          max_speech_chunks_(max_speech_chunks),
          silence_counter_(0) {}

    void processChunk(const std::vector<float>& chunk, bool is_speech, UtteranceCallback on_utterance_ready);

    void reset();

private:
    VADState state_;
    size_t silence_threshold_chunks_;
    size_t pre_roll_limit_;
    size_t min_speech_chunks_;
    size_t max_speech_chunks_;
    size_t silence_counter_;

    std::deque<std::vector<float>> pre_roll_buffer_;
    std::vector<float> current_utterance_;
    std::mutex mutex_;
};

#endif