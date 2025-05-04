#include "metadata.h"
#include <fstream>
#include <vector>
#include <cstring>
#include <cmath>
#include <iostream>
#include <algorithm>

using namespace std;

uint32_t synchsafeToUInt(const char bytes[4]) {
    uint32_t result = 0;
    for (int i = 0; i < 4; ++i) {
        result = (result << 7) | (bytes[i] & 0x7F);
    }
    return result;
}

string getFrameText(const char* data, size_t data_size) {
    if (data_size < 1) return "";
    const char* text_start = data + 1;
    size_t text_length = data_size - 1;
    while (text_length > 0 && text_start[text_length - 1] == '\0') {
        text_length--;
    }
    return string(text_start, text_length);
}

AudioMetadata readMP3Metadata(const std::string& filePath) {
    AudioMetadata metadata;
    ifstream file(filePath, ios::binary);
    if (!file) {
        cerr << "Error opening file" << endl;
        return metadata;
    }

    // Read ID3v2 tag
    char header[10];
    file.read(header, 10);
    if (strncmp(header, "ID3", 3) != 0) {
        cerr << "No ID3v2 tag found" << endl;
        return metadata;
    }

    uint32_t tag_size = synchsafeToUInt(header + 6);
    uint32_t total_tag_size = 10 + tag_size;

    vector<char> tag_buffer(total_tag_size);
    file.seekg(0);
    file.read(tag_buffer.data(), total_tag_size);

    size_t pos = 10;
    while (pos < total_tag_size) {
        if (pos + 10 > total_tag_size) break;

        char frame_id[5];
        memcpy(frame_id, &tag_buffer[pos], 4);
        frame_id[4] = '\0';
        pos += 4;

        if (frame_id[0] == '\0') break;

        uint32_t frame_size = synchsafeToUInt(&tag_buffer[pos]);
        pos += 4;
        pos += 2; // Skip flags

        if (pos + frame_size > total_tag_size) break;

        const char* frame_data = &tag_buffer[pos];
        pos += frame_size;

        string value = getFrameText(frame_data, frame_size);
        if (value.empty()) continue;

        if (strcmp(frame_id, "TIT2") == 0) {
            metadata.title = value;
        } else if (strcmp(frame_id, "TPE1") == 0) {
            metadata.artist = value;
        } else if (strcmp(frame_id, "TALB") == 0) {
            metadata.album = value;
        } else if (strcmp(frame_id, "TYER") == 0) {
            metadata.year = value;
        } else if (strcmp(frame_id, "TRCK") == 0) {
            metadata.track = value;
        } else if (strcmp(frame_id, "TCON") == 0) {
            metadata.genre = value;
        }
    }

    // Calculate duration and bitrate from MP3 frames
    file.seekg(total_tag_size);
    char frame_header[4];
    int total_frames = 0;
    double total_duration = 0; // Changed to double to accumulate in milliseconds
    int total_bitrate = 0;

    while (file.read(frame_header, 4)) {
        if ((frame_header[0] & 0xFF) == 0xFF && (frame_header[1] & 0xE0) == 0xE0) {
            int version = (frame_header[1] >> 3) & 0x03;
            int layer = (frame_header[1] >> 1) & 0x03;
            int bitrate_index = (frame_header[2] >> 4) & 0x0F;
            int sampling_rate_index = (frame_header[2] >> 2) & 0x03;
            int padding = (frame_header[2] >> 1) & 0x01;

            const int bitrate_table[16] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0};
            int bitrate = bitrate_table[bitrate_index];

            const int sampling_rate_table[4] = {44100, 48000, 32000, 0};
            int sampling_rate = sampling_rate_table[sampling_rate_index];

            if (bitrate == 0 || sampling_rate == 0) break;

            // Calculate frame size and duration
            int frame_size;
            if (layer == 3) { // Layer I
                frame_size = (12 * bitrate * 1000 / sampling_rate + padding) * 4;
            } else if (layer == 2) { // Layer II
                frame_size = (144 * bitrate * 1000 / sampling_rate + padding);
            } else { // Layer III (MP3)
                frame_size = (144 * bitrate * 1000 / sampling_rate + padding);
            }

            float frame_duration;
            if (version == 3 || version == 2) { // MPEG-1 or MPEG-2
                int samples_per_frame = (layer == 3) ? 576 : 1152;
                frame_duration = static_cast<float>(samples_per_frame) / sampling_rate * 1000; // in milliseconds
            } else {
                break; // unsupported version
            }

            total_bitrate += bitrate;
            total_frames++;
            total_duration += frame_duration;

            // Skip the rest of the frame
            if (frame_size > 4) {
                file.seekg(frame_size - 4, ios::cur);
            } else {
                file.seekg(0, ios::cur);
            }
        } else {
            // Attempt to resync by moving 1 byte forward
            file.seekg(-3, ios::cur);
        }
    }

    if (total_frames > 0) {
        metadata.bitrate = total_bitrate / total_frames;
        metadata.duration = total_duration / 1000.0; // Convert ms to seconds
    }

    return metadata;
}

AudioMetadata readFLACMetadata(const std::string& filePath) {
    AudioMetadata metadata;
    ifstream file(filePath, ios::binary | ios::ate);
    if (!file) {
        cerr << "Error opening FLAC file" << endl;
        return metadata;
    }

    streamsize file_size = file.tellg();
    file.seekg(0);

    // Check FLAC signature
    char signature[4];
    file.read(signature, 4);
    if (strncmp(signature, "fLaC", 4) != 0) {
        cerr << "Not a FLAC file" << endl;
        return metadata;
    }

    bool last_block = false;
    uint32_t sample_rate = 0;
    uint64_t total_samples = 0;

    while (!last_block && file) {
        char header[4];
        file.read(header, 4);
        if (file.gcount() != 4) break;

        uint8_t type_byte = static_cast<uint8_t>(header[0]);
        last_block = (type_byte & 0x80) != 0;
        uint8_t block_type = type_byte & 0x7F;
        uint32_t block_length = (static_cast<uint8_t>(header[1]) << 16) |
                               (static_cast<uint8_t>(header[2]) << 8) |
                               static_cast<uint8_t>(header[3]);

        vector<char> block_data(block_length);
        file.read(block_data.data(), block_length);
        if (file.gcount() != block_length) break;

        if (block_type == 0) { // STREAMINFO
            if (block_length < 34) {
                cerr << "Invalid STREAMINFO block" << endl;
                continue;
            }

            // Parse sample rate (20 bits)
            sample_rate = (static_cast<uint32_t>(static_cast<unsigned char>(block_data[10])) << 12);
            sample_rate |= (static_cast<uint32_t>(static_cast<unsigned char>(block_data[11])) << 4);
            sample_rate |= (static_cast<unsigned char>(block_data[12]) >> 4);

            // Parse total_samples (36 bits)
            uint32_t raw_total = (static_cast<uint32_t>(static_cast<unsigned char>(block_data[13])) << 16);
            raw_total |= (static_cast<uint32_t>(static_cast<unsigned char>(block_data[14])) << 8);
            raw_total |= static_cast<unsigned char>(block_data[15]);
            total_samples = raw_total;

        } else if (block_type == 4) { // VORBIS_COMMENT
            auto read_le32 = [](const char* data) {
                return static_cast<uint32_t>(static_cast<unsigned char>(data[0])) |
                       (static_cast<uint32_t>(static_cast<unsigned char>(data[1])) << 8) |
                       (static_cast<uint32_t>(static_cast<unsigned char>(data[2])) << 16) |
                       (static_cast<uint32_t>(static_cast<unsigned char>(data[3])) << 24);
            };

            const char* data = block_data.data();
            size_t pos = 0;

            uint32_t vendor_len = read_le32(data + pos);
            pos += 4 + vendor_len; // Skip vendor string

            if (pos + 4 > block_length) continue;
            uint32_t num_comments = read_le32(data + pos);
            pos += 4;

            for (uint32_t i = 0; i < num_comments; ++i) {
                if (pos + 4 > block_length) break;
                uint32_t comment_len = read_le32(data + pos);
                pos += 4;
                if (pos + comment_len > block_length) break;

                string comment(data + pos, comment_len);
                pos += comment_len;

                size_t eq = comment.find('=');
                if (eq == string::npos) continue;

                string key = comment.substr(0, eq);
                string value = comment.substr(eq + 1);

                // Case-insensitive comparison
                transform(key.begin(), key.end(), key.begin(), ::toupper);

                if (key == "TITLE") {
                    metadata.title = value;
                } else if (key == "ARTIST") {
                    metadata.artist = value;
                } else if (key == "ALBUM") {
                    metadata.album = value;
                } else if (key == "DATE") {
                    metadata.year = value;
                } else if (key == "TRACKNUMBER") {
                    metadata.track = value;
                } else if (key == "GENRE") {
                    metadata.genre = value;
                }
            }
        }
    }

    if (sample_rate > 0 && total_samples > 0) {
        metadata.duration = static_cast<double>(total_samples) / sample_rate;
    }

    if (metadata.duration > 0 && file_size > 0) {
        metadata.bitrate = static_cast<int>((file_size * 8) / (metadata.duration * 1000));
    }

    return metadata;
}

AudioMetadata readAudioMetadata(const std::string& filePath) {
    // Check file extension and call appropriate reader
    size_t dot_pos = filePath.find_last_of('.');
    if (dot_pos != string::npos) {
        string ext = filePath.substr(dot_pos + 1);
        if (ext == "mp3") {
            return readMP3Metadata(filePath);
        } else if (ext == "flac") {
            return readFLACMetadata(filePath);
        }
        // Add other format handlers here
    }
    return AudioMetadata(); // Unsupported format
}