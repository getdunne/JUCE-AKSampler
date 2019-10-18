//
//  Sampler.hpp
//  AudioKit Core
//
//  Created by Shane Dunne, revision history on Github.
//  Copyright © 2018 AudioKit. All rights reserved.
//

#include "AKSampler_Typedefs.h"
#include "SamplerVoice.hpp"
#include "FunctionTable.hpp"
#include "VoiceManager.hpp"

#include <list>

#define MAX_POLYPHONY 64        // number of voices
#define MIDI_NOTENUMBERS 128    // MIDI offers 128 distinct note numbers
#define CHUNKSIZE 16            // process samples in "chunks" this size

namespace AudioKitCore
{
    
    class Sampler
    {
    public:
        Sampler();
        ~Sampler();
        
        int init(double sampleRate);    // returns system error code, nonzero only if a problem occurs
        void deinit();                  // call this to un-load all samples and clear the keymap

        void setFilterStages(int n);

        // call before/after loading/unloading samples, to ensure none are in use
        void stopAllVoices();
        void restartVoices();

        // call to load samples
        void loadSampleData(AKSampleDataDescriptor& sdd);
        void loadTestWaveform();
        
        // after loading samples, call one of these to build the key map
        void buildKeyMap(void);         // use this when you have full key mapping data (min/max note, vel)
        void buildSimpleKeyMap(void);   // or this when you don't
        
        // optionally call this to make samples continue looping after note-release
        void setLoopThruRelease(bool value) { loopThruRelease = value; }
        
        void playNote(unsigned noteNumber, unsigned velocity, float noteHz);
        void stopNote(unsigned noteNumber, bool immediate);
        void sustainPedal(bool down);
        
        void Render(unsigned channelCount, unsigned sampleCount, float *outBuffers[]);
        
    protected:
        // list of (pointers to) all loaded samples
        std::list<KeyMappedSampleBuffer*> sampleBufferList;
        
        // maps MIDI note numbers to "closest" samples (all velocity layers)
        std::list<KeyMappedSampleBuffer*> keyMap[MIDI_NOTENUMBERS];
        bool keyMapValid;

        // array of voice resources, and a voice manager
        SamplerVoice voice[MAX_POLYPHONY];
        VoiceManager voiceManager;

        // objects shared by all voices
        FunctionTableOscillator vibratoLFO;

        // simple parameters
        SamplerVoiceParams voiceParams;
        ADSREnvelopeParameters ampEGParams;
        ADSREnvelopeParameters filterEGParams;

        // modulation parameters
        SamplerModParameters modParams;
        float vibratoDepth;
        float ampVelocitySensitivity, filterVelocitySensitivity;

        // voice- and render-prep callbacks
        static float voicePrepCallback(void* thisPtr, void* voicePtr,
                                       unsigned noteNumber, unsigned velocity, float noteHz);
        static void renderPrepCallback(void* thisPtr);

        // sample-related parameters
        bool loopThruRelease;   // if true, sample continue looping thru note release phase

        // temporary state
        bool stoppingAllVoices;
        
        // helper functions
        KeyMappedSampleBuffer* lookupSample(unsigned noteNumber, unsigned velocity);
    };
}

