#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <filesystem>
#include <set>

// #include "metadata.h"
#include "hnswlib_csv_to_db.h"

int main() {
    // print current directory using fstream
    std::cout << "Current directory is: " << std::filesystem::current_path() << std::endl;

    std::string base_path = "music_db"; // Base name without extension
    std::string csv_path = "../songs/Playlist.csv";

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

        std::string song_name = "Porcelain";

        try {
            auto vec = db.get_vector_by_song_name(song_name);
            std::cout << "\nVector for song \"" << song_name << "\":" << std::endl;
            for (float val : vec) {
                std::cout << val << ", ";
            }
            std::cout << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Reverse search error: " << e.what() << std::endl;
        }

        HNSWVectorDB tempDB = db;
        std::vector<float> query = {0.251997, 0.622581, 0.444701, 0.271762, 0.227291, 0, 0.474347, 0, 0, -0.0247056};
        std::set<size_t> removed_ids;

        for (int i = 0; i < 10; ++i) {
            // Search for k+N results to account for already "removed" items
            size_t k_search = 5; // Search for more results than needed
            std::vector<std::pair<size_t, float>> search_results = db.search(query, k_search + removed_ids.size()); 
            
            size_t id_to_remove = -1;
            std::unordered_map<std::string, std::string> metadata;
            float distance = 0.0f;
            bool found_new_item = false;

            // Iterate through search results to find the first one that hasn't been "removed" yet
            for (const auto& result : search_results) {
                if (removed_ids.find(result.first) == removed_ids.end()) {
                    id_to_remove = result.first;
                    distance = result.second;
                    metadata = db.get_metadata(id_to_remove);
                    found_new_item = true;
                    break; 
                }
            }

            if (found_new_item) {
                db.remove(query); // Mark for deletion in HNSW (soft delete)
                removed_ids.insert(id_to_remove); // Keep track of it in our set

                std::cout << "Song: " << metadata["Song"]
                          << ", Artist: " << metadata["Artist"]
                          << ", Album: " << metadata["Album"]
                          << ", Distance: " << distance << "\n";
                
                // Update query to the vector of the just-"removed" item for the next iteration's search
                query = db.vector_from_metadata(metadata);
            } else {
                std::cout << "No new unique items found to remove in this iteration." << std::endl;
                break; // Exit loop if no new items can be found
            }
        }

        std::cout << "\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}