#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <fstream>
#include <vector>

void save_wav(const std::string& filename, const std::vector<float>& audio, int sample_rate) {
    std::ofstream file(filename, std::ios::binary);

    int32_t data_size = audio.size() * sizeof(int16_t);
    int32_t chunk_size = 36 + data_size;
    int16_t num_channels = 1;
    int32_t byte_rate = sample_rate * num_channels * sizeof(int16_t);
    int16_t block_align = num_channels * sizeof(int16_t);
    int16_t bits_per_sample = 16;

    // RIFF/WAVE header
    file.write("RIFF", 4);
    file.write(reinterpret_cast<const char*>(&chunk_size), 4);
    file.write("WAVEfmt ", 8);
    int32_t subchunk1_size = 16;
    file.write(reinterpret_cast<const char*>(&subchunk1_size), 4);
    int16_t audio_format = 1; // PCM
    file.write(reinterpret_cast<const char*>(&audio_format), 2);
    file.write(reinterpret_cast<const char*>(&num_channels), 2);
    file.write(reinterpret_cast<const char*>(&sample_rate), 4);
    file.write(reinterpret_cast<const char*>(&byte_rate), 4);
    file.write(reinterpret_cast<const char*>(&block_align), 2);
    file.write(reinterpret_cast<const char*>(&bits_per_sample), 2);
    file.write("data", 4);
    file.write(reinterpret_cast<const char*>(&data_size), 4);

    // Samples converted to int16 [-32768, 32767]
    for (float sample : audio) {
        float clamped = std::max(-1.0f, std::min(1.0f, sample));
        int16_t pcm16 = static_cast<int16_t>(clamped * 32767.0f);
        file.write(reinterpret_cast<const char*>(&pcm16), sizeof(int16_t));
    }
}

#endif