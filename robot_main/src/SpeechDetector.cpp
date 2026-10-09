#include "../include/SpeechDetector.hpp"

void SpeechDetector::processChunk(const std::vector<float>& chunk, bool is_speech, UtteranceCallback on_utterance_ready) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (state_ == VADState::LISTENING) {
        pre_roll_buffer_.push_back(chunk);
        if (pre_roll_buffer_.size() > pre_roll_limit_) {
            pre_roll_buffer_.pop_front();
        }

        if (is_speech) {
            state_ = VADState::RECORDING;
            current_utterance_.clear();
            silence_counter_ = 0;

            for (const auto& pre_chunk : pre_roll_buffer_) {
                current_utterance_.insert(current_utterance_.end(), pre_chunk.begin(), pre_chunk.end());
            }
            pre_roll_buffer_.clear();

            current_utterance_.insert(current_utterance_.end(), chunk.begin(), chunk.end());
        }
    } 
    else if (state_ == VADState::RECORDING) {
        current_utterance_.insert(current_utterance_.end(), chunk.begin(), chunk.end());

        if (is_speech) {
            silence_counter_ = 0;
        } else {
            silence_counter_++;
        }

        size_t total_chunks = current_utterance_.size() / chunk.size();

        bool end_of_speech = (silence_counter_ >= silence_threshold_chunks_);
        bool timeout_reached = (total_chunks >= max_speech_chunks_);

        if (end_of_speech || timeout_reached) {
            if (total_chunks >= min_speech_chunks_) {
                on_utterance_ready(current_utterance_);
            }
            current_utterance_.clear();
            state_ = VADState::LISTENING;
            silence_counter_ = 0;
        }
    }
}

void SpeechDetector::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    state_ = VADState::LISTENING;
    silence_counter_ = 0;
    current_utterance_.clear();
    pre_roll_buffer_.clear();
}