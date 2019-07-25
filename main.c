#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"

struct WAVE readWaveFile(FILE *fp, struct WAVE wave) {    
    // Read wave header
    fread(wave.waveHeader.sGroupID,                 sizeof(wave.waveHeader.sGroupID), 1, fp);
    fread(wave.waveHeader.dwFileLength,             sizeof(wave.waveHeader.dwFileLength), 1, fp);
    fread(wave.waveHeader.sRiffType,                sizeof(wave.waveHeader.sRiffType), 1, fp);

    // Read wave format chunk
    fread(wave.waveFormatChunk.sGroupID,            sizeof(wave.waveFormatChunk.sGroupID), 1, fp);
    fread(wave.waveFormatChunk.dwChunkSize,         sizeof(wave.waveFormatChunk.dwChunkSize), 1, fp);
    fread(wave.waveFormatChunk.wFormatTag,          sizeof(wave.waveFormatChunk.wFormatTag), 1, fp);
    fread(wave.waveFormatChunk.wChannels,           sizeof(wave.waveFormatChunk.wChannels), 1, fp);
    fread(wave.waveFormatChunk.dwSamplesPerSec,     sizeof(wave.waveFormatChunk.dwSamplesPerSec), 1, fp);
    fread(wave.waveFormatChunk.dwAvgBytesPerSec,    sizeof(wave.waveFormatChunk.dwAvgBytesPerSec), 1, fp);
    fread(wave.waveFormatChunk.wBlockAlign,         sizeof(wave.waveFormatChunk.wBlockAlign), 1, fp);
    fread(wave.waveFormatChunk.dwBitsPerSample,     sizeof(wave.waveFormatChunk.dwBitsPerSample), 1, fp);

    // Read wave data chunk
    fread(wave.waveFormatChunk.sGroupID,            sizeof(wave.waveFormatChunk.sGroupID), 1, fp);
    fread(wave.waveDataChunk.dwChunkSize,           sizeof(wave.waveDataChunk.dwChunkSize), 1, fp);
    wave.waveDataChunk.sampleData =                 (__uint16_t*) malloc(sizeof(char) * *wave.waveDataChunk.dwChunkSize);
    fread(wave.waveDataChunk.sampleData,            sizeof(wave.waveDataChunk.sampleData), 1, fp);    
}

void displayCompleteWave(struct WAVE wave) {
    //printf("\n(1-4):\t%zu\n", wave.waveHeader.sGroupID);
    //printf("\n(x-y):\t%zu\n", wave.waveDataChunk.dwChunkSize);
    //printf("\n%zu\n", wave.waveDataChunk.sampleData);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        unsigned int x;
        // printf("%zu\n", sizeof(x));             // 4 bytes (int)
        // printf("%zu\n", sizeof(__uint32_t));    // 4 bytes (int)
        // printf("%zu\n", sizeof(__uint16_t));    // 2 bytes (short)
        // printf("%zu\n", sizeof(__uint8_t));     // 1 byte (char)
        return printf("\nPlease input a valid .wav file\n");
    }
    printf("\nInput Wave Filename:\t%s\n", argv[1]);

    FILE *fp;
    fp = fopen(argv[1], "r");
    struct WAVE wave;

    readWaveFile(fp, wave);

    return 0;
}
