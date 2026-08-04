//GEORGE CHARLES ROSAR II CEO OF iridescent
//©️2026 GEORGE C. ROSAR II
//
//  LTCDecoderEngine.mm
//  Audio iOS + Synth
//
//  Created by George Rosar on 8/3/26.
//

#include "LTCDecoderEngine"

#include <string.h>

LTCDecoderEngine::LTCDecoderEngine()
    : _decoder(NULL), _initialized(false), _hasNewTimecode(false),
      _callback(NULL), _userData(NULL) {
    memset(&_lastTimecode, 0, sizeof(LTCDecodedTimecode));
}

LTCDecoderEngine::~LTCDecoderEngine() {
    if (_decoder) {
        ltc_decoder_free(_decoder);
        _decoder = NULL;
    }
}

bool LTCDecoderEngine::begin(double sampleRate, double fps) {
    if (_decoder) {
        ltc_decoder_free(_decoder);
        _decoder = NULL;
    }

    int samplesPerFrame = (int)(sampleRate / fps);
    // Create decoder queue with max 32 queue frames
    _decoder = ltc_decoder_create(samplesPerFrame, 32);
    
    if (!_decoder) {
        _initialized = false;
        return false;
    }

    _initialized = true;
    _hasNewTimecode = false;
    return true;
}

void LTCDecoderEngine::setCallback(LTCTimecodeCallback callback, void* userData) {
    _callback = callback;
    _userData = userData;
}
//static void ltc_frame_to_timecode(SMPTETimecode * ltcFrame, char * buffer, bool isit) {
//    if (strlen(buffer) < 11) return;
//    ltcFrame->hours   = ((buffer[0]-'0')*10) + (buffer[1]-'0');
//    ltcFrame->minutes = ((buffer[3]-'0')*10) + (buffer[4]-'0');
//    ltcFrame->seconds = ((buffer[6]-'0')*10) + (buffer[7]-'0');
//    ltcFrame->frames  = ((buffer[9]-'0')*10) + (buffer[10]-'0');
//}
void LTCDecoderEngine::processSamples(const int16_t* buffer, size_t sampleCount) {
    if (!_initialized || !_decoder || !buffer || sampleCount == 0) return;

    // Feed 16-bit signed PCM audio array directly to libltc
    ltc_decoder_write_s16(_decoder, (short*)buffer, (int)sampleCount, 0);

    // Drain and parse decoded frames
    parseAvailableFrames();
}

void LTCDecoderEngine::parseAvailableFrames() {
    LTCFrameExt frameExt;

    while (ltc_decoder_read(_decoder, &frameExt) != 0) {
        SMPTETimecode stime;
        
        ltc_frame_to_time(&stime, &frameExt.ltc, 1);

        _lastTimecode.hours   = stime.hours;
        _lastTimecode.minutes = stime.mins;
        _lastTimecode.seconds = stime.secs;
        _lastTimecode.frames  = stime.frame;
        _hasNewTimecode       = true;

        if (_callback) {
            _callback(&_lastTimecode, _userData);
        }
    }
}

bool LTCDecoderEngine::getLatestTimecode(LTCDecodedTimecode* outTimecode) {
    if (!_hasNewTimecode) return false;

    if (outTimecode) {
        *outTimecode = _lastTimecode;
    }
    
    _hasNewTimecode = false; // Reset read flag
    return true;
}
/*
#include <Arduino.h>
#include <Audio.h>
#include "LTCDecoderEngine.h" // Your standalone library
#include "LTCGenerator.h"


// -------------------------------------------------------------------
// Example Sketch Loop: Patching Encoder straight into Decoder Node
// -------------------------------------------------------------------

AudioAdapterLTCEncoder ltcEncoderNode; // Generator node
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

