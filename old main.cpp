#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <filesystem>
#include <set>
#include <iomanip>

// db vector dimensions

// popularity
// BPM
// dancea
// energy
// acoustic
// instrumental
// happy
// speech
// live
// loud

// #include "metadata.h"
#include "hnswlib_csv_to_db.h"

int main() {
    // print current directory using fstream
    std::cout << "Current directory is: " << std::filesystem::current_path() << std::endl;

    std::string base_path = "music_db"; // Base name without extension
    // std::string csv_path = "../songs/Playlist.csv";
    // std::string csv_path = "../songs/Chill.csv";
    std::string csv_path = "../songs/Ahh.csv";

    try {

        // Create vector database
        HNSWVectorDB db(10); // Using 10 dimensions

        // check if db is saved locally
        if (std::filesystem::exists(base_path + ".index") && 
            std::filesystem::exists(base_path + ".data")) {
            db.load_db_from_file(base_path);
        } else {
            db.load_from_csv(csv_path);
            db.save_db_to_file(base_path);
        }
        
        std::vector<float> weights = {
    0.0f, // Popularity - ignore completely
    0.15f, // BPM - important for rhythm
    0.1f,  // Dance - very important
    0.1f,  // Energy - very important
    0.05f, // Acoustic - extremely important
    0.2f,  // Instrumental - moderate importance
    0.1f,  // Happy - very important
    0.0f,  // Speech - ignore
    0.0f,  // Live - ignore
    0.15f  // Loud (Db) - important
};

        db.test_weights(weights, "Evening Announcements", csv_path);

        std::cout << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}