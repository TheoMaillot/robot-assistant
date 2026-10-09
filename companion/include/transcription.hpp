#ifndef TRANSCRIPTION_HPP
#define TRANSCRIPTION_HPP

#include "whisper.h"
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>

#include "config.h"

class WhisperTranscriber {
public:
    WhisperTranscriber(const std::string& model_path);
    ~WhisperTranscriber() {
        if (ctx_) {
            whisper_free(ctx_);
        }
    }
    std::string transcribe(const std::vector<float>& pcm_samples);

private:
    struct whisper_context* ctx_;
};

#endif