#pragma once

#include <string>

struct AudioMetadata {
    std::string title;
    std::string artist;
    std::string album;
    std::string year;
    std::string track;
    std::string genre;
    double duration; // in seconds
    int bitrate;  // in kbps
};

AudioMetadata readAudioMetadata(const std::string& filePath);