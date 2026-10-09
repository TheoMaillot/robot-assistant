// WavFileSource.hpp
#ifndef WAVFILESOURCE_HPP
#define WAVFILESOURCE_HPP

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <thread>
#include <chrono>
#include <atomic>
#include <functional>
#include <cmath>

class WavFileSource {
public:
    using AudioCallback = std::function<void(const std::vector<float>& samples, bool is_speech)>;

    WavFileSource(const std::string& wav_path,
                  size_t period_size = 1600,      // même valeur que ton AudioCapture
                  float vad_threshold = 0.02f,
                  bool realtime_pacing = true)     // simule le timing réel (utile pour tester la latence globale)
        : path_(wav_path), period_size_(period_size),
          vad_threshold_(vad_threshold), realtime_pacing_(realtime_pacing), running_(false) {}

    ~WavFileSource() { stop(); }

    bool init() {
        std::ifstream file(path_, std::ios::binary);
        if (!file) {
            std::cerr << "Impossible d'ouvrir " << path_ << std::endl;
            return false;
        }

        char riff[4]; file.read(riff, 4);           // "RIFF"
        file.seekg(4, std::ios::cur);                // taille totale (ignorée)
        char wave[4]; file.read(wave, 4);            // "WAVE"

        // Cherche le chunk "fmt " puis "data"
        uint16_t num_channels = 0, bits_per_sample = 0;
        uint32_t sample_rate = 0;

        char chunk_id[4];
        uint32_t chunk_size;
        while (file.read(chunk_id, 4)) {
            file.read(reinterpret_cast<char*>(&chunk_size), 4);

            if (std::strncmp(chunk_id, "fmt ", 4) == 0) {
                uint16_t audio_format;
                file.read(reinterpret_cast<char*>(&audio_format), 2);
                file.read(reinterpret_cast<char*>(&num_channels), 2);
                file.read(reinterpret_cast<char*>(&sample_rate), 4);
                file.seekg(6, std::ios::cur);        // byte_rate + block_align
                file.read(reinterpret_cast<char*>(&bits_per_sample), 2);
                if (chunk_size > 16) file.seekg(chunk_size - 16, std::ios::cur);
            }
            else if (std::strncmp(chunk_id, "data", 4) == 0) {
                raw_data_.resize(chunk_size);
                file.read(raw_data_.data(), chunk_size);
                break;
            }
            else {
                file.seekg(chunk_size, std::ios::cur);  // skip chunks inconnus
            }
        }

        if (num_channels != 1 || bits_per_sample != 16) {
            std::cerr << "Attendu: WAV mono 16 bits. Reçu: "
                      << num_channels << " canal(aux), " << bits_per_sample << " bits" << std::endl;
            std::cerr << "Convertis avec: sox input.wav -r 16000 -c 1 -b 16 output.wav" << std::endl;
            return false;
        }
        if (sample_rate != 16000) {
            std::cerr << "Attention: sample_rate=" << sample_rate
                      << " (attendu 16000), le pipeline peut mal se comporter" << std::endl;
        }

        std::cout << "WAV chargé: " << raw_data_.size() / 2 << " samples, "
                  << sample_rate << "Hz" << std::endl;
        return true;
    }

    void start(AudioCallback callback) {
        running_ = true;
        callback_ = callback;
        worker_thread_ = std::thread(&WavFileSource::playbackLoop, this);
    }

    void stop() {
        running_ = false;
        if (worker_thread_.joinable()) worker_thread_.join();
    }

private:
    std::string path_;
    size_t period_size_;
    float vad_threshold_;
    bool realtime_pacing_;
    std::atomic<bool> running_;
    std::vector<char> raw_data_;
    std::thread worker_thread_;
    AudioCallback callback_;

    float computeRMS(const std::vector<float>& buffer) {
        float sum = 0.0f;
        for (float v : buffer) sum += v * v;
        return std::sqrt(sum / static_cast<float>(buffer.size()));
    }

    void playbackLoop() {
        const int16_t* samples = reinterpret_cast<const int16_t*>(raw_data_.data());
        size_t total_samples = raw_data_.size() / 2;
        size_t pos = 0;

        while (running_ && pos < total_samples) {
            size_t chunk_len = std::min(period_size_, total_samples - pos);
            std::vector<float> float_buffer(period_size_, 0.0f);  // zero-pad le dernier chunk si besoin

            for (size_t i = 0; i < chunk_len; ++i) {
                float_buffer[i] = static_cast<float>(samples[pos + i]) / 32768.0f;
            }

            float energy = computeRMS(float_buffer);
            bool is_speech = (energy > vad_threshold_);

            if (callback_) callback_(float_buffer, is_speech);

            pos += chunk_len;

            if (realtime_pacing_) {
                // simule le rythme réel (100ms par chunk à 16kHz/1600 samples)
                auto chunk_duration_ms = (period_size_ * 1000) / 16000;
                std::this_thread::sleep_for(std::chrono::milliseconds(chunk_duration_ms));
            }
        }
        running_ = false;
    }
};

#endif