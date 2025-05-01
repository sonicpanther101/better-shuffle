#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <filesystem>

#include "metadata.h"

int main() {
    // print current directory using fstream
    std::cout << "Current directory is: " << std::filesystem::current_path() << std::endl;

    AudioMetadata meta = readAudioMetadata("../songs/Ain't No Rest For The Wicked.mp3");

    std::cout << "Length: " << meta.duration << std::endl;

    return 0;
}