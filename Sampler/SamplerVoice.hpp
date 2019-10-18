//
//  SamplerVoice.hpp
//  AudioKit Core
//
//  Created by Shane Dunne, revision history on Github.
//  Copyright © 2018 AudioKit. All rights reserved.
//

#pragma once
#include <math.h>

#include "VoiceBase.hpp"
#include "SampleBuffer.hpp"
#include "SampleOscillator.hpp"
#include "ADSREnvelope.hpp"
#include "MultiStageFilter.hpp"

namespace AudioKitCore
{
    struct SamplerVoiceParams
    {
        bool loopThruRelease;
        float pitchOffset;      // semitones, relative to MIDI note
        int filterStages;
    };

    struct SamplerModParameters
    {
        float masterVolume;
        float pitchOffset;
        float cutoffMultiple;
        float cutoffEgStrength;
        float filterQ;
        float filterVel;
    };

    struct SamplerVoice : public VoiceBase
    {
        SampleOscillator oscillator;      // every voice has 1 oscillator,
        SampleBuffer* pSampleBuffer;      // a pointer to the sample buffer for that oscillator,
        MultiStageFilter filterL, filterR;     // two filters (left/right),
        ADSREnvelope ampEG, filterEG;
        
        // temporary holding variables
        float noteFVel;     // filter EG multiplier: fraction 0.0 - 1.0, based on MIDI velocity
        SampleBuffer* pNewSampleBuffer; // holds next sample buffer to use at restart
        
        SamplerVoice() : VoiceBase() {}

        void init(double sampleRate, SamplerVoiceParams* pTimbreParameters, SamplerModParameters* pModParameters);
        void setFilterStages(int n) { filterL.setStages(n); filterR.setStages(n); }
        
        virtual void start(unsigned evt, unsigned noteNum, float freqHz, float volume);
        virtual void restart(unsigned evt, float volume);
        virtual void restart(unsigned evt, unsigned noteNum, float freqHz, float volume);
        virtual void release(unsigned evt);
        virtual bool isReleasing(void) { return ampEG.isReleasing(); }
        virtual void stop(unsigned evt);

        // return true if amp envelope is finished
        virtual bool doModulation(void);
        virtual bool getSamples(int nSamples, float* pOutLeft, float* pOutRight);
    };

}
