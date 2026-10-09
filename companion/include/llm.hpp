#ifndef LLM_HPP
#define LLM_HPP

#include <iostream>
#include <vector>
#pragma once
#include <string>
#include "llama.h"
#include <chrono>

#include "config.h"

class LLM {
public:
    LLM(const std::string& model_path);
    ~LLM();

    std::string generate    (const std::string& input);
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