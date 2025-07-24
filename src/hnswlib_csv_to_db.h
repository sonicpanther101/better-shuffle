#pragma once

#include "../vendor/hnswlib/hnswlib/hnswlib.h"
#include <cstddef>
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

    // Smaller = more important, 0 = no weight
    float WEIGHTS[10] = {
        0.0f, // Popularity
        0.0f, // BPM
        0.0f, // Dance
        1.0f, // Energy
        1.0f, // Acoustic
        0.0f, // Instrumental
        1.0f, // Happy
        0.0f, // Speech
        0.0f, // Live
        0.0f // Loud (Db)
    };

    // Helper: trim whitespace
    static std::string trim(const std::string& str) {
        size_t start = 0;
        while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start]))) start++;
        size_t end = str.size();
        while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) end--;
        return str.substr(start, end - start);
    }

    // Helper: strip surrounding quotes
    static std::string strip_quotes(const std::string& str) {
        std::string t = trim(str);
        if (t.size() >= 2 && t.front() == '"' && t.back() == '"') {
            return t.substr(1, t.size() - 2);
        }
        return t;
    }

    // Helper to parse a CSV line, handling quoted fields and commas within them
    std::unordered_map<std::string, std::string> parse_csv_line(const std::string& line, const std::vector<std::string>& headers) {
        std::unordered_map<std::string, std::string> record;
        std::stringstream ss(line);
        std::string token;
        size_t i = 0;
        bool in_quote = false;
        std::string current_field;

        for (char c : line) {
            if (c == '"') {
                in_quote = !in_quote;
                current_field += c;
            } else if (c == ',' && !in_quote) {
                if (i < headers.size()) {
                    record[headers[i]] = strip_quotes(current_field);
                }
                current_field.clear();
                i++;
            } else {
                current_field += c;
            }
        }
        // Add the last field
        if (i < headers.size()) {
            record[headers[i]] = strip_quotes(current_field);
        }
        return record;
    }

    // Helper function to normalize features
    void normalize_features(std::vector<float>& features) {
        float norm = 0.0f;
        for (size_t i = 0; i < features.size(); i++) {
            norm += features[i] * features[i] * WEIGHTS[i];
        }
        norm = std::sqrt(norm);
        if (norm > 0) {
            for (float& val : features) {
                val /= norm;
            }
        }
    }

    // Helper to serialize metadata
    std::string serialize_metadata(const std::unordered_map<std::string, std::string>& meta) {
        std::ostringstream oss;
        for (const auto& [key, value] : meta) {
            oss << key.size() << ',' << key << ',' << value.size() << ',' << value << ';';
        }
        return oss.str();
    }

    // Helper to deserialize metadata
    std::unordered_map<std::string, std::string> deserialize_metadata(const std::string& data) {
        std::unordered_map<std::string, std::string> meta;
        std::istringstream iss(data);
        char delim;
        size_t key_size, val_size;
        
        while (iss >> key_size) {
            iss >> delim; // Read comma
            std::string key(key_size, ' ');
            iss.read(&key[0], key_size);
            
            iss >> delim >> val_size >> delim;
            std::string value(val_size, ' ');
            iss.read(&value[0], val_size);
            
            meta[key] = value;
            iss >> delim; // Read semicolon
        }
        return meta;
    }

public:
    // Constructor
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
        load_data_from_csv(csv_file_path);
        for (size_t i = 0; i < metadata.size(); i++) {
            index->addPoint(data_buffer.data() + i * dim, i);
        }
    }

    // Helper function to load just metadata from CSV
    void load_data_from_csv(const std::string& csv_file_path) {
        std::ifstream file(csv_file_path);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + csv_file_path);
        }

        std::string line;
        std::vector<std::string> headers;

        // Clear existing data
        metadata.clear();
        data_buffer.clear();

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

            // Extract features
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
            } catch (...) {
                continue; // Skip records with invalid data
            }

            // Pad with zeros if needed
            while (features.size() < static_cast<size_t>(dim)) {
                features.push_back(0.0f);
            }

            // Normalize features
            normalize_features(features);

            // Add to data buffer
            data_buffer.insert(data_buffer.end(), features.begin(), features.end());
        }
    }

    // Save entire database to file
    void save_db_to_file(const std::string& file_path) {
        // Save HNSW index
        index->saveIndex(file_path + ".index");

        // Save metadata and vectors
        std::ofstream data_file(file_path + ".data", std::ios::binary);
        if (!data_file) {
            throw std::runtime_error("Cannot open data file for writing");
        }

        // Write vector data
        size_t data_size = data_buffer.size();
        data_file.write(reinterpret_cast<const char*>(&data_size), sizeof(data_size));
        data_file.write(reinterpret_cast<const char*>(data_buffer.data()), data_size * sizeof(float));

        // Write metadata
        for (const auto& meta : metadata) {
            std::string serialized = serialize_metadata(meta);
            size_t len = serialized.size();
            data_file.write(reinterpret_cast<const char*>(&len), sizeof(len));
            data_file.write(serialized.data(), len);
        }
    }

    // Load entire database from file
    void load_db_from_file(const std::string& file_path) {
        // Load HNSW index
        if (index) delete index;
        index = new hnswlib::HierarchicalNSW<float>(space, file_path + ".index");

        // Load metadata and vectors
        std::ifstream data_file(file_path + ".data", std::ios::binary);
        if (!data_file) {
            throw std::runtime_error("Cannot open data file for reading");
        }

        // Read vector data
        size_t data_size;
        data_file.read(reinterpret_cast<char*>(&data_size), sizeof(data_size));
        data_buffer.resize(data_size);
        data_file.read(reinterpret_cast<char*>(data_buffer.data()), data_size * sizeof(float));

        // Read metadata
        metadata.clear();
        while (data_file.peek() != EOF) {
            size_t len;
            data_file.read(reinterpret_cast<char*>(&len), sizeof(len));
            std::string serialized(len, ' ');
            data_file.read(&serialized[0], len);
            metadata.push_back(deserialize_metadata(serialized));
        }
        index_loaded = true;
    }

    // Get metadata for a specific item
    std::unordered_map<std::string, std::string> get_metadata(size_t id) {
        if (id >= metadata.size()) {
            throw std::out_of_range("Invalid ID");
        }
        return metadata[id];
    }

    // Build a query vector from a metadata record
    std::vector<float> vector_from_metadata(const std::unordered_map<std::string,std::string>& record) const {
        std::vector<float> features;
        // Order must match how data_buffer was constructed
        const std::vector<std::string> keys = {
            "Popularity","BPM","Dance","Energy","Acoustic",
            "Instrumental","Happy","Speech","Live","Loud (Db)"
        };
        for (const auto& key : keys) {
            auto it = record.find(key);
            if (it != record.end()) {
                try {
                    features.push_back(std::stof(it->second));
                } catch (...) {
                    features.push_back(0.0f); // Default to 0.0f if conversion fails
                }
            } else {
                features.push_back(0.0f);
            }
        }
        // Normalize features, as done during data loading
        std::vector<float> normalized_features = features;
        float norm = 0.0f;
        for (float val : normalized_features) {
            norm += val * val;
        }
        norm = std::sqrt(norm);
        if (norm > 0) {
            for (float& val : normalized_features) {
                val /= norm;
            }
        }
        return normalized_features;
    }

    // Remove the nearest point to the given query
    void remove(const std::vector<float>& query, size_t k = 1) {
        if (query.size() != static_cast<size_t>(dim))
            throw std::invalid_argument("Query dimension doesn't match index dimension");
        // Find nearest neighbor(s)
        auto results = index->searchKnn(query.data(), k);
        if (results.empty()) return;
        // Remove top result
        size_t id_to_remove = results.top().second;
        index->markDelete(id_to_remove);
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

    std::vector<float> get_vector_by_song_name(const std::string& song_name) {
        if (data_buffer.empty()) {
            throw std::runtime_error("Vector data not loaded");
        }

        for (size_t i = 0; i < metadata.size(); ++i) {
            if (metadata[i].count("Song") && metadata[i]["Song"] == song_name) {
                std::vector<float> vec(data_buffer.begin() + i * dim, data_buffer.begin() + (i + 1) * dim);
                return vec;
            }
        }
        throw std::runtime_error("Song not found: " + song_name);
    }

    std::vector<std::string> get_songs_by_artist_name(const std::string& artist_name) {
        std::vector<std::string> songs;
        for (const auto& meta : metadata) {
            if (meta.count("Artist") && meta.at("Artist") == artist_name) {
                if (meta.count("Song")) {
                    songs.push_back(meta.at("Song"));
                }
            }
        }
        return songs;
    }

    std::vector<std::string> get_songs_by_album_name(const std::string& album_name) {
        std::vector<std::string> songs;
        for (const auto& meta : metadata) {
            if (meta.count("Album") && meta.at("Album") == album_name) {
                if (meta.count("Song")) {
                    songs.push_back(meta.at("Song"));
                }
            }
        }
        return songs;
    }

    std::vector<std::string> get_songs_by_genre_name(const std::string& genre_name) {
        std::vector<std::string> songs;
        for (const auto& meta : metadata) {
            if (meta.count("Genre") && meta.at("Genre") == genre_name) {
                if (meta.count("Song")) {
                    songs.push_back(meta.at("Song"));
                }
            }
        }
        return songs;
    }

    std::vector<float> get_average_vector_from_songs(const std::vector<std::string>& song_names) {
        std::vector<float> average_vector(dim, 0.0f);
        for (const auto& song_name : song_names) {
            auto song_vector = get_vector_by_song_name(song_name);
            for (size_t i = 0; i < dim; ++i) {
                average_vector[i] += song_vector[i];
            }
        }
        for (size_t i = 0; i < dim; ++i) {
            average_vector[i] /= song_names.size();
        }
        return average_vector;
    }
};