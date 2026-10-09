#include "../include/transcription.hpp"

WhisperTranscriber::WhisperTranscriber(const std::string& model_path) : ctx_(nullptr) {
    struct whisper_context_params cparams = whisper_context_default_params();
    cparams.use_gpu = true;

    ctx_ = whisper_init_from_file_with_params(model_path.c_str(), cparams);
    if (!ctx_) {
        std::cerr << "Error: Unable to load the template from " << model_path << std::endl;
        return;
    }

    if (whisper_is_multilingual(ctx_) == 0) {
        std::cerr << "Warning: This template is in English (.en) only" << std::endl;
    } else {
        std::cout << "Multilingual template successfully loaded" << std::endl;
    }
}

std::string WhisperTranscriber::transcribe(const std::vector<float>& pcm_samples) {
        if (!ctx_) return "";

        struct whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
        
        wparams.language = LANGUAGE;
        wparams.detect_language = DETECT_LANGUAGE;
        wparams.n_threads = TRANSCR_THREADS;
        wparams.print_progress = false;
        wparams.print_special = false;
        wparams.audio_ctx = 512;

        if (whisper_full(ctx_, wparams, pcm_samples.data(), pcm_samples.size()) != 0) {
            std::cerr << "Whisper decoding failed." << std::endl;
            return "";
        }

        std::string result = "";
        int n_segments = whisper_full_n_segments(ctx_);
        for (int i = 0; i < n_segments; ++i) {
            result += whisper_full_get_segment_text(ctx_, i);
        }

        return result;
    }
