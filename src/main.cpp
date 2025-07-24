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
    std::string csv_path = "../songs/Playlist.csv";

    try {

        // Create vector database
        HNSWVectorDB db(10); // Using 10 dimensions

        // check if db is saved locally
        // if (std::filesystem::exists(base_path + ".index") && 
        //     std::filesystem::exists(base_path + ".data")) {
        //     db.load_db_from_file(base_path);
        // } else {
            db.load_from_csv(csv_path);
            db.save_db_to_file(base_path);
        // }
        
        // Example search

        std::string song_name = "Long Drives";

        auto vec = db.get_vector_by_song_name(song_name);
        std::cout << "\nFound vector for song \"" << song_name << "\"!\n" << std::endl;

        std::vector<float> query = vec;
        std::set<size_t> removed_ids;

        std::printf("%-45s%-45s%-45s%-5s\n", "Song", "Artist", "Album", "Distance");
        std::printf("---------------------------------------------------------------------------------------------------------------------------\n");

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

                std::printf("%-45s%-45s%-45s%-5f\n", metadata["Song"].c_str(), metadata["Artist"].c_str(), metadata["Album"].c_str(), distance);
                
                // Update query to the vector of the just-"removed" item for the next iteration's search
                query = db.vector_from_metadata(metadata);
            } else {
                std::cout << "No new unique items found to remove in this iteration." << std::endl;
                break; // Exit loop if no new items can be found
            }
        }

        std::cout << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}