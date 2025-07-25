#include "neural_weight_trainer.h"
#include "hnswlib_csv_to_db.h"
#include <iostream>
#include <string>

int main() {
    NeuralWeightTrainer trainer;
    HNSWVectorDB db(10);
    std::string csv_path = "../songs/Playlist.csv";
    db.load_from_csv(csv_path);
    std::vector<std::string> training_songs = {
        "Long Drives", // BoyWithUke
        "BLINK WAVE", // blink-182
        "Killer Queen - Remastered 2011", // Queen
        "Galaxy", // BoyWithUke
        "Alpha", // C418
        "Come Hang Out", // AJR
        "21st Century", // Red Hot Chili Peppers
        "When I Come Around", // Green Day
        "\"99\"", // Barns Courtney
        "BABY SAID", // Måneskin
        "Trouble" // Cage The Elephant
    };

    if (trainer.load()) {
        std::cout << "Weights loaded from file.\n";
    }

    int song_iterator = 0;
    std::cout << "Training on " << training_songs.size() << " songs...\n";

    while (true) {
        auto weights = trainer.generate_weights();
        db.test_weights(weights, training_songs[song_iterator++], csv_path);

        std::cout << "\nScore (1-10, or 0 to exit): ";
        int score;
        std::cin >> score;
        if (score == 0) break;

        trainer.add_feedback(weights, score);
        trainer.train(100); // You can increase epochs later
    }

    std::cout << "\nTraining complete.\n";
    trainer.save();

    return 0;
}