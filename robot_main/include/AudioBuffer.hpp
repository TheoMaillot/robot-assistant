#ifndef AUDIOBUFFER_HPP
#define AUDIOBUFFER_HPP

#pragma once
#include <vector>
#include <mutex>
#include <condition_variable>

class AudioBuffer {
public:
    void push(const std::vector<float>& chunk);
    bool consume_if_ready(std::vector<float>& output, size_t min_samples);
    void clear();

private:
    std::vector<float> buffer_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

#endif