//GEORGE CHARLES ROSAR II CEO OF iridescent
//©️2026 GEORGE C. ROSAR II
//
//  LTCGenerator.mm
//  Audio iOS + Synth
//
//  Created by George Rosar on 8/3/26.
//

#include "LTCGenerator"
#include <string.h>

LTCGenerator::LTCGenerator()
    : _encoder(NULL), _initialized(false),
      _writeIndex(0), _readIndex(0), _bufferedCount(0) {
}

LTCGenerator::~LTCGenerator() {
    if (_encoder) {
        ltc_encoder_free(_encoder);
        _encoder = NULL;
    }
}

bool LTCGenerator::begin(double sampleRate, double fps) {
    if (_encoder) {
        ltc_encoder_free(_encoder);
        _encoder = NULL;
    }

    _encoder = ltc_encoder_create(sampleRate, fps, LTC_TV_625_50, 0);
    if (!_encoder) {
        _initialized = false;
        return false;
    }

    _writeIndex = 0;
    _readIndex = 0;
    _bufferedCount = 0;
    _initialized = true;

    setTimecode(0, 0, 0, 0);
    return true;
}

void LTCGenerator::setTimecode(uint8_t hours, uint8_t minutes, uint8_t seconds, uint8_t frames) {
    if (!_encoder) return;

    SMPTETimecode stime;
    memset(&stime, 0, sizeof(SMPTETimecode));
    stime.hours = hours;
    stime.mins  = minutes;
    stime.secs  = seconds;
    stime.frame = frames;

    ltc_encoder_set_timecode(_encoder, &stime);
}

void LTCGenerator::encodeNextFrameToRingBuffer() {
    if (!_encoder) return;

    // Trigger libltc frame generation
    ltc_encoder_encode_frame(_encoder);

    int sampleBytes = 0;
    // Retrieve raw 8-bit unsigned PCM buffer from libltc
    ltcsnd_sample_t* rawSamples = ltc_encoder_get_bufptr(_encoder, &sampleBytes, 1);

    // Convert 8-bit unsigned PCM to 16-bit signed PCM into internal ring buffer
    for (int i = 0; i < sampleBytes; i++) {
        if (_bufferedCount < 1024) {
            int16_t sample16 = ((int16_t)rawSamples[i] - 128) << 8;
            _ringBuffer[_writeIndex] = sample16;
            _writeIndex = (_writeIndex + 1) % 1024;
            _bufferedCount++;
        }
    }
}

void LTCGenerator::generateSamples(int16_t* buffer, size_t sampleCount) {
    if (!_initialized || !buffer) return;

    size_t written = 0;
    while (written < sampleCount) {
        // Encode more LTC frames as needed to fill request
        while (_bufferedCount == 0) {
            encodeNextFrameToRingBuffer();
        }

        buffer[written] = _ringBuffer[_readIndex];
        _readIndex = (_readIndex + 1) % 1024;
        _bufferedCount--;
        written++;
    }
}
/*
#include <Arduino.h>
#include <Audio.h>
#include "LTCDecoderEngine.h" // Your standalone library
#include "LTCGenerator.h"



// -------------------------------------------------------------------
// Example Sketch Loop: Patching Encoder straight into Decoder Node
// -------------------------------------------------------------------

AudioAdapterLTC        ltcEncoderNode; // Generator node
AudioAdapterLTCDecoder ltcDecoderNode; // Receiver node

// Directly route encoded AudioStream into Decoder AudioStream
AudioConnection patchCord(ltcEncoderNode, 0, ltcDecoderNode, 0);

void onTimecodeDecoded(const LTCDecodedTimecode* tc, void* userData) {
    Serial.printf("Decoded SMPTE: %02d:%02d:%02d:%02d\n",
                  tc->hours, tc->minutes, tc->seconds, tc->frames);
}

void setup() {
    Serial.begin(115200);
    AudioMemory(16);

    // 1. Initialize Encoder
    ltcEncoderNode.begin(25.0);
    ltcEncoderNode.generator.setTimecode(1, 30, 0, 0); // Start @ 01:30:00:00

    // 2. Initialize Decoder & assign callback
    ltcDecoderNode.begin(25.0);
    ltcDecoderNode.decoder.setCallback(onTimecodeDecoded);
}

void loop() {
    // The AudioStream interrupt handles generating, transferring audio blocks,
    // and decoding timecode behind the scenes.
}

*/

