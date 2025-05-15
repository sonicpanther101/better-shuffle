#pragma once

#include "../vendor/hnswlib/hnswlib/hnswlib.h"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <stdexcept>

class HNSWVectorDB {
private:
    hnswlib::L2Space* space;
    hnswlib::HierarchicalNSW<float>* index;
    int dim;
    std::vector<std::unordered_map<std::string, std::string>> metadata;
    std::vector<float> data_buffer;
    bool index_loaded = false;

    // Helper function to parse a CSV line
    std::unordered_map<std::string, std::string> parse_csv_line(const std::string& line, const std::vector<std::string>& headers) {
        std::unordered_map<std::string, std::string> record;
        std::stringstream ss(line);
        std::string token;
        size_t i = 0;
        
        while (std::getline(ss, token, ',')) {
            if (i < headers.size()) {
                record[headers[i]] = token;
            }
            i++;
        }
        
        return record;
    }

    // Helper function to normalize features
    void normalize_features(std::vector<float>& features) {
        float norm = 0.0f;
        for (float val : features) {
            norm += val * val;
        }
        norm = std::sqrt(norm);
        if (norm > 0) {
            for (float& val : features) {
                val /= norm;
            }
        }
    }

public:
    HNSWVectorDB(int dimension = 16, int max_elements = 10000, int M = 16, int ef_construction = 200) 
        : dim(dimension) {
        space = new hnswlib::L2Space(dim);
        index = new hnswlib::HierarchicalNSW<float>(space, max_elements, M, ef_construction);
    }

    ~HNSWVectorDB() {
        delete index;
        delete space;
    }

    // Load data from CSV file and create vector database
    void load_from_csv(const std::string& csv_file_path) {
        std::ifstream file(csv_file_path);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + csv_file_path);
        }

        std::string line;
        std::vector<std::string> headers;

        // Read headers
        if (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string header;
            while (std::getline(ss, header, ',')) {
                headers.push_back(header);
            }
        }

        // Read data
        while (std::getline(file, line)) {
            auto record = parse_csv_line(line, headers);
            metadata.push_back(record);

            // Extract features (modify this based on which columns you want to use as features)
            std::vector<float> features;
            try {
                features.push_back(std::stof(record["Popularity"]));
                features.push_back(std::stof(record["BPM"]));
                features.push_back(std::stof(record["Dance"]));
                features.push_back(std::stof(record["Energy"]));
                features.push_back(std::stof(record["Acoustic"]));
                features.push_back(std::stof(record["Instrumental"]));
                features.push_back(std::stof(record["Happy"]));
                features.push_back(std::stof(record["Speech"]));
                features.push_back(std::stof(record["Live"]));
                features.push_back(std::stof(record["Loud (Db)"]));
                // Add more features as needed
            } catch (...) {
                continue; // Skip records with invalid data
            }

            // Pad with zeros if needed to match dimension
            while (features.size() < static_cast<size_t>(dim)) {
                features.push_back(0.0f);
            }

            // Normalize features
            normalize_features(features);

            // Add to data buffer
            data_buffer.insert(data_buffer.end(), features.begin(), features.end());
        }

        // Add data to index
        for (size_t i = 0; i < metadata.size(); i++) {
            index->addPoint(data_buffer.data() + i * dim, i);
        }
    }

    // Load a previously saved index from file
    void load_index(const std::string& index_file_path, const std::string& metadata_csv_path = "") {
        if (index) delete index;
        index = new hnswlib::HierarchicalNSW<float>(space, index_file_path);
        index_loaded = true;

        // If metadata CSV is provided, load it to populate the metadata
        if (!metadata_csv_path.empty()) {
            load_metadata_from_csv(metadata_csv_path);
        }
    }

    // Helper function to load just metadata from CSV
    void load_metadata_from_csv(const std::string& csv_file_path) {
        std::ifstream file(csv_file_path);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + csv_file_path);
        }

        std::string line;
        std::vector<std::string> headers;
        metadata.clear();

        // Read headers
        if (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string header;
            while (std::getline(ss, header, ',')) {
                headers.push_back(header);
            }
        }

        // Read data
        while (std::getline(file, line)) {
            metadata.push_back(parse_csv_line(line, headers));
        }
    }

    // Save the index to a file
    void save_index(const std::string& file_path) {
        index->saveIndex(file_path);
    }

    // Get metadata for a specific item
    std::unordered_map<std::string, std::string> get_metadata(size_t id) {
        if (id >= metadata.size()) {
            throw std::out_of_range("Invalid ID");
        }
        return metadata[id];
    }

    // Search for similar items
    std::vector<std::pair<size_t, float>> search(const std::vector<float>& query, size_t k = 5) {
        if (query.size() != static_cast<size_t>(dim)) {
            throw std::invalid_argument("Query dimension doesn't match index dimension");
        }

        std::vector<std::pair<size_t, float>> results;
        auto pq = index->searchKnn(query.data(), k);
        
        while (!pq.empty()) {
            results.emplace_back(pq.top().second, pq.top().first);
            pq.pop();
        }
        
        return results;
    }
};