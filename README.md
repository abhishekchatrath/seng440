# seng440
UVic SENG 440 Embedded Systems Project

## Setup
- Replace `Voice001.wav` in Makefile `mu:` section with the desired wave file to be compressed.
- Run `make` to clear the terminal, compile the `main.c` file and run the executable with the desired input wave file.
- Run `make opt` to clear the terminal, compile the `main_opt.c` file and run the executable with the desired input wave file.

## Optimizations
Certain software optimization techniques were applied to `main.c` and saved in a new file `main_opt.c` (with its corresponding header file `main_opt.h`). These optimizations have been described below:

### Global variables
- `FILE *fp` was made local to `main()`
- `struct WAVE wave` was made local to `main()`
- `struct WAVE_COMPRESSED waveCompressed` was changed to `__uint8_t *codewords` local to `compressDataSamples()` and `main()`
- `__uint64_t numSamples` was made local to `readWaveFile()` and `main()`

### Strength Reduction

### Software pipelining

### Loop unrolling

### Other
- `printf()` statements were removed from `compressDataSamples()` and `decompressDataSamples()`

## References
- [1] Lesson 104: Audio Compression, SENG440 Embedded Systems, <em>Mihai Sima</em>. Accessed July 16th, 2019.
- [2] Intro to Audio Programming, Part 2: Demystifying the WAV Format, <em>Microsoft Developer Blogs</em> [Online]. Available: https://blogs.msdn.microsoft.com/dawate/2009/06/23/intro-to-audio-programming-part-2-demystifying-the-wav-format/. Accessed July 18th, 2019.
- [3] Parsing a WAV file in C, <em>TRUELOGIC BLOG</em> [Online]. Available: http://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/. Accessed July 18th, 2019.
- [4] WAVE PCM soundfile format, <em>SAPP</em> [Online]. Available: http://soundfile.sapp.org/doc/WaveFormat/. Accessed July 19th, 2019.
- [5] RIFF WAVE (.WAV) file format, <em>Rob Ryan</em> [Online]. Available: http://www.neurophys.wisc.edu/auditory/riff-format.txt. Accessed July 19th, 2019.
