#include "../include/correction.hpp"

Corrector::Corrector(const std::string& model_path) {
    llama_backend_init();

    llama_model_params mparams = llama_model_default_params();
    model_ = llama_load_model_from_file(model_path.c_str(), mparams);
    if (!model_) {
        std::cerr << "Error: Unable to load the template." << std::endl;
        return;
    }

    vocab_ = llama_model_get_vocab(model_);

    llama_context_params cparams = llama_context_default_params();
    cparams.n_ctx = 2048;          // Petit contexte suffisant pour des commandes vocales
    cparams.n_threads = 4;
    ctx_ = llama_new_context_with_model(model_, cparams);

    sampler_ = llama_sampler_chain_init(llama_sampler_chain_default_params());
    llama_sampler_chain_add(sampler_, llama_sampler_init_greedy());
}

Corrector::~Corrector() {
    if (sampler_) llama_sampler_free(sampler_);
    if (ctx_) llama_free(ctx_);
    if (model_) llama_free_model(model_);
    llama_backend_free();
}

void Corrector::reset_context() {
    if (ctx_) {
        llama_free(ctx_);
        
        llama_context_params cparams = llama_context_default_params();
        cparams.n_ctx = max_ctx_; // ou 1024
        cparams.n_threads = 4;
        ctx_ = llama_new_context_with_model(model_, cparams);
    }
    n_past_ = 0;
    has_active_context_ = false;
}

void Corrector::eval_tokens(const std::vector<llama_token>& tokens) {
    llama_batch batch = llama_batch_get_one(const_cast<llama_token*>(tokens.data()), tokens.size());
    llama_decode(ctx_, batch);
    n_past_ += tokens.size();
}

std::string Corrector::correct(const std::string& noisy_text) {
    if (!ctx_ || !model_) return noisy_text;

    auto now = std::chrono::steady_clock::now();
    
    if (has_active_context_) {
        auto elapsed_sec = std::chrono::duration_cast<std::chrono::seconds>(now - last_interaction_).count();
        if (elapsed_sec > inactivity_time_limit) {
            reset_context();
        }
    }

    std::string prompt_chunk;
    if (!has_active_context_) {
        prompt_chunk = 
            "<|im_start|>system\n"
            "Tu es un correcteur orthographique et grammatical. "
            "Corrige uniquement la phrase utilisateur sans explications.<|im_end|>\n"
            "<|im_start|>user\n" + noisy_text + "<|im_end|>\n"
            "<|im_start|>assistant\n";
        has_active_context_ = true;
    } else {
        prompt_chunk = 
            "<|im_start|>user\n" + noisy_text + "<|im_end|>\n"
            "<|im_start|>assistant\n";
    }

    const int n_tokens_est = prompt_chunk.length();
    if (n_past_ + n_tokens_est + 64 >= max_ctx_) {
        reset_context();
        return correct(noisy_text);
    }

    const int n_prompt = -llama_tokenize(vocab_, prompt_chunk.c_str(), prompt_chunk.length(), NULL, 0, true, true);
    std::vector<llama_token> prompt_tokens(n_prompt);
    llama_tokenize(vocab_, prompt_chunk.c_str(), prompt_chunk.length(), prompt_tokens.data(), prompt_tokens.size(), true, true);

    eval_tokens(prompt_tokens);

    std::string result;
    std::vector<llama_token> generated_tokens;
    int max_tokens = 64;

    for (int i = 0; i < max_tokens; ++i) {
        llama_token new_token_id = llama_sampler_sample(sampler_, ctx_, -1);

        if (llama_vocab_is_eog(vocab_, new_token_id)) break;

        char buf[128];
        int n = llama_token_to_piece(vocab_, new_token_id, buf, sizeof(buf), 0, true);
        if (n > 0) result.append(buf, n);

        generated_tokens.push_back(new_token_id);

        llama_batch batch = llama_batch_get_one(&new_token_id, 1);
        if (llama_decode(ctx_, batch) != 0) break;
    }
    n_past_ += generated_tokens.size();

    std::string end_turn = "<|im_end|>\n";
    const int n_end = -llama_tokenize(vocab_, end_turn.c_str(), end_turn.length(), NULL, 0, false, true);
    std::vector<llama_token> end_tokens(n_end);
    llama_tokenize(vocab_, end_turn.c_str(), end_turn.length(), end_tokens.data(), end_tokens.size(), false, true);
    eval_tokens(end_tokens);

    last_interaction_ = std::chrono::steady_clock::now();
    return result;
}