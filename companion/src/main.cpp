#include "../include/main.hpp"

int main() {
    Server server(5001);
    WhisperTranscriber transcriber_largeq8("third_party/whisper.cpp/models/ggml-large-v3-turbo-q8_0.bin");
    LLM llm("third_party/Qwen3/qwen2.5-0.5b-instruct-q4_k_m.gguf");
    Kokoro tts(
        "third_party/kokoro.cpp/models/kokoro-v1.0.onnx",
        "third_party/kokoro.cpp/models/voices-v1.0.bin",
        "third_party/kokoro.cpp/dict/vocab.txt"
    );

    server.open_server();

    while (is_running) {
        Message msg = server.receive_message();

        if (msg.type == MsgType::Disconnected) {
            std::cout << "Client disconnected." << std::endl;
            break;
        }
        if (msg.type == MsgType::Command && msg.command == 'X') {
            std::cout << "Closing" << std::endl;
            break;
        }
        if (msg.type == MsgType::Audio && !msg.audio.empty()) {
            auto start = std::chrono::steady_clock::now();
            std::string sentence = transcriber_largeq8.transcribe(msg.audio);
            auto finish = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = finish - start;
            std::cout << "Large Q8: " << sentence << " - END" << std::endl;
            std::cout << "Durée: " << elapsed_seconds.count() << " s" << std::endl;
            
            start = std::chrono::steady_clock::now();
            std::string response = llm.generate(sentence);
            finish = std::chrono::steady_clock::now();
            elapsed_seconds = finish - start;
            std::cout << "[Qwen2.5]     : " << response << std::endl;
            std::cout << "Durée: " << elapsed_seconds.count() << " s" << std::endl;

            start = std::chrono::steady_clock::now();
            auto [audio, sample_rate] = tts.create(response, voice, 1.0f);
            finish = std::chrono::steady_clock::now();
            elapsed_seconds = finish - start;
            std::cout << "Durée: " << elapsed_seconds.count() << " s" << std::endl;
            save_wav("output/output.wav", audio, sample_rate);
        }
    }

    server.close_server();
    return 0;
}