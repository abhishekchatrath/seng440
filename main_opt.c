#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main_opt.h"
#include <time.h>

// FILE *fp;
unsigned char buffer[4];
// struct WAVE wave;
// struct WAVE_COMPRESSED waveCompressed;

// unsigned long numSamples;
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

    FILE *fp;

    fp = fopen(argv[1], "rb");  //read in binary mode
    if (fp == NULL) {
        printf("Error opening file %s", argv[1]);
    }

    struct WAVE *wave;
    struct WAVE_COMPRESSED *waveCompressed;

    __uint64_t numSamples = readWaveFile(fp, wave);
    displayWaveHeadersAndSaveDataSamples(wave, numSamples);

    printf("Begin Compressing Data Samples:\n...\n");
    start = clock();
    __uint8_t *codewords = compressDataSamples(wave, numSamples);
    stop = clock();
    printf("COMPLETE\n\n");
    compressionDuration = (double) (stop - start) / CLOCKS_PER_SEC;

    saveCompressedDataSamples(codewords, numSamples);

    printf("Begin Decompressing Data Samples:\n...\n");
    start = clock();
    decompressDataSamples(wave, codewords, numSamples);
    stop = clock();
    printf("COMPLETE\n\n");
    decompressionDuration = (double) (stop - start) / CLOCKS_PER_SEC;

    saveMuLawWaveFile(wave, numSamples);

    free(codewords);
    free(wave->waveDataChunk.sampleData);
    fclose(fp);

    printf("Audio Compression (Mu Law):\t\t%fs sec\n", compressionDuration);
    printf("Audio Decompression (Inverse Mu Law):\t%fs sec\n\n", decompressionDuration);

    return 0;
}


// read wave file functions 

void readWaveFileHeaders(FILE *fp, struct WAVE *wave) {
    printf("\nBegin Reading Wave Headers:\t...\n");

    // Read wave header
    // wave->waveHeader.sGroupID
    fread(wave->waveHeader.sGroupID,                 sizeof(wave->waveHeader.sGroupID), 1, fp);
    
    fread(buffer,                                   sizeof(buffer), 1, fp);
    wave->waveHeader.dwFileLength = (buffer[0]) | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
    
    fread(wave->waveHeader.sRiffType,                sizeof(wave->waveHeader.sRiffType), 1, fp);
    

    // Read wave format chunk
    fread(wave->waveFormatChunk.sGroupID,            sizeof(wave->waveFormatChunk.sGroupID), 1, fp);
    
    fread(buffer,                                   sizeof(buffer), 1, fp);
    wave->waveFormatChunk.dwChunkSize = (buffer[0]) | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
    
    fread(buffer,                                   sizeof(__uint16_t), 1, fp);
    wave->waveFormatChunk.wFormatTag = buffer[0] | buffer[1] << 8;

    fread(buffer,                                   sizeof(__uint16_t), 1, fp);
    wave->waveFormatChunk.wChannels = (buffer[0]) | (buffer[1] << 8);

    fread(buffer,                                   sizeof(buffer), 1, fp);
    wave->waveFormatChunk.dwSamplesPerSec = (buffer[0]) | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

    fread(buffer,                                   sizeof(buffer), 1, fp);
    wave->waveFormatChunk.dwAvgBytesPerSec = (buffer[0]) | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
    
    fread(buffer,                                   sizeof(__uint16_t), 1, fp);
    wave->waveFormatChunk.wBlockAlign = (buffer[0]) | (buffer[1] << 8);
    
    fread(buffer,                                   sizeof(__uint16_t), 1, fp);
    wave->waveFormatChunk.dwBitsPerSample = (buffer[0]) | (buffer[1] << 8);


    // Read wave data chunk
    fread(buffer, sizeof(buffer), 1, fp);
    int notData = strcmp(buffer, "data");
    int fileEnd = wave->waveHeader.dwFileLength - 40;
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
    strcpy(wave->waveDataChunk.sGroupID, buffer);

    fread(buffer,                                sizeof(buffer), 1, fp);
    wave->waveDataChunk.dwChunkSize = (buffer[0]) | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

    printf("Reading Wave Headers:\t\tCOMPLETE\n\n");
}


__uint64_t readWaveFileDataSamples(FILE *fp, struct WAVE *wave) {
    if (wave->waveFormatChunk.wFormatTag == 1) {
        printf("Begin Reading PCM data:\t\t...\n");
        
        // numSamples = size of data in bits / (bits per sample * num of channels)
        __uint64_t numSamples = (wave->waveDataChunk.dwChunkSize * 8) / (wave->waveFormatChunk.dwBitsPerSample * wave->waveFormatChunk.wChannels);
        printf("numSamples:\t\t\t%lu\n", numSamples);

        // sizeOfEachSample = size of each sample in bytes
        sizeOfEachSample = (wave->waveFormatChunk.dwBitsPerSample * wave->waveFormatChunk.wChannels) / 8;
        printf("sizeOfEachSample:\t\t%lu\n", sizeOfEachSample);
        
        // printf("%lu\n%u\n", numSamples, sizeOfEachSample);
        wave->waveDataChunk.sampleData = calloc(numSamples, sizeOfEachSample);
        if (wave->waveDataChunk.sampleData == NULL) {
            printf("Could not allocate enough memory to read data samples\n");
            return 0;
        }

        for (int i = 0; i < numSamples; i++) {
            fread(buffer, sizeOfEachSample, 1, fp);
            wave->waveDataChunk.sampleData[i] = (buffer[0]) | (buffer[1] << 8);
        }

        printf("Reading PCM data:\t\tCOMPLETE\n\n");
        return numSamples;
    } else //if (wave->waveFormatChunk.wFormatTag == 7) 
    {
        printf("Only PCM data please");
        exit(1);
        // printf("Begin Reading Mu Law data:\t...");
        // printf("Reading Mu Law data:\t\tCOMPLETE\n\n");        
    }
}


__uint64_t readWaveFile(FILE *fp, struct WAVE *wave) {
    readWaveFileHeaders(fp, wave);
    return readWaveFileDataSamples(fp, wave);
}


// compress and decompress

__uint8_t * compressDataSamples(struct WAVE *wave, __uint64_t numSamples) {
    __uint8_t *codewords = calloc(numSamples, sizeof(char));
    if (codewords == NULL) {
        printf("Could not allocate enough memory to store compressed data samples\n");
        return 0;
    }
    volatile short *sampleData = wave->waveDataChunk.sampleData;
    short sample1, sample2, sample3, sample4, sample5;
    short sign1, sign2, sign3, sign4, sign5;
    __uint16_t magnitude1, magnitude2, magnitude3, magnitude4, magnitude5;
    __uint8_t codeword;
    
    int i = 5;

    sample1 = (sampleData[0] >> 2);
    sample2 = (sampleData[1] >> 2);
    sample3 = (sampleData[2] >> 2);
    sample4 = (sampleData[3] >> 2);
    sample5 = (sampleData[4] >> 2);

    numSamples -= 5;

    magnitude1 = sample1 < 0 ? -sample1 : sample1;
    magnitude2 = sample2 < 0 ? -sample2 : sample2;
    magnitude3 = sample3 < 0 ? -sample3 : sample3;
    magnitude4 = sample4 < 0 ? -sample4 : sample4;
    magnitude5 = sample5 < 0 ? -sample5 : sample5;

    sign1 = sample1 >= 0;
    sign2 = sample2 >= 0;
    sign3 = sample3 >= 0;
    sign4 = sample4 >= 0;
    sign5 = sample5 >= 0;

    magnitude1 += 33;
    magnitude2 += 33;
    magnitude3 += 33;
    magnitude4 += 33;
    magnitude5 += 33;

    codewords[0] = ~(generateCodeword(sign1, magnitude1));
    codewords[1] = ~(generateCodeword(sign2, magnitude2));
    codewords[2] = ~(generateCodeword(sign3, magnitude3));
    codewords[3] = ~(generateCodeword(sign4, magnitude4));
    codewords[4] = ~(generateCodeword(sign5, magnitude5));

    while (numSamples >= 5) {
        sample1 = (sampleData[i] >> 2);
        sample2 = (sampleData[i + 1] >> 2);
        sample3 = (sampleData[i + 2] >> 2);
        sample4 = (sampleData[i + 3] >> 2);
        sample5 = (sampleData[i + 4] >> 2);

        magnitude1 = sample1 < 0 ? -sample1 : sample1;
        magnitude2 = sample2 < 0 ? -sample2 : sample2;
        magnitude3 = sample3 < 0 ? -sample3 : sample3;
        magnitude4 = sample4 < 0 ? -sample4 : sample4;
        magnitude5 = sample5 < 0 ? -sample5 : sample5;

        sign1 = sample1 >= 0;
        sign2 = sample2 >= 0;
        sign3 = sample3 >= 0;
        sign4 = sample4 >= 0;
        sign5 = sample5 >= 0;

        numSamples -= 5;
        i += 5;

        magnitude1 += 33;
        magnitude2 += 33;
        magnitude3 += 33;
        magnitude4 += 33;
        magnitude5 += 33;

        codewords[i - 5] = ~(generateCodeword(sign1, magnitude1));
        codewords[i - 4] = ~(generateCodeword(sign2, magnitude2));
        codewords[i - 3] = ~(generateCodeword(sign3, magnitude3));
        codewords[i - 2] = ~(generateCodeword(sign4, magnitude4));
        codewords[i - 1] = ~(generateCodeword(sign5, magnitude5));
    }

    switch (numSamples) {
        case 4: 
        sample1 = (sampleData[i] >> 2);
        sample2 = (sampleData[i + 1] >> 2);
        sample3 = (sampleData[i + 2] >> 2);
        sample4 = (sampleData[i + 3] >> 2);
        magnitude1 = sample1 < 0 ? -sample1 : sample1;
        magnitude2 = sample2 < 0 ? -sample2 : sample2;
        magnitude3 = sample3 < 0 ? -sample3 : sample3;
        magnitude4 = sample4 < 0 ? -sample4 : sample4;
        sign1 = sample1 >= 0;
        sign2 = sample2 >= 0;
        sign3 = sample3 >= 0;
        sign4 = sample4 >= 0;
        magnitude1 += 33;
        magnitude2 += 33;
        magnitude3 += 33;
        magnitude4 += 33;
        codewords[i] = ~(generateCodeword(sign1, magnitude1));
        codewords[i + 1] = ~(generateCodeword(sign2, magnitude2));
        codewords[i + 2] = ~(generateCodeword(sign3, magnitude3));
        codewords[i - 2] = ~(generateCodeword(sign4, magnitude4));
        break;

        case 3: 
        sample1 = (sampleData[i] >> 2);
        sample2 = (sampleData[i + 1] >> 2);
        sample3 = (sampleData[i + 2] >> 2);
        magnitude1 = sample1 < 0 ? -sample1 : sample1;
        magnitude2 = sample2 < 0 ? -sample2 : sample2;
        magnitude3 = sample3 < 0 ? -sample3 : sample3;
        sign1 = sample1 >= 0;
        sign2 = sample2 >= 0;
        sign3 = sample3 >= 0;
        magnitude1 += 33;
        magnitude2 += 33;
        magnitude3 += 33;
        codewords[i] = ~(generateCodeword(sign1, magnitude1));
        codewords[i + 1] = ~(generateCodeword(sign2, magnitude2));
        codewords[i + 2] = ~(generateCodeword(sign3, magnitude3));
        break;

        case 2: 
        sample1 = (sampleData[i] >> 2);
        sample2 = (sampleData[i + 1] >> 2);
        magnitude1 = sample1 < 0 ? -sample1 : sample1;
        magnitude2 = sample2 < 0 ? -sample2 : sample2;
        sign1 = sample1 >= 0;
        sign2 = sample2 >= 0;
        magnitude1 += 33;
        magnitude2 += 33;
        codewords[i] = ~(generateCodeword(sign1, magnitude1));
        codewords[i + 1] = ~(generateCodeword(sign2, magnitude2));
        break;

        case 1: 
        sample1 = (sampleData[i] >> 2);
        magnitude1 = sample1 < 0 ? -sample1 : sample1;
        sign1 = sample1 >= 0;
        magnitude1 += 33;
        codewords[i] = ~(generateCodeword(sign1, magnitude1));
        // break;
        case 0:
        default:    break;
    }

    return codewords;
}


void decompressDataSamples(struct WAVE *wave, __uint8_t *codewords, __uint64_t numSamples) {
    __uint8_t codeword1, codeword2, codeword3, codeword4, codeword5;
    __uint16_t magnitude1, magnitude2, magnitude3, magnitude4, magnitude5;
    short sample1, sample2, sample3, sample4, sample5;
    short sign1, sign2, sign3, sign4, sign5;
    
    int i = 5;
    
    codeword1 = ~(codewords[0]);
    codeword2 = ~(codewords[1]);
    codeword3 = ~(codewords[2]);
    codeword4 = ~(codewords[3]);
    codeword5 = ~(codewords[4]);
    
    magnitude1 = getMagnitudeFromCodeword(codeword1);
    magnitude2 = getMagnitudeFromCodeword(codeword2);
    magnitude3 = getMagnitudeFromCodeword(codeword3);
    magnitude4 = getMagnitudeFromCodeword(codeword4);
    magnitude5 = getMagnitudeFromCodeword(codeword5);
    
    sign1 = (codeword1 & 0x80) >> 7;
    sign2 = (codeword2 & 0x80) >> 7;
    sign3 = (codeword3 & 0x80) >> 7;
    sign4 = (codeword4 & 0x80) >> 7;
    sign5 = (codeword5 & 0x80) >> 7;
    
    magnitude1 -= 33;
    magnitude2 -= 33;
    magnitude3 -= 33;
    magnitude4 -= 33;
    magnitude5 -= 33;
    
    numSamples -= 5;
    
    sample1 = (short) (sign1 ? magnitude1 : -magnitude1);
    sample2 = (short) (sign2 ? magnitude2 : -magnitude2);
    sample3 = (short) (sign3 ? magnitude3 : -magnitude3);
    sample4 = (short) (sign4 ? magnitude4 : -magnitude4);
    sample5 = (short) (sign5 ? magnitude5 : -magnitude5);
    
    wave->waveDataChunk.sampleData[0] = sample1 << 2;
    wave->waveDataChunk.sampleData[1] = sample2 << 2;
    wave->waveDataChunk.sampleData[2] = sample3 << 2;
    wave->waveDataChunk.sampleData[3] = sample4 << 2;
    wave->waveDataChunk.sampleData[4] = sample5 << 2;
    
    while (numSamples >= 5) {
        codeword1 = ~(codewords[i]);
        codeword2 = ~(codewords[i + 1]);
        codeword3 = ~(codewords[i + 2]);
        codeword4 = ~(codewords[i + 3]);
        codeword5 = ~(codewords[i + 4]);

        magnitude1 = getMagnitudeFromCodeword(codeword1);
        magnitude2 = getMagnitudeFromCodeword(codeword2);
        magnitude3 = getMagnitudeFromCodeword(codeword3);
        magnitude4 = getMagnitudeFromCodeword(codeword4);
        magnitude5 = getMagnitudeFromCodeword(codeword5);

        sign1 = (codeword1 & 0x80) >> 7;
        sign2 = (codeword2 & 0x80) >> 7;
        sign3 = (codeword3 & 0x80) >> 7;
        sign4 = (codeword4 & 0x80) >> 7;
        sign5 = (codeword5 & 0x80) >> 7;

        magnitude1 -= 33;
        magnitude2 -= 33;
        magnitude3 -= 33;
        magnitude4 -= 33;
        magnitude5 -= 33;

        numSamples -= 5;
        i += 5;

        sample1 = (short) (sign1 ? magnitude1 : -magnitude1);
        sample2 = (short) (sign2 ? magnitude2 : -magnitude2);
        sample3 = (short) (sign3 ? magnitude3 : -magnitude3);
        sample4 = (short) (sign4 ? magnitude4 : -magnitude4);
        sample5 = (short) (sign5 ? magnitude5 : -magnitude5);

        wave->waveDataChunk.sampleData[i - 5] = sample1 << 2;
        wave->waveDataChunk.sampleData[i - 4] = sample2 << 2;
        wave->waveDataChunk.sampleData[i - 3] = sample3 << 2;
        wave->waveDataChunk.sampleData[i - 2] = sample4 << 2;
        wave->waveDataChunk.sampleData[i - 1] = sample5 << 2;
    }

    switch (numSamples) {
        case 4:
        codeword1 = ~(codewords[i]);
        codeword2 = ~(codewords[i + 1]);
        codeword3 = ~(codewords[i + 2]);
        codeword4 = ~(codewords[i + 3]);
        magnitude1 = getMagnitudeFromCodeword(codeword1);
        magnitude2 = getMagnitudeFromCodeword(codeword2);
        magnitude3 = getMagnitudeFromCodeword(codeword3);
        magnitude4 = getMagnitudeFromCodeword(codeword4);
        sign1 = (codeword1 & 0x80) >> 7;
        sign2 = (codeword2 & 0x80) >> 7;
        sign3 = (codeword3 & 0x80) >> 7;
        sign4 = (codeword4 & 0x80) >> 7;
        magnitude1 -= 33;
        magnitude2 -= 33;
        magnitude3 -= 33;
        magnitude4 -= 33;
        sample1 = (short) (sign1 ? magnitude1 : -magnitude1);
        sample2 = (short) (sign2 ? magnitude2 : -magnitude2);
        sample3 = (short) (sign3 ? magnitude3 : -magnitude3);
        sample4 = (short) (sign4 ? magnitude4 : -magnitude4);
        wave->waveDataChunk.sampleData[i - 5] = sample1 << 2;
        wave->waveDataChunk.sampleData[i - 4] = sample2 << 2;
        wave->waveDataChunk.sampleData[i - 3] = sample3 << 2;
        wave->waveDataChunk.sampleData[i - 2] = sample4 << 2;
        break;

        case 3:
        codeword1 = ~(codewords[i]);
        codeword2 = ~(codewords[i + 1]);
        codeword3 = ~(codewords[i + 2]);
        magnitude1 = getMagnitudeFromCodeword(codeword1);
        magnitude2 = getMagnitudeFromCodeword(codeword2);
        magnitude3 = getMagnitudeFromCodeword(codeword3);
        sign1 = (codeword1 & 0x80) >> 7;
        sign2 = (codeword2 & 0x80) >> 7;
        sign3 = (codeword3 & 0x80) >> 7;
        magnitude1 -= 33;
        magnitude2 -= 33;
        magnitude3 -= 33;
        sample1 = (short) (sign1 ? magnitude1 : -magnitude1);
        sample2 = (short) (sign2 ? magnitude2 : -magnitude2);
        sample3 = (short) (sign3 ? magnitude3 : -magnitude3);
        wave->waveDataChunk.sampleData[i - 5] = sample1 << 2;
        wave->waveDataChunk.sampleData[i - 4] = sample2 << 2;
        wave->waveDataChunk.sampleData[i - 3] = sample3 << 2;
        break;

        case 2:
        codeword1 = ~(codewords[i]);
        codeword2 = ~(codewords[i + 1]);
        magnitude1 = getMagnitudeFromCodeword(codeword1);
        magnitude2 = getMagnitudeFromCodeword(codeword2);
        sign1 = (codeword1 & 0x80) >> 7;
        sign2 = (codeword2 & 0x80) >> 7;
        magnitude1 -= 33;
        magnitude2 -= 33;
        sample1 = (short) (sign1 ? magnitude1 : -magnitude1);
        sample2 = (short) (sign2 ? magnitude2 : -magnitude2);
        wave->waveDataChunk.sampleData[i - 5] = sample1 << 2;
        wave->waveDataChunk.sampleData[i - 4] = sample2 << 2;
        break;

        case 1:
        codeword1 = ~(codewords[i]);
        magnitude1 = getMagnitudeFromCodeword(codeword1);
        sign1 = (codeword1 & 0x80) >> 7;
        magnitude1 -= 33;
        sample1 = (short) (sign1 ? magnitude1 : -magnitude1);
        wave->waveDataChunk.sampleData[i - 5] = sample1 << 2;
        // break;
        case 0:
        default:    break;
    }
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
    int bit = 1;
    int magnitude;
    switch(chord) {
        
        default:
        case 0x0:   magnitude = bit | (step << 1) | (bit << 5);
        break;

        case 0x1:   magnitude = (bit << 1) | (step << 2) | (bit << 6);
        break;
        
        case 0x2:   magnitude = (bit << 2) | (step << 3) | (bit << 7);
        break;
        
        case 0x3:   magnitude = (bit << 3) | (step << 4) | (bit << 8);
        break;
        
        case 0x4:   magnitude = (bit << 4) | (step << 5) | (bit << 9);
        break;
        
        case 0x5:   magnitude = (bit << 5) | (step << 6) | (bit << 10);
        break;
        
        case 0x6:   magnitude = (bit << 6) | (step << 7) | (bit << 11);
        break;
        
        case 0x7:   magnitude = (bit << 7) | (step << 8) | (bit << 12);
        break;
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

void displayWaveHeadersAndSaveDataSamples(struct WAVE *wave, __uint64_t numSamples) {
    displayWaveHeaders(wave);
    // displayWaveDataSamples(wave);
    saveWaveDataSamples(wave, numSamples);
}


void displayWaveHeaders(struct WAVE *wave) {
    printf("Display Wave Headers:\t\t...\n");

    fwrite("(01-04): sGroupID\t\t", 1, 19, stdout);
    fwrite(wave->waveHeader.sGroupID, sizeof(wave->waveHeader.sGroupID), 1, stdout);

    printf("\n(05-08): dwFileLength\t\t%u", wave->waveHeader.dwFileLength);

    fwrite("\n(09-12): sRiffType\t\t", 1, 21, stdout);
    fwrite(wave->waveHeader.sRiffType, sizeof(wave->waveHeader.sRiffType), 1, stdout);

    fwrite("\n(13-16): sGroupID\t\t", 1, 20, stdout);
    fwrite(wave->waveFormatChunk.sGroupID, sizeof(wave->waveFormatChunk.sGroupID), 1, stdout);
    
    printf("\n(17-20): dwChunkSize\t\t%u", wave->waveFormatChunk.dwChunkSize);
    printf("\n(21-22): wFormatTag\t\t%u", wave->waveFormatChunk.wFormatTag);
    printf("\n(23-24): wChannels\t\t%u", wave->waveFormatChunk.wChannels);
    printf("\n(25-28): dwSamplesPerSec\t%u", wave->waveFormatChunk.dwSamplesPerSec);
    printf("\n(29-32): dwAvgBytesPerSec\t%u", wave->waveFormatChunk.dwAvgBytesPerSec);
    printf("\n(33-34): wBlockAlign\t\t%u", wave->waveFormatChunk.wBlockAlign);
    printf("\n(35-36): dwBitsPerSample\t%u", wave->waveFormatChunk.dwBitsPerSample);

    fwrite("\n(37-40): sGroupID\t\t", 1, 20, stdout);
    fwrite(wave->waveDataChunk.sGroupID, sizeof(wave->waveDataChunk.sGroupID), 1, stdout);

    printf("\n(41-44): dwChunkSize\t\t%u", wave->waveDataChunk.dwChunkSize);

    printf("\n...\nDisplaying Wave Headers:\tCOMPLETE\n\n");
}


void displayWaveDataSamples(struct WAVE *wave, __uint64_t numSamples) {
    printf("Display Wave Data Samples:\t\t...\n");
    for (int i = 0; i < numSamples; i++) {
        printf("Sample %i:\t%hhx\n", i, wave->waveDataChunk.sampleData[i]);
    }
    printf("Displaying Wave Data Samples:\tCOMPLETE\n\n");
}


// save original data samples into display.txt
void saveWaveDataSamples(struct WAVE *wave, __uint64_t numSamples) {
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
        sprintf(str, "\nSample %i:\t%d", i, wave->waveDataChunk.sampleData[i]);
        fwrite(str, 1, strlen(str), fpwriter);
    }
    fwrite("\n", 1, 1, fpwriter);
    fclose(fpwriter);
    printf("COMPLETE\n\n");
}


// save compressed data samples into display_compressed.txt
void saveCompressedDataSamples(__uint8_t *codewords, __uint64_t numSamples) {
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
        sprintf(str, "\nSample %i:\t%d", i, codewords[i]);
        fwrite(str, 1, strlen(str), fpwriter);
    }
    fwrite("\n", 1, 1, fpwriter);
    fclose(fpwriter);
    printf("COMPLETE\n\n");
}


// save decompressed wave file into decompressed.wav
void saveMuLawWaveFile(struct WAVE *wave, __uint64_t numSamples) {
    printf("Saving Decompressed Wave File to \"decompressed.wav\"\n...\n");
    wave->waveFormatChunk.wFormatTag = (__uint8_t) 0x7;
    FILE *fpwriter = fopen("decompressed.wav", "w");
    if (fpwriter == NULL) {
        printf("Could not write to \"decompressed.wav\"");
        return;
    }
    
    // headers
    fwrite(wave->waveHeader.sGroupID, sizeof(wave->waveHeader.sGroupID), 1, fpwriter);
    
    convertIntToLittleEndian(wave->waveHeader.dwFileLength); 
    fwrite(buffer, sizeof(buffer), 1, fpwriter);

    fwrite(wave->waveHeader.sRiffType, sizeof(wave->waveHeader.sRiffType), 1, fpwriter);

    fwrite(wave->waveFormatChunk.sGroupID, sizeof(wave->waveFormatChunk.sGroupID), 1, fpwriter);

    convertIntToLittleEndian(wave->waveFormatChunk.dwChunkSize); 
    fwrite(buffer, sizeof(buffer), 1, fpwriter);

    convertShortToLittleEndian(wave->waveFormatChunk.wFormatTag); 
    fwrite(buffer, sizeof(__uint16_t), 1, fpwriter);

    convertShortToLittleEndian(wave->waveFormatChunk.wChannels); 
    fwrite(buffer, sizeof(__uint16_t), 1, fpwriter);
    
    convertIntToLittleEndian(wave->waveFormatChunk.dwSamplesPerSec); 
    fwrite(buffer, sizeof(buffer), 1, fpwriter);

    convertIntToLittleEndian(wave->waveFormatChunk.dwAvgBytesPerSec); 
    fwrite(buffer, sizeof(buffer), 1, fpwriter);

    convertShortToLittleEndian(wave->waveFormatChunk.wBlockAlign); 
    fwrite(buffer, sizeof(__uint16_t), 1, fpwriter);

    convertShortToLittleEndian(wave->waveFormatChunk.dwBitsPerSample); 
    fwrite(buffer, sizeof(__uint16_t), 1, fpwriter);

    fwrite(wave->waveDataChunk.sGroupID, sizeof(wave->waveDataChunk.sGroupID), 1, fpwriter);

    convertIntToLittleEndian(wave->waveDataChunk.dwChunkSize); 
    fwrite(buffer, sizeof(buffer), 1, fpwriter);

    // data
    for (int i = 0; i < numSamples; i++) {
        convertShortToLittleEndian(wave->waveDataChunk.sampleData[i]);
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
