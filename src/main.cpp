#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <filesystem>

#include "metadata.h"

int main() {
    // print current directory using fstream
    std::cout << "Current directory is: " << std::filesystem::current_path() << std::endl;

    std::string path = "../songs/Take Me to Church.flac";
    // std::string path = "../songs/Golden.flac";
    // std::string path = "../songs/Ain't No Rest For The Wicked.mp3";

    AudioMetadata meta = readAudioMetadata(path);

    std::cout << "Metadata for: " << path << "\n"
                  << "Title: " << meta.title << "\n"
                  << "Artist: " << meta.artist << "\n"
                  << "Album: " << meta.album << "\n"
                  << "Year: " << meta.year << "\n"
                  << "Track: " << meta.track << "\n"
                  << "Genre: " << meta.genre << "\n"
                  << "Duration: " << meta.duration << " seconds\n"
                  << "Bitrate: " << meta.bitrate << " kbps\n";

    return 0;
}