#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"

FILE *fp;
unsigned char buffer[4];
struct WAVE wave;


struct WAVE readWaveFile() {    

    printf("\nBegin Reading Wave File:\t...\n");


    // Read wave header
    fread(wave.waveHeader.sGroupID,                 sizeof(wave.waveHeader.sGroupID), 1, fp);
    
    fread(buffer,                                   sizeof(buffer), 1, fp);
    wave.waveHeader.dwFileLength = (buffer[0]) | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
    
    fread(wave.waveHeader.sRiffType,                sizeof(wave.waveHeader.sRiffType), 1, fp);
    

    // Read wave format chunk
    fread(wave.waveFormatChunk.sGroupID,            sizeof(wave.waveFormatChunk.sGroupID), 1, fp);
    
    fread(buffer,                                   sizeof(buffer), 1, fp);
    wave.waveFormatChunk.dwChunkSize = (buffer[0]) | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
    
    fread(buffer,                                   sizeof(__uint16_t), 1, fp);
    wave.waveFormatChunk.wFormatTag = buffer[0] | buffer[1] << 8;

    fread(buffer,                                   sizeof(__uint16_t), 1, fp);
    wave.waveFormatChunk.wChannels = (buffer[0]) | (buffer[1] << 8);

    fread(buffer,                                   sizeof(buffer), 1, fp);
    wave.waveFormatChunk.dwSamplesPerSec = (buffer[0]) | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

    fread(buffer,                                   sizeof(buffer), 1, fp);
    wave.waveFormatChunk.dwAvgBytesPerSec = (buffer[0]) | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
    
    fread(buffer,                                   sizeof(__uint16_t), 1, fp);
    wave.waveFormatChunk.wBlockAlign = (buffer[0]) | (buffer[1] << 8);
    
    fread(buffer,                                   sizeof(__uint16_t), 1, fp);
    wave.waveFormatChunk.dwBitsPerSample = (buffer[0]) | (buffer[1] << 8);


    // Read wave data chunk
    fread(buffer, sizeof(buffer), 1, fp);
    int notData = strcmp(buffer, "data");
    int fileEnd = wave.waveHeader.dwFileLength - 40;
    if (notData) {
        while (fileEnd--  >= 0) {
            fread(buffer, sizeof(buffer), 1, fp);
            notData = strcmp(buffer, "data");
            if (!notData) {
                break;
            }
            fseek(fp, -3, SEEK_CUR);
        }
    }
    strcpy(wave.waveDataChunk.sGroupID, buffer);

    fread(buffer,                                sizeof(buffer), 1, fp);
    wave.waveDataChunk.dwChunkSize = (buffer[0]) | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
    
    // wave.waveDataChunk.sampleData =                 (__uint16_t*) malloc(sizeof(char) * *wave.waveDataChunk.dwChunkSize);
    // fread(wave.waveDataChunk.sampleData,            sizeof(wave.waveDataChunk.sampleData), 1, fp);    
    
    printf("Reading Wave File:\t\tCOMPLETE\n\n");
}

void displayCompleteWave() {
    char displayBuffer[100];

    printf("Display Wave File Details:\t...\n");

    fwrite("(01-04): sGroupID\t\t", 1, 19, stdout);
    fwrite(wave.waveHeader.sGroupID, sizeof(wave.waveHeader.sGroupID), 1, stdout);

    printf("\n(05-08): dwFileLength\t\t%u", wave.waveHeader.dwFileLength);

    fwrite("\n(09-12): sRiffType\t\t", 1, 21, stdout);
    fwrite(wave.waveHeader.sRiffType, sizeof(wave.waveHeader.sRiffType), 1, stdout);

    fwrite("\n(13-16): sGroupID\t\t", 1, 20, stdout);
    fwrite(wave.waveFormatChunk.sGroupID, sizeof(wave.waveFormatChunk.sGroupID), 1, stdout);
    
    printf("\n(17-20): dwChunkSize\t\t%u", wave.waveFormatChunk.dwChunkSize);
    printf("\n(21-22): wFormatTag\t\t%u", wave.waveFormatChunk.wFormatTag);
    printf("\n(23-24): wChannels\t\t%u", wave.waveFormatChunk.wChannels);
    printf("\n(25-28): dwSamplesPerSec\t%u", wave.waveFormatChunk.dwSamplesPerSec);
    printf("\n(29-32): dwAvgBytesPerSec\t%u", wave.waveFormatChunk.dwAvgBytesPerSec);
    printf("\n(33-34): wBlockAlign\t\t%u", wave.waveFormatChunk.wBlockAlign);
    printf("\n(35-36): dwBitsPerSample\t%u", wave.waveFormatChunk.dwBitsPerSample);

    fwrite("\n(37-40): sGroupID\t\t", 1, 20, stdout);
    fwrite(wave.waveDataChunk.sGroupID, sizeof(wave.waveDataChunk.sGroupID), 1, stdout);

    printf("\n(41-44): dwChunkSize\t\t%u", wave.waveDataChunk.dwChunkSize);


    printf("\n\n");
}


int main (int argc, char **argv) {
    if (argc < 2) {
        perror("\nPlease input a valid .wav file\n");
        return printf("\nPlease input a valid .wav file\n");
    }

    printf("\nInput Wave Filename:\t%s\n", argv[1]);

    fp = fopen(argv[1], "rb");  //read in binary mode
    if (fp == NULL) {
        printf("Error opening file %s", argv[1]);
    }

    readWaveFile();
    displayCompleteWave();

    fclose(fp);
    return 0;
}
