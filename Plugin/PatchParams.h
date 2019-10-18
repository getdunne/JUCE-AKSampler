//
//  PatchParams.h
//  JUCE-AKSampler
//
//  Created by Shane Dunne, revision history on Github.
//  Copyright © 2018 AudioKit. All rights reserved.
//
#pragma once
#include "JuceHeader.h"
#include "SampleBuffer.hpp"
#include "ADSREnvelope.hpp"

struct AKSamplerParams
{
    // main
    struct MainParams {
        float masterLevel;
        int pitchBendUpSemitones;
        int pitchBendDownSemitones;
        float ampVelocitySensitivity;       // [0.0, 1.0]
        float filterVelocitySensitivity;    // [0.0, 1.0]
    } main;

    // oscillators
    struct OscillatorParams {
        String sfzName;
        int pitchOffsetSemitones;
        float detuneOffsetCents;
    } osc1;

    // filters
    struct FilterParams {
        int stages;                     // [0, 4]
        float cutoff;                   // [0.0, 1000.0]
        float resonance;                // [-12.0, 12.0]
        float envAmount;                // [0.0, 1000.0]
    } filter;

    // envelope generators
    struct EnvelopeParams {
        float attackTimeSeconds;
        float decayTimeSeconds;
        float sustainLevel;             // [0.0, 1.0]
        float releaseTimeSeconds;
    } ampEG, filterEG;

    // Set default values
    void setDefaultValues();

    // Save and Restore from XML
    void getXml(XmlElement* xml);
    void putXml(XmlElement* xml);
};

struct PatchParams
{
    // program name
    String programName;

    // parameters
    AKSamplerParams sampler;

    // Set default values
    void setDefaultValues();

    // Save and Restore from XML
    void getXml(XmlElement* xml);
    void putXml(XmlElement* xml);
};
