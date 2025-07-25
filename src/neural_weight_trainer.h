#pragma once

#include <tiny_dnn/tiny_dnn.h>
#include <vector>
#include <utility>
#include <random>
#include <filesystem>
#include <algorithm>

class NeuralWeightTrainer {
private:
    tiny_dnn::network<tiny_dnn::sequential> net;
    tiny_dnn::adam optimizer;

    // (input_vector, target_weights_scaled_to_[-1,1], score)
    std::vector<std::tuple<std::vector<float>, std::vector<float>, float>> training_data;
    std::vector<float> last_output;

public:
    NeuralWeightTrainer() {
        using namespace tiny_dnn;

        net << fully_connected_layer(10, 32)
            << relu_layer()
            << fully_connected_layer(32, 32)
            << relu_layer()
            << fully_connected_layer(32, 10)
            << tanh_layer(); // Output in range [-1, 1]

        last_output = std::vector<float>(10, 0.0f);
    }

    std::vector<float> generate_weights() {
        tiny_dnn::vec_t input(last_output.begin(), last_output.end());

        auto output = net.predict(input);

        last_output = std::vector<float>(output.begin(), output.end());

        std::vector<float> weights = last_output;
        // Convert from [-1, 1] to [0.0, 0.3] for example
        for (float& w : weights)
            w = std::clamp((w + 1.0f) * 0.15f, 0.0f, 0.3f);

        return weights;
    }

    void add_feedback(const std::vector<float>& weights_used, int score) {
        float normalized_score = static_cast<float>(score) / 10.0f;

        std::vector<float> input = last_output; // input that led to these weights

        // Scale target to [-1, 1]
        std::vector<float> target_scaled;

        for (float w : weights_used) {
            float scaled = std::clamp((w / 0.15f) - 1.0f, -1.0f, 1.0f);
            target_scaled.push_back(scaled);
        }

        training_data.emplace_back(input, target_scaled, normalized_score);
    }

    void train(int epochs = 10) {
        using namespace tiny_dnn;
        std::vector<vec_t> inputs;
        std::vector<vec_t> targets;

        for (const auto& [input, target_scaled, score] : training_data) {
            vec_t i(input.begin(), input.end());
            vec_t t(target_scaled.begin(), target_scaled.end());

            // Optionally amplify loss for higher scores
            float weight = std::clamp(score, 0.1f, 1.0f);  // Ensure minimum weight

            // Repeat data proportional to score (simple way to scale impact)
            int repeat = static_cast<int>(weight * 5); // e.g. score 0.8 → 4 times
            for (int r = 0; r < repeat; ++r) {
                inputs.push_back(i);
                targets.push_back(t);
            }
        }

        if (!inputs.empty())
            net.fit<mse>(optimizer, inputs, targets, 1, epochs);
    }

    void save() {
        net.save("weights.nn");
    }

    bool load() {
        if (!std::filesystem::exists("weights.nn")) return false;
        net.load("weights.nn");
        return true;
    }
};