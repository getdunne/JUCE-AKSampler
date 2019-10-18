//
//  AKSampler.h
//  JUCE-AKSampler
//
//  Created by Shane Dunne, revision history on Github.
//  Copyright © 2018 AudioKit. All rights reserved.
//
#pragma once
#include "Sampler.hpp"
#include "PatchParams.h"

class AKSampler : public AudioKitCore::Sampler
{
public:
    // quick setup for testing: sine waves, simple envelopes
    void setupForTesting();

    void pitchBend(float offsetSemitones);
    void controller(unsigned ccNumber, unsigned value);
        
    void setParams(AKSamplerParams& params);
};
