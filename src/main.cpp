#include "neural_weight_trainer.h"
#include "hnswlib_csv_to_db.h"
#include <iostream>
#include <string>

int main() {
    NeuralWeightTrainer trainer;
    HNSWVectorDB db(10);
    std::string csv_path = "../songs/Playlist.csv";
    db.load_from_csv(csv_path);

    while (true) {
        auto weights = trainer.generate_weights();
        db.test_weights(weights, "Long Drives", csv_path);

        std::cout << "\nScore (1–10, or 0 to exit): ";
        int score;
        std::cin >> score;
        if (score == 0) break;

        trainer.add_feedback(weights, score);
        trainer.train(); // You can increase epochs later
    }

    return 0;
}