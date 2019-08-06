#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include <time.h>

FILE *fp;
unsigned char buffer[4];
struct WAVE wave;
struct WAVE_COMPRESSED waveCompressed;

unsigned long numSamples;
unsigned int sizeOfEachSample;

time_t start, stop;
double compressionDuration;
double decompressionDuration;


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
    
    start = clock();
    compressDataSamples();
    stop = clock();
    compressionDuration = (double) (stop - start) / CLOCKS_PER_SEC;

    saveCompressedDataSamples();

    start = clock();
    decompressDataSamples();
    stop = clock();
    decompressionDuration = (double) (stop - start) / CLOCKS_PER_SEC;

    saveMuLawWaveFile();

    free(waveCompressed.waveDataChunkCompressed.sampleData);
    free(wave.waveDataChunk.sampleData);
    fclose(fp);

    printf("Audio Compression (Mu Law):\t\t%fs sec\n", compressionDuration);
    printf("Audio Decompression (Inverse Mu Law):\t%fs sec\n\n", decompressionDuration);
    
    return 0;
}


// read wave file functions 

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
        printf("numSamples:\t\t\t%lu\n", numSamples);

        // sizeOfEachSample = size of each sample in bytes
        sizeOfEachSample = (wave.waveFormatChunk.dwBitsPerSample * wave.waveFormatChunk.wChannels) / 8;
        printf("sizeOfEachSample:\t\t%lu\n", sizeOfEachSample);
        
        // printf("%lu\n%u\n", numSamples, sizeOfEachSample);
        wave.waveDataChunk.sampleData = calloc(numSamples, sizeOfEachSample);
        if (wave.waveDataChunk.sampleData == NULL) {
            printf("Could not allocate enough memory to read data samples\n");
            return;
        }

        int i;
        for (i = 0; i < numSamples; i++) {
            fread(buffer, sizeOfEachSample, 1, fp);
            wave.waveDataChunk.sampleData[i] = (buffer[0]) | (buffer[1] << 8);
        }

        printf("Reading PCM data:\t\tCOMPLETE\n\n");
    } else //if (wave.waveFormatChunk.wFormatTag == 7) 
    {
        printf("Only PCM data please");
        exit(1);
        // printf("Begin Reading Mu Law data:\t...");
        // printf("Reading Mu Law data:\t\tCOMPLETE\n\n");        
    }
}


void readWaveFile() {    
    readWaveFileHeaders();
    readWaveFileDataSamples();  
}


// compress and decompress

void compressDataSamples() {
    printf("Begin Compressing Data Samples:\n...\n");
    waveCompressed.waveDataChunkCompressed.sampleData = calloc(numSamples, sizeof(char));
    if (waveCompressed.waveDataChunkCompressed.sampleData == NULL) {
        printf("Could not allocate enough memory to store compressed data samples\n");
        return;
    }
    int i;
    for (i = 0; i < numSamples; i++) {
        short sample = (wave.waveDataChunk.sampleData[i] >> 2);
        short sign = getSignFromSample(sample);
        unsigned short magnitude = getMagnitudeFromSample(sample) + 33;
        __uint8_t codeword = generateCodeword(sign, magnitude);
        codeword = ~codeword;
        waveCompressed.waveDataChunkCompressed.sampleData[i] = codeword;
    }
    printf("COMPLETE\n\n");
}


void decompressDataSamples() {
    printf("Begin Decompressing Data Samples:\n...\n");
    __uint8_t codeword;
    int i;
    for (i = 0; i < numSamples; i++) {
        codeword = ~(waveCompressed.waveDataChunkCompressed.sampleData[i]);
        short sign = (codeword & 0x80) >> 7;
        unsigned short magnitude = (getMagnitudeFromCodeword(codeword) - 33);
        short sample = (short) (sign ? magnitude : -magnitude);
        wave.waveDataChunk.sampleData[i] = sample << 2;
    }
    printf("COMPLETE\n\n");
}


// helper functions 

// 0: negative; 1: positive
short getSignFromSample(short sample) {
    return sample >= 0;
}


// if magnitude is negative then return -magnitude
unsigned short getMagnitudeFromSample(short sample) {
    return (unsigned short) (sample < 0 ? -sample : sample);
}


// constructed from mu law binary encoding table and code snippets provided in audio compression slides
__uint8_t generateCodeword(short sign, unsigned short magnitude) {
    int chord, step, codeword;
    if (magnitude & (1 << 12)) {
        chord = 0x7;
        step = (magnitude >> 8) & 0xF;
    } 
    else if (magnitude & (1 << 11)) {
        chord = 0x6;
        step = (magnitude >> 7) & 0xF;
    } 
    else if (magnitude & (1 << 10)) {
        chord = 0x5;
        step = (magnitude >> 6) & 0xF;
    } 
    else if (magnitude & (1 << 9)) {
        chord = 0x4;
        step = (magnitude >> 5) & 0xF;
    } 
    else if (magnitude & (1 << 8)) {
        chord = 0x3;
        step = (magnitude >> 4) & 0xF;
    } 
    else if (magnitude & (1 << 7)) {
        chord = 0x2;
        step = (magnitude >> 3) & 0xF;
    } 
    else if (magnitude & (1 << 6)) {
        chord = 0x1;
        step = (magnitude >> 2) & 0xF;
    } 
    else if (magnitude & (1 << 5)) {
        chord = 0x0;
        step = (magnitude >> 1) & 0xF;
    } 
    else {
        chord = 0x0;
        step = magnitude;
    }
    codeword = (sign << 7) | (chord << 4) | step;
    return (__uint8_t) codeword;
}


// decode magnitude from the sign, chord and step bits in the codeword
unsigned short getMagnitudeFromCodeword(char codeword) {
    int chord = (codeword & 0x70) >> 4;
    int step = codeword & 0x0F;
    int msb = 1, lsb = 1;
    int magnitude;
    
    if (chord == 0x7) {
        magnitude = (lsb << 7) | (step << 8) | (msb << 12);
    }
    else if (chord == 0x6) {
        magnitude = (lsb << 6) | (step << 7) | (msb << 11);
    }
    else if (chord == 0x5) {
        magnitude = (lsb << 5) | (step << 6) | (msb << 10);
    }
    else if (chord == 0x4) {
        magnitude = (lsb << 4) | (step << 5) | (msb << 9);
    }
    else if (chord == 0x3) {
        magnitude = (lsb << 3) | (step << 4) | (msb << 8);
    }
    else if (chord == 0x2) {
        magnitude = (lsb << 2) | (step << 3) | (msb << 7);
    }
    else if (chord == 0x1) {
        magnitude = (lsb << 1) | (step << 2) | (msb << 6);
    }
    else if (chord == 0x0) {
        magnitude = lsb | (step << 1) | (msb << 5);
    }

    return (unsigned short) magnitude;
}


// converts unigned integer into little endian form
void convertIntToLittleEndian(__uint32_t chunk) {
    buffer[0] =  chunk & 0x000000FF;
    buffer[1] = (chunk & 0x0000FF00) >> 8;
    buffer[2] = (chunk & 0x00FF0000) >> 16;
    buffer[3] = (chunk & 0xFF000000) >> 24;
}


// converts unigned short into little endian form
void convertShortToLittleEndian(__uint16_t chunk) {
    buffer[0] =  chunk & 0x000000FF;
    buffer[1] = (chunk & 0x0000FF00) >> 8;
}


// display and file write functions

void displayWaveHeadersAndSaveDataSamples() {
    displayWaveHeaders();     
    // displayWaveDataSamples();
    saveWaveDataSamples();
}


void displayWaveHeaders() {
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
    int i;
    for (i = 0; i < numSamples; i++) {
        printf("Sample %i:\t%hhx\n", i, wave.waveDataChunk.sampleData[i]);
    }
    printf("Displaying Wave Data Samples:\tCOMPLETE\n\n");
}


// save original data samples into display.txt
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
    int i;
    for (i = 0; i < numSamples; i++) {
        sprintf(str, "\nSample %i:\t%d", i, wave.waveDataChunk.sampleData[i]);
        fwrite(str, 1, strlen(str), fpwriter);
    }
    fwrite("\n", 1, 1, fpwriter);
    fclose(fpwriter);
    printf("COMPLETE\n\n");
}


// save compressed data samples into display_compressed.txt
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
    int i;
    for (i = 0; i < numSamples; i++) {
        sprintf(str, "\nSample %i:\t%d", i, waveCompressed.waveDataChunkCompressed.sampleData[i]);
        fwrite(str, 1, strlen(str), fpwriter);
    }
    fwrite("\n", 1, 1, fpwriter);
    fclose(fpwriter);
    printf("COMPLETE\n\n");
}


// save decompressed wave file into decompressed.wav
void saveMuLawWaveFile() {
    printf("Saving Decompressed Wave File to \"decompressed.wav\"\n...\n");
    wave.waveFormatChunk.wFormatTag = (__uint8_t) 0x7;
    FILE *fpwriter = fopen("decompressed.wav", "w");
    if (fpwriter == NULL) {
        printf("Could not write to \"decompressed.wav\"");
        return;
    }
    
    // headers
    fwrite(wave.waveHeader.sGroupID, sizeof(wave.waveHeader.sGroupID), 1, fpwriter);
    
    convertIntToLittleEndian(wave.waveHeader.dwFileLength); 
    fwrite(buffer, sizeof(buffer), 1, fpwriter);

    fwrite(wave.waveHeader.sRiffType, sizeof(wave.waveHeader.sRiffType), 1, fpwriter);

    fwrite(wave.waveFormatChunk.sGroupID, sizeof(wave.waveFormatChunk.sGroupID), 1, fpwriter);

    convertIntToLittleEndian(wave.waveFormatChunk.dwChunkSize); 
    fwrite(buffer, sizeof(buffer), 1, fpwriter);

    convertShortToLittleEndian(wave.waveFormatChunk.wFormatTag); 
    fwrite(buffer, sizeof(__uint16_t), 1, fpwriter);

    convertShortToLittleEndian(wave.waveFormatChunk.wChannels); 
    fwrite(buffer, sizeof(__uint16_t), 1, fpwriter);
    
    convertIntToLittleEndian(wave.waveFormatChunk.dwSamplesPerSec); 
    fwrite(buffer, sizeof(buffer), 1, fpwriter);

    convertIntToLittleEndian(wave.waveFormatChunk.dwAvgBytesPerSec); 
    fwrite(buffer, sizeof(buffer), 1, fpwriter);

    convertShortToLittleEndian(wave.waveFormatChunk.wBlockAlign); 
    fwrite(buffer, sizeof(__uint16_t), 1, fpwriter);

    convertShortToLittleEndian(wave.waveFormatChunk.dwBitsPerSample); 
    fwrite(buffer, sizeof(__uint16_t), 1, fpwriter);

    fwrite(wave.waveDataChunk.sGroupID, sizeof(wave.waveDataChunk.sGroupID), 1, fpwriter);

    convertIntToLittleEndian(wave.waveDataChunk.dwChunkSize); 
    fwrite(buffer, sizeof(buffer), 1, fpwriter);

    // data
    for (int i = 0; i < numSamples; i++) {
        convertShortToLittleEndian(wave.waveDataChunk.sampleData[i]);
        // if (i > 18100 && i < 18105) {
        //     printf("Data Buffer: %.2x %.2x %.2x %.2x\n", buffer[0], buffer[1], buffer[2], buffer[3]);
        // }
        fwrite(buffer, sizeOfEachSample, 1, fpwriter);
    }

    fclose(fpwriter);
    printf("COMPLETE\n\n");
}


/*
const char* hexToBin(char *hexString) {
    printf("inside %x\n", hexString);
    char hex;
    sprintf(hex, "%x", hexString);
    static char binary[10];
    switch((hex & 0xF0) >> 1) {
        case 0x0: strcpy(binary,"0000"); break;
        case 0x1: strcpy(binary,"0001"); break;
        case 0x2: strcpy(binary,"0010"); break;
        case 0x3: strcpy(binary,"0011"); break;
        case 0x4: strcpy(binary,"0100"); break;
        case 0x5: strcpy(binary,"0101"); break;
        case 0x6: strcpy(binary,"0110"); break;
        case 0x7: strcpy(binary,"0111"); break;
        case 0x8: strcpy(binary,"1000"); break;
        case 0x9: strcpy(binary,"1001"); break;
        case 0xa: strcpy(binary,"1010"); break;
        case 0xb: strcpy(binary,"1011"); break;
        case 0xc: strcpy(binary,"1100"); break;
        case 0xd: strcpy(binary,"1101"); break;
        case 0xe: strcpy(binary,"1110"); break;
        case 0xf: strcpy(binary,"1111"); break;
    }
    strcat(binary, " ");
    switch(hex & 0x0F) {
        case 0x0: strcat(binary,"0000"); break;
        case 0x1: strcat(binary,"0001"); break;
        case 0x2: strcat(binary,"0010"); break;
        case 0x3: strcat(binary,"0011"); break;
        case 0x4: strcat(binary,"0100"); break;
        case 0x5: strcat(binary,"0101"); break;
        case 0x6: strcat(binary,"0110"); break;
        case 0x7: strcat(binary,"0111"); break;
        case 0x8: strcat(binary,"1000"); break;
        case 0x9: strcat(binary,"1001"); break;
        case 0xa: strcat(binary,"1010"); break;
        case 0xb: strcat(binary,"1011"); break;
        case 0xc: strcat(binary,"1100"); break;
        case 0xd: strcat(binary,"1101"); break;
        case 0xe: strcat(binary,"1110"); break;
        case 0xf: strcat(binary,"1111"); break;
    }
    return binary;
}
*/
