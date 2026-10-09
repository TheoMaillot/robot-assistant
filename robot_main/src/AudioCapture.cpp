#include "../include/AudioCapture.hpp"

bool AudioCapture::init() {
    int err;

    if ((err = snd_pcm_open(&pcm_handle_, device_.c_str(), SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        std::cerr << "ALSA open error (" << device_ << ") : " << snd_strerror(err) << std::endl;
        return false;
    }

    snd_pcm_hw_params_t* hw_params;
    snd_pcm_hw_params_alloca(&hw_params);
    snd_pcm_hw_params_any(pcm_handle_, hw_params);

    snd_pcm_hw_params_set_rate_resample(pcm_handle_, hw_params, 1);

    if ((err = snd_pcm_hw_params_set_access(pcm_handle_, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
        std::cerr << "set_access failed: " << snd_strerror(err) << std::endl;
    if ((err = snd_pcm_hw_params_set_format(pcm_handle_, hw_params, SND_PCM_FORMAT_S16_LE)) < 0)
        std::cerr << "set_format failed: " << snd_strerror(err) << std::endl;
    if ((err = snd_pcm_hw_params_set_channels(pcm_handle_, hw_params, 1)) < 0)
        std::cerr << "set_channels failed: " << snd_strerror(err) << std::endl;
    if ((err = snd_pcm_hw_params_set_rate_near(pcm_handle_, hw_params, &rate_, nullptr)) < 0)
        std::cerr << "set_rate_near failed: " << snd_strerror(err) << std::endl;
    if ((err = snd_pcm_hw_params_set_period_size_near(pcm_handle_, hw_params, &period_size_, nullptr)) < 0)
        std::cerr << "set_period_size_near failed: " << snd_strerror(err) << std::endl;

    snd_pcm_uframes_t buffer_size = period_size_ * 4;
    if ((err = snd_pcm_hw_params_set_buffer_size_near(pcm_handle_, hw_params, &buffer_size)) < 0)
        std::cerr << "set_buffer_size_near failed: " << snd_strerror(err) << std::endl;

    if ((err = snd_pcm_hw_params(pcm_handle_, hw_params)) < 0) {
        std::cerr << "Hardware configuration error: " << snd_strerror(err) << std::endl;
        return false;
    }

    std::cerr << "Negotiated: rate=" << rate_ << " period_size=" << period_size_ << std::endl;

    return true;
}

void AudioCapture::start(AudioCallback callback) {
    running_ = true;
    callback_ = callback;
    worker_thread_ = std::thread(&AudioCapture::captureLoop, this);
}

void AudioCapture::stop() {
    if (running_) {
        running_ = false;
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
    }
    if (pcm_handle_) {
        snd_pcm_close(pcm_handle_);
        pcm_handle_ = nullptr;
    }
}

float AudioCapture::computeRMS(const std::vector<float>& buffer) {
    float sum = 0.0f;
    for (float val : buffer) {
        sum += val * val;
    }
    return std::sqrt(sum / static_cast<float>(buffer.size()));
}

void AudioCapture::captureLoop() {
    std::vector<int16_t> raw_buffer(period_size_);
    std::vector<float> float_buffer(period_size_);

    std::cerr << "captureLoop started, period_size=" << period_size_ << std::endl;

    // Prepare the device and start the capture DMA
    snd_pcm_prepare(pcm_handle_);
    int start_err = snd_pcm_start(pcm_handle_);
    if (start_err < 0 && start_err != -EBUSY) {
        std::cerr << "snd_pcm_start error: " << snd_strerror(start_err) << std::endl;
    }

    while (running_) {
        // Blocking read of the audio segment from the ALSA DMA buffer
        long frames_read = snd_pcm_readi(pcm_handle_, raw_buffer.data(), period_size_);

        if (frames_read < 0) {
            std::cerr << "ALSA read error: " << snd_strerror(frames_read) << std::endl;
            // Handling buffer overflows (Overrun / Buffer Overflows)
            frames_read = snd_pcm_recover(pcm_handle_, frames_read, 0);
            if (frames_read < 0) {
                std::cerr << "Unrecoverable ALSA error: " << snd_strerror(frames_read) << std::endl;
                break;
            }
            continue;
        }

        // Normalisation from int16 to float32 [-1.0f, +1.0f]
        for (size_t i = 0; i < static_cast<size_t>(frames_read); ++i) {
            float_buffer[i] = static_cast<float>(raw_buffer[i]) / 32768.0f;
        }

        // Basic speech detection using RMS threshold
        float energy = computeRMS(float_buffer);
        bool is_speech = (energy > vad_threshold_);

        // Pass to callback (to the queue / Ring Buffer)
        if (callback_) {
            callback_(float_buffer, is_speech);
        }
    }

    std::cerr << "captureLoop exited" << std::endl;
}

void AudioCapture::configure_audio_mixer() {
    std::system("amixer -c 0 cset numid=1 on,on > /dev/null 2>&1");
    std::system("amixer -c 0 sset 'ADC' 8 > /dev/null 2>&1");
}