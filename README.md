# better-shuffle

This is a [C++](https://en.wikipedia.org/wiki/C%2B%2B) project that uses [QT](https://www.qt.io) and compiles to [web assembly](https://webassembly.org) using [emscripten](https://emscripten.org)
This is a project to (Oli pls fill in the rest Mr Wordsmith)

## Data collected from songs
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

We are using [HNSWLIB](https://github.com/nmslib/hnswlib/blob/master/examples/cpp/EXAMPLES.md) as our vector database using (insert number of dimensions here) to analyze your songs and to find the most similar songs to play next
