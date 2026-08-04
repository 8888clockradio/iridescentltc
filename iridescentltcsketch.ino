//GEORGE CHARLES ROSAR II CEO OF iridescent
//©️2026 GEORGE C. ROSAR II
//
//  iridescentltcsketch.ino
//  Audio iOS + Synth
//
//  Created by George Rosar on 8/3/26.
//
//GPL 3.0 LICENSED


#include <Arduino.h>
#include <Audio.h>
#include "LTCDecoderEngine" // Your standalone library
#include "LTCGenerator"



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
