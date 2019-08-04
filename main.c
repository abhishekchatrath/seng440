#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"

FILE *fp;
unsigned char buffer[4];
struct WAVE wave;
struct WAVE_COMPRESSED waveCompressed;

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

    printf("Display Wave Headers:\t\t...\n");

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

    printf("\n...\nDisplaying Wave Headers:\tCOMPLETE\n\n");
}


void displayWaveDataSamples() {
    printf("Display Wave Data Samples:\t\t...\n");
    for (int i = 0; i < numSamples; i++) {
        printf("Sample %i:\t%hhx\n", i, wave.waveDataChunk.sampleData[i]);
    }
    printf("Displaying Wave Data Samples:\tCOMPLETE\n\n");
}

void saveWaveDataSamples() {
    printf("Saving Wave Data Samples to \"display.txt\"\n...\n");
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
    printf("COMPLETE\n\n");
}


void displayWaveHeadersAndSaveDataSamples() {
    displayWaveHeaders();
    // displayWaveDataSamples();
    saveWaveDataSamples();
}


void saveCompressedDataSamples() {
    printf("Saving Wave Data Samples to \"display_compressed.txt\"\n...\n");
    FILE *fpwriter = fopen("display_compressed.txt", "w");
    if (fpwriter == NULL) {
        printf("Could not write to \"display_compressed.txt\"");
        return;
    }
    char str[50];
    sprintf(str, "Wave Data Samples Compressed Using Mu Law");
    fwrite(str, 1, strlen(str), fpwriter);
    for (int i = 0; i < numSamples; i++) {
        sprintf(str, "\nSample %i:\t%d", i, waveCompressed.waveDataChunkCompressed.sampleData[i]);
        fwrite(str, 1, strlen(str), fpwriter);
    }
    fwrite("\n", 1, 1, fpwriter);
    fclose(fpwriter);
    printf("COMPLETE\n\n");
}

// 0: negative; 1: positive
short getSignFromSample(short sample) {
    return sample >= 0;
}


unsigned short getMagnitudeFromSample(short sample) {
    return (unsigned short) (sample < 0 ? -sample : sample);
}


__uint8_t generateCodeword(short sign, unsigned short magnitude) {
    int chord, step, codeword;
    if (magnitude & (1 << 12)) {
        chord = 0x7;
        step = (magnitude >> 8) & 0xF;
    } else if (magnitude & (1 << 11)) {
        chord = 0x6;
        step = (magnitude >> 7) & 0xF;
    } else if (magnitude & (1 << 10)) {
        chord = 0x5;
        step = (magnitude >> 6) & 0xF;
    } else if (magnitude & (1 << 9)) {
        chord = 0x4;
        step = (magnitude >> 5) & 0xF;
    } else if (magnitude & (1 << 8)) {
        chord = 0x3;
        step = (magnitude >> 4) & 0xF;
    } else if (magnitude & (1 << 7)) {
        chord = 0x2;
        step = (magnitude >> 3) & 0xF;
    } else if (magnitude & (1 << 6)) {
        chord = 0x1;
        step = (magnitude >> 2) & 0xF;
    } else if (magnitude & (1 << 5)) {
        chord = 0x0;
        step = (magnitude >> 1) & 0xF;
    } else {
        chord = 0x0;
        step = magnitude;
    }
    codeword = (sign << 7) | (chord << 4) | step;
    return (__uint8_t) codeword;
}


unsigned short getMagnitudeFromCodeword(char codeword) {
    int chord = (codeword & 0x70) >> 4;
    int step = codeword & 0x0F;
    int msb = 1;
    int magnitude;
    switch(chord) {
        
        default:
        case 0x7:   magnitude = (step << 8) | (msb << 12);
        break;

        case 0x6:   magnitude = (step << 7) | (msb << 11);
        break;
        
        case 0x5:   magnitude = (step << 6) | (msb << 10);
        break;
        
        case 0x4:   magnitude = (step << 5) | (msb << 9);
        break;
        
        case 0x3:   magnitude = (step << 4) | (msb << 8);
        break;
        
        case 0x2:   magnitude = (step << 3) | (msb << 7);
        break;
        
        case 0x1:   magnitude = (step << 2) | (msb << 6);
        break;
        
        case 0x0:   magnitude = (step << 1) | (msb << 5);
        break;
    }
    return (unsigned short) magnitude;
}


void compressDataSamples() {
    printf("Begin Compressing Data Samples:\n...\n");
    waveCompressed.waveDataChunkCompressed.sampleData = calloc(numSamples, sizeof(char));
    if (waveCompressed.waveDataChunkCompressed.sampleData == NULL) {
        printf("Could not allocate enough memory to store compressed data samples\n");
        return;
    }
    for (int i = 0; i < numSamples; i++) {
        short sign = getSignFromSample(wave.waveDataChunk.sampleData[i]);
        unsigned short magnitude = getMagnitudeFromSample(wave.waveDataChunk.sampleData[i]);
        __uint8_t codeword = generateCodeword(sign, magnitude);
        waveCompressed.waveDataChunkCompressed.sampleData[i] = codeword;
    }
    printf("COMPLETE\n\n");
}


void decompressDataSamples() {
    printf("Begin Decompressing Data Samples:\n...\n");
    for (int i = 0; i < numSamples; i++) {
        short sign = waveCompressed.waveDataChunkCompressed.sampleData[i] & 0x80;
        unsigned short magnitude = getMagnitudeFromCodeword(waveCompressed.waveDataChunkCompressed.sampleData[i]);
        short sample = (short) (sign ? magnitude : -magnitude);
        wave.waveDataChunk.sampleData[i] = sample;
    }
    printf("COMPLETE\n\n");
}


void convertIntToLittleEndian(__uint32_t chunk) {
    buffer[0] =  chunk & 0x000000FF;
    buffer[1] = (chunk & 0x0000FF00) >> 8;
    buffer[2] = (chunk & 0x00FF0000) >> 16;
    buffer[3] = (chunk & 0xFF000000) >> 24;
}


void convertShortToLittleEndian(__uint16_t chunk) {
    buffer[0] =  chunk & 0x000000FF;
    buffer[1] = (chunk & 0x0000FF00) >> 8;
}


void saveMuLawWaveFile() {
    printf("Saving Decompressed Wave File to \"decompressed.wav\"\n...\n");
    wave.waveFormatChunk.wFormatTag = (__uint8_t) 0x7;
    FILE *fpwriter = fopen("decompressed.wav", "w");
    if (fpwriter == NULL) {
        printf("Could not write to \"decompressed.wav\"");
        return;
    }
    
    fwrite(wave.waveHeader.sGroupID, sizeof(wave.waveHeader.sGroupID), 1, fpwriter);
    
    convertIntToLittleEndian(wave.waveHeader.dwFileLength); 
    fwrite(buffer, sizeof(buffer), 1, fpwriter);
    
    // convertIntToLittleEndian(wave.waveHeader.sRiffType); 
    // fwrite(buffer, sizeof(buffer), 1, fpwriter);

    fwrite(wave.waveHeader.sRiffType, sizeof(wave.waveHeader.sRiffType), 1, fpwriter);

    fclose(fpwriter);
    printf("COMPLETE\n\n");
}


int main (int argc, char **argv) {
    if (argc < 2) {
        perror("\nPlease input a valid .wav file\n");
        return printf("\nPlease input a valid .wav file\n");
    }

    printf("\nInput Wave Filename:\t\t%s\n", argv[1]);

    fp = fopen(argv[1], "rb");  //read in binary mode
    if (fp == NULL) {
        printf("Error opening file %s", argv[1]);
    }

    readWaveFile();
    displayWaveHeadersAndSaveDataSamples();
    compressDataSamples();
    saveCompressedDataSamples();
    decompressDataSamples();
    saveMuLawWaveFile();

    free(waveCompressed.waveDataChunkCompressed.sampleData);
    free(wave.waveDataChunk.sampleData);
    fclose(fp);
    return 0;
}
