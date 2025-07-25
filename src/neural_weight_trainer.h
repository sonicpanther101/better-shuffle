#pragma once

#include "../vendor/tiny_dnn/tiny_dnn/tiny_dnn.h"
#include <vector>
#include <utility>
#include <random>

class NeuralWeightTrainer {
private:
    tiny_dnn::network<tiny_dnn::sequential> net;
    tiny_dnn::adam optimizer;
    std::vector<std::pair<std::vector<float>, float>> history;

public:
    NeuralWeightTrainer() {
        using namespace tiny_dnn;

        net << fully_connected_layer(10, 32)
            << relu_layer()
            << fully_connected_layer(32, 32)
            << relu_layer()
            << fully_connected_layer(32, 10)
            << tanh_layer(); // Output in range [-1, 1]
    }

    std::vector<float> generate_weights() {
        tiny_dnn::vec_t input(10, 0.0f); // Dummy input, optionally encode session state
        auto output = net.predict(input);

        std::vector<float> weights(output.begin(), output.end());
        // Convert from [-1, 1] to [0.0, 0.3] for example
        for (float& w : weights)
            w = (w + 1.0f) * 0.15f;

        return weights;
    }

    void add_feedback(const std::vector<float>& weights, int score) {
        float normalized_score = static_cast<float>(score) / 10.0f;
        history.emplace_back(weights, normalized_score);
    }

    void train(int epochs = 5) {
        using namespace tiny_dnn;
        std::vector<vec_t> inputs;
        std::vector<vec_t> targets;

        for (const auto& [w, score] : history) {
            vec_t input(10, 0.0f); // dummy
            vec_t target = w;
            for (float& val : target) {
                val = val / 0.15f - 1.0f; // scale back to [-1, 1]
            }
            inputs.push_back(input);
            targets.push_back(target);
        }

        net.fit<mse>(optimizer, inputs, targets, 1, epochs);
    }
};