#ifndef AUDIOCAPTURE_HPP
#define AUDIOCAPTURE_HPP

#include <alsa/asoundlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <cmath>
#include <functional>
#include <cstdlib>

class AudioCapture {
public:
    using AudioCallback = std::function<void(const std::vector<float>& samples, bool is_speech)>;

    AudioCapture(const std::string& device_name = "default",
                 unsigned int sample_rate = 16000,
                 snd_pcm_uframes_t period_size = 1600, // 1600 samples = 100 ms at 16 kHz
                 float energy_threshold = 0.015f)
        : device_(device_name),
          rate_(sample_rate),
          period_size_(period_size),
          vad_threshold_(energy_threshold),
          running_(false),
          pcm_handle_(nullptr) {}

    ~AudioCapture() {
        stop();
    }

    bool init();

    void start(AudioCallback callback);

    void stop();

    void configure_audio_mixer();

private:
    std::string device_;
    unsigned int rate_;
    snd_pcm_uframes_t period_size_;
    float vad_threshold_;
    std::atomic<bool> running_;
    snd_pcm_t* pcm_handle_;
    std::thread worker_thread_;
    AudioCallback callback_;

    // Calculation of effective energy (Root Mean Square)
    float computeRMS(const std::vector<float>& buffer);

    void captureLoop();
};

#endif