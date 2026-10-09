#include "../include/AudioBuffer.hpp"

void AudioBuffer::push(const std::vector<float>& chunk) {
    std::lock_guard<std::mutex> lock(mutex_);
    buffer_.insert(buffer_.end(), chunk.begin(), chunk.end());
    cv_.notify_one();
}

bool AudioBuffer::consume_if_ready(std::vector<float>& output, size_t min_samples) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (buffer_.size() >= min_samples) {
        output = std::move(buffer_);
        buffer_.clear();
        return true;
    }
    return false;
}

void AudioBuffer::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    buffer_.clear();
}