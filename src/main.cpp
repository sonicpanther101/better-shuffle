#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <filesystem>

// #include "metadata.h"
#include "hnswlib_csv_to_db.h"

int main() {
    // print current directory using fstream
    std::cout << "Current directory is: " << std::filesystem::current_path() << std::endl;

    std::string path = "../songs/Adams Playlist #5326.csv";
    // std::string path = "../songs/Golden.flac";
    // std::string path = "../songs/Ain't No Rest For The Wicked.mp3";

    try {
        // Create vector database
        HNSWVectorDB db(10); // Using 10 dimensions
        
        // Load data from CSV file
        // db.load_from_csv(path);
        
        // Save the index to a file
        db.load_index("music_index.bin", path);
        
        // Example search (using dummy query)

        // Popularity
        // BPM
        // Dance
        // Energy
        // Acoustic
        // Instrumental
        // Happy
        // Speech
        // Live
        // Loud

        std::vector<float> query = {65, 130, 85, 67, 51, 0, 99, 10, 10, -7};
        auto results = db.search(query);
        
        std::cout << "\nSearch results:" << std::endl;
        for (const auto& result : results) {
            auto metadata = db.get_metadata(result.first);
            std::cout << "Song: " << metadata["Song"] 
                      << ", Artist: " << metadata["Artist"]
                      << ", Distance: " << result.second << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}