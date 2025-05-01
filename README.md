# Better Shuffle

Better Shuffle is a music player designed to adapt to your mood without the need for "mood playlists". On your first play, you set a "mood," and the application adjusts its queue based on how simmilar songs feel to eachother, with a "spiciness" meter to change how random it feels.

This music player is both efficient and powerful, developed in [C++](https://en.wikipedia.org/wiki/C%2B%2B) using [SableUI](https://github.com/oliwilliams1/SableUI) for its desktop interface. Better Shuffle will have its backend compiled to [WebAssembly](https://webassembly.org) via [emscripten](https://emscripten.org), making the backend compatible across both desktop and mobile platforms with tailored user interfaces.

The WebAssembly backend will also be able to integrate with other WebAssembly custom-made modules, making plugins and custom components easy to make whilst ensuring safety.


## Libraries used
[libsndfile](https://github.com/libsndfile/libsndfile)

Roadmap: [HNSWLIB](https://github.com/nmslib/hnswlib), [SableUI](https://github.com/oliwilliams1/SableUI), [emscripten](https://emscripten.org)

## Current data collected from songs
- [x] Year released
- [ ] Key
- [x] BPM/Tempo https://getsongbpm.com/api
- [ ] Valence
- [ ] Arousal
- [ ] Popularity
- [ ] Happiness
- [ ] Dancability
- [ ] Energy
- [ ] Acousticness
- [ ] Instrumentalness
- [ ] Liveness
- [ ] Speechiness
- [x] Song length
- [ ] Analised timbre

## Vector Database
Better Shuffle uses [HNSWLIB](https://github.com/nmslib/hnswlib/blob/master/examples/cpp/EXAMPLES.md) as its vector database using (insert number of dimensions here) to analyze your songs and to find the most similar songs to play next.

## Building from Source
Ensure you have CMake installed on your machine with a C++ compiler like MSVC for windows, gcc for Linux, MacOS.

To ensure all third-party libraries are included, clone this repository with the recursive option:
```bash
git clone https://github.com/sonicpanther101/better-shuffle --recursive
cd better-shuffle
```

### Windows
1. Run ```setup.bat``` to generate project files for Microsoft Visual Studio via CMake.
2. Navigate to the build directory and open ```better-shuffle.sln```.
3. In Solution Explorer, set ```better-shuffle``` as the startup project.
4. Build and Run!

### Linux / macOS
1. Create a build directory and navigate into it:
```bash
mkdir build
cd build
```

2. Run CMake to configure the project:
```
cmake ..
```
3. Compile the project using ```make```. You can use the ```-j``` flag for faster compilation (e.g., ```-j12```):
```
make -j12
```
4. Run the application
```
./SableUI
```