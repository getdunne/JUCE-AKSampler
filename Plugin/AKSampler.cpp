//
//  AKSampler.cpp
//  JUCE-AKSampler
//
//  Created by Shane Dunne, revision history on Github.
//  Copyright © 2018 AudioKit. All rights reserved.
//
#include "AKSampler.h"

void AKSampler::setupForTesting()
{
    loadTestWaveform();
    //// load one sinewave or sawtooth sample
    //float sine[1024];
    //for (int i = 0; i < 1024; i++) sine[i] = (float)sin(2 * 3.141592654 * i / 1024.0);   // sine
    ////for (int i = 0; i < 1024; i++) sine[i] = 2.0f * i / 1024.0f - 1.0f; // saw
    //AKSampleDataDescriptor sdd = {
    //    { 29, 44100.0f / 1024, 0, 127, 0, 127, true, 0.0f, 1.0f, 0.0f, 0.0f },
    //    44100.0f, false, 1, 1024, sine };
    //loadSampleData(sdd);
    //buildSimpleKeyMap();
    loopThruRelease = true;

    ampEGParams.setAttackDurationSeconds(0.1f);
    ampEGParams.setDecayDurationSeconds(0.1f);
    ampEGParams.sustainFraction = 0.5f;
    ampEGParams.setReleaseDurationSeconds(0.5f);

    modParams.cutoffMultiple = 2.0f;
    modParams.cutoffEgStrength = 20.0f;
    modParams.filterQ = 0.1f;
    filterEGParams.setAttackDurationSeconds(2.0f);
    filterEGParams.setDecayDurationSeconds(2.0f);
    filterEGParams.sustainFraction = 0.1f;
    filterEGParams.setReleaseDurationSeconds(2.0f);
}

void AKSampler::pitchBend(float offsetSemitones)
{
    voiceParams.pitchOffset = offsetSemitones;
}

enum {
    kMidiCCModWheel = 1,
    // lots more to add here...
    kMidiCCDamperPedal = 64,
    // lots more to add here...
    kMidiCCAllSoundOff = 120   // this or anything higher means all notes off
};

void AKSampler::controller(unsigned ccNumber, unsigned value)
{
    if (ccNumber == kMidiCCModWheel)
    {
        //cutoffMultiple = 100.0f * value / 127.0f;
        vibratoDepth = 1.0f * value / 127.0f;
    }
    else if (ccNumber == kMidiCCDamperPedal)
    {
        bool pedalDown = value >= 64;
        sustainPedal(pedalDown);
    }
    else if (ccNumber >= kMidiCCAllSoundOff)
    {
        voiceManager.stopAll();
    }
}
    
void AKSampler::setParams(AKSamplerParams& params)
{
    modParams.masterVolume = params.main.masterLevel;
    ampVelocitySensitivity = params.main.ampVelocitySensitivity;
    filterVelocitySensitivity = params.main.filterVelocitySensitivity;

    ampEGParams.setAttackDurationSeconds(params.ampEG.attackTimeSeconds);
    ampEGParams.setDecayDurationSeconds(params.ampEG.decayTimeSeconds);
    ampEGParams.sustainFraction = params.ampEG.sustainLevel;
    ampEGParams.setReleaseDurationSeconds(params.ampEG.releaseTimeSeconds);

    setFilterStages(params.filter.stages);
    if (params.filter.stages > 0)
    {
        modParams.cutoffMultiple = params.filter.cutoff;
        float res = params.filter.resonance / params.filter.stages;
        modParams.filterQ = pow(10.0f, -0.05f * res);
        modParams.cutoffEgStrength = params.filter.envAmount;
    }

    filterEGParams.setAttackDurationSeconds(params.filterEG.attackTimeSeconds);
    filterEGParams.setDecayDurationSeconds(params.filterEG.decayTimeSeconds);
    filterEGParams.sustainFraction = params.filterEG.sustainLevel;
    filterEGParams.setReleaseDurationSeconds(params.filterEG.releaseTimeSeconds);
}
