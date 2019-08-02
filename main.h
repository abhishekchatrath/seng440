/**
 * 
 *  @sGroupID:      For WAV files, this value is always RIFF. RIFF stands for Resource Interchange File Format, and is not limited to WAV audio – RIFFs can also hold AVI video.
 *  @dwFileLength:  The total file size, in bytes, minus 8 (to ignore the text RIFF and WAVE in this header).
 *  @sRiffType:     For WAV files, this value is always WAVE.
 */
struct WAVE_HEADER {
    unsigned char   sGroupID[4];        // sGroupID = "RIFF"
    __uint32_t      dwFileLength;       // dwFileLength = /* varies */
    unsigned char   sRiffType[4];       // sRiffType = "WAVE"
};

/**
 * 
 *  @sGroupID:          Indicates the format chunk is defined below. Note the single space at the end to fill out the 4 bytes required here.
 *  @dwChunkSize:       The length of the rest of this chunk, in bytes (not including sGroupID or dwChunkSize).
 *  @wFormatTag:        For WAV files, this value is always 0x1 and indicates PCM format. 0x3 = IEEE Float. 0x6 = A law. 0x7 = Mu law. 
 *  @wChannels:         Indicates the number of channels in the audio. 1 for mono, 2 for stereo, etc.
 *  @dwSamplesPerSec:   The sampling rate for the audio (e.g. 44100 (CD), 48000 (DAT), 8000, 96000, depending on what you want).
 *  @dwAvgBytesPerSec:  The number of multichannel audio frames per second. Used to estimate how much memory is needed to play the file.
 *  @wBlockAlign:       The number of bytes in a multichannel audio frame.
 *  @dwBitsPerSample:   The bit depth (bits per sample) of the audio. Usually 8, 16, or 32.
 */
struct WAVE_FORMAT_CHUNK {
    unsigned char   sGroupID[4];        // sGroupID = "fmt " 
    __uint32_t      dwChunkSize;       // dwChunkSize = /* varies */ 
    __uint8_t       wFormatTag;        // wFormatTag = 1 
    __uint16_t      wChannels;         // wChannels = 1 
    __uint32_t      dwSamplesPerSec;   // dwSamplesPerSec = /* varies */ 
    __uint32_t      dwAvgBytesPerSec;  // dwAvgBytesPerSec = sampleRate * blockAlign 
    __uint16_t      wBlockAlign;       // wBlockAlign = wChannels * (dwBitsPerSample / 8) 
    __uint16_t      dwBitsPerSample;   // dwBitsPerSample = /* varies */
};

/**
 * 
 *  @sGroupID:       	Indicates the data chunk is coming next.
 *  @dwChunkSize:       The length of the array below.
 *  @sampleData:        All sample data is stored here.
 */
struct WAVE_DATA_CHUNK {
    unsigned char   sGroupID[4];        // sGroupID = "data"
    __uint32_t      dwChunkSize;        // dwChunkSize = /* varies */
    __uint16_t      sampleData;         // sampleData = dwSamplesPerSec * wChannels 
};

/**
 * 
 *  @sGroupID:       	Indicates the data chunk is coming next.
 *  @dwChunkSize:       The length of the array below.
 *  @sampleData:        All sample data is stored here.
 */
struct WAVE_DATA_CHUNK_COMPRESSED {
    unsigned char   sGroupID[4];        // sGroupID = "data"
    __uint32_t      dwChunkSize;        // dwChunkSize = /* varies */
    __uint8_t       sampleData;         // sampleData = dwSamplesPerSec * wChannels 
};

struct WAVE {
    struct WAVE_HEADER          waveHeader;
    struct WAVE_FORMAT_CHUNK    waveFormatChunk;
    struct WAVE_DATA_CHUNK      waveDataChunk;
};

struct WAVE_COMPRESSED {
    struct WAVE_HEADER                  waveHeader;
    struct WAVE_FORMAT_CHUNK            waveFormatChunk;
    struct WAVE_DATA_CHUNK_COMPRESSED   waveDataChunkCompressed;
};
