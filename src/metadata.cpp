#include "metadata.h"
#include <fstream>
#include <vector>
#include <cstring>
#include <cmath>
#include <iostream>

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
    int total_duration = 0;
    int total_bitrate = 0;

    while (file.read(frame_header, 4)) {
        if ((frame_header[0] & 0xFF) == 0xFF && (frame_header[1] & 0xE0) == 0xE0) {
            // MP3 frame header found
            int bitrate_index = (frame_header[2] >> 4) & 0x0F;
            int sampling_rate_index = (frame_header[2] >> 2) & 0x03;
            int padding = (frame_header[2] >> 1) & 0x01;

            // Bitrate table (kbps)
            const int bitrate_table[16] = {
                0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0
            };
            int bitrate = bitrate_table[bitrate_index];

            // Sampling rate table (Hz)
            const int sampling_rate_table[4] = {44100, 48000, 32000, 0};
            int sampling_rate = sampling_rate_table[sampling_rate_index];

            if (bitrate == 0 || sampling_rate == 0) break;

            // Frame size calculation
            int frame_size = ((144 * bitrate * 1000) / sampling_rate) + padding;
            total_bitrate += bitrate;
            total_frames++;

            // Duration per frame (ms)
            float frame_duration = (144 * 1000.0f) / sampling_rate;
            total_duration += frame_duration;
        } else {
            break;
        }
        file.seekg(-3, ios::cur); // Move back to check next frame
    }

    if (total_frames > 0) {
        metadata.bitrate = total_bitrate / total_frames;
        metadata.duration = total_duration;
        // metadata.duration = static_cast<int>(total_duration / 1000.0f);
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
        }
        // Add other format handlers here
    }
    return AudioMetadata(); // Unsupported format
}