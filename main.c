#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"

FILE *fp;
unsigned char buffer[4];
struct WAVE wave;

unsigned long numSamples;
unsigned int sizeOfEachSample;


void readWaveFileHeaders() {
    printf("\nBegin Reading Wave Headers:\t...\n");

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

    printf("Reading Wave Headers:\t\tCOMPLETE\n\n");
}


void readWaveFileDataSamples() {
    if (wave.waveFormatChunk.wFormatTag == 1) {
        printf("Begin Reading PCM data:\t\t...\n");
        
        // numSamples = size of data in bits / (bits per sample * num of channels)
        numSamples = (wave.waveDataChunk.dwChunkSize * 8) / (wave.waveFormatChunk.dwBitsPerSample * wave.waveFormatChunk.wChannels);
        
        // sizeOfEachSample = size of each sample in bytes
        sizeOfEachSample = (wave.waveFormatChunk.dwBitsPerSample * wave.waveFormatChunk.wChannels) / 8;
        
        // printf("%lu\n%u\n", numSamples, sizeOfEachSample);
        wave.waveDataChunk.sampleData = calloc(numSamples, sizeOfEachSample);
        if (wave.waveDataChunk.sampleData == NULL) {
            printf("Could not allocate enough memory to read data samples\n");
            return;
        }

        for (int i = 0; i < numSamples; i++) {
            fread(buffer, sizeOfEachSample, 1, fp);
            wave.waveDataChunk.sampleData[i] = (buffer[0]) | (buffer[1] << 8);
        }

        printf("Reading PCM data:\t\tCOMPLETE\n\n");
    } else if (wave.waveFormatChunk.wFormatTag == 7) {
        printf("Begin Reading Mu Law data:\t...");
        printf("Reading Mu Law data:\t\tCOMPLETE\n\n");        
    }
}


void readWaveFile() {    
    readWaveFileHeaders();
    readWaveFileDataSamples();  
}


void displayWaveHeaders() {
    // char displayBuffer[100];

    printf("Display Wave Headers:\t...\n");

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

    printf("\nDisplaying Wave Headers:\t\tCOMPLETE\n\n");
}


void displayWaveDataSamples() {
    printf("Display Wave Data Samples:\t\t...\n");
    for (int i = 0; i < numSamples; i++) {
        printf("Sample %i:\t%hhx\n", i, wave.waveDataChunk.sampleData[i]);
    }
    printf("Displaying Wave Data Samples:\t\tCOMPLETE\n\n");
}

void saveWaveDataSamples() {
    printf("Saving Wave Data Samples to \"display.txt\"...\t");
    FILE *fpwriter = fopen("display.txt", "w");
    if (fpwriter == NULL) {
        printf("Could not write to \"display.txt\"");
        return;
    }
    char str[50];
    sprintf(str, "Wave Data Samples");
    fwrite(str, 1, strlen(str), fpwriter);
    for (int i = 0; i < numSamples; i++) {
        sprintf(str, "\nSample %i:\t%d", i, wave.waveDataChunk.sampleData[i]);
        fwrite(str, 1, strlen(str), fpwriter);
    }
    fwrite("\n", 1, 1, fpwriter);
    fclose(fpwriter);
    printf("DONE\n\n");
}


void displayCompleteWave() {
    displayWaveHeaders();
    // displayWaveDataSamples();
    saveWaveDataSamples();
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

    free(wave.waveDataChunk.sampleData);
    fclose(fp);
    return 0;
}
