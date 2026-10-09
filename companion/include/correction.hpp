#ifndef CORRECTION_HPP
#define CORRECTION_HPP

#include <iostream>
#include <vector>
#pragma once
#include <string>
#include "llama.h"
#include <chrono>

#include "config.h"

class Corrector {
public:
    Corrector(const std::string& model_path);
    ~Corrector();

    std::string correct(const std::string& noisy_text);
    void reset_context();
private:
    llama_model* model_ = nullptr;
    llama_context* ctx_ = nullptr;
    const llama_vocab* vocab_ = nullptr;
    llama_sampler* sampler_ = nullptr;

    int inactivity_time_limit = 30;

    int n_past_ = 0;
    int max_ctx_ = 2048;
    bool has_active_context_ = false;
    std::chrono::steady_clock::time_point last_interaction_;

    void eval_tokens(const std::vector<llama_token>& tokens);
};

#endif