#include "../include/main.hpp"

void signal_handler(int) {
    is_running = false;
    if (g_mic) g_mic->stop();
    std::exit(0);
}

int main() {
    Client robot_client("192.168.42.25", 5001);
    AudioCapture mic("plughw:0,0", 16000, 1600, 0.1f);
    g_mic = &mic;
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    mic.configure_audio_mixer();
    // WavFileSource mic("/root/question.wav", 1600, 0.02f);
    SpeechDetector speech_detector;

    robot_client.connect_server();
    
    robot_client.send_command('S');
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cerr << "1" << std::endl;
    
    if (!mic.init()) {
        std::cerr << "Unable to initialise the microphone." << std::endl;
        mic.stop();
        return 1;
    }

    std::cerr << "2" << std::endl;

    mic.start([&speech_detector](const std::vector<float>& samples, bool is_speech) {
        speech_detector.processChunk(samples, is_speech, [](const std::vector<float>& utterance) {
            {
                std::cerr << "Speech detected" << std::endl;
                std::lock_guard<std::mutex> lock(queue_mutex);
                speech_queue.push(utterance);
            }
            queue_cv.notify_one();
        });
    });

    while(is_running) {
        std::vector<float> audio_speech;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            queue_cv.wait(lock, [] { return !speech_queue.empty() || !is_running; });
            if (!is_running) break;

            audio_speech = std::move(speech_queue.front());
            speech_queue.pop(); 
        }
        robot_client.send_audio(audio_speech);
        std::cerr << "audio sent" << std::endl;
    }
    
    mic.stop();
    robot_client.disconnect();
}